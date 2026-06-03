#include "app_car.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "bsp_usart.h"
#include "motor.h"
#include "ultrasonic.h"
#include "encoder.h"
#include "oled.h"
#include "mpu6050.h"
#include "ir.h"
#include "car_config.h"

typedef enum
{
    CAR_MODE_AUTO = 0,
    CAR_MODE_REMOTE,
    CAR_MODE_STOP
} CarMode;

static CarMode s_mode = CAR_MODE_AUTO;
static int16_t s_speed = CAR_DEFAULT_SPEED;
static uint16_t s_distance_cm = 0U;
static uint32_t s_last_remote_ms = 0U;
static uint8_t s_mpu_ok = 0U;
static uint8_t s_tilted = 0U;
static UltrasonicStatus s_us_status = ULTRASONIC_NO_ECHO;
static uint8_t s_close_confirm = 0U;
static int16_t s_motor_left = 0;
static int16_t s_motor_right = 0;
static uint8_t s_remote_forward_running = 0U;

static int16_t limit_speed(int16_t speed)
{
    if (speed > CAR_SPEED_MAX) return CAR_SPEED_MAX;
    if (speed < 180) return 180;
    return speed;
}

static void drive_from_key(IrKey key)
{
    switch (key)
    {
        case IR_KEY_FORWARD:
            s_mode = CAR_MODE_REMOTE;
            Motor_SetSpeed(s_speed, s_speed);
            s_motor_left = s_speed;
            s_motor_right = s_speed;
            s_remote_forward_running = 1U;
            break;
        case IR_KEY_BACKWARD:
            s_mode = CAR_MODE_REMOTE;
            Motor_SetSpeed((int16_t)-s_speed, (int16_t)-s_speed);
            s_motor_left = (int16_t)-s_speed;
            s_motor_right = (int16_t)-s_speed;
            s_remote_forward_running = 0U;
            break;
        case IR_KEY_LEFT:
            s_mode = CAR_MODE_REMOTE;
            Motor_SetSpeed((int16_t)-CAR_TURN_SPEED, CAR_TURN_SPEED);
            s_motor_left = (int16_t)-CAR_TURN_SPEED;
            s_motor_right = CAR_TURN_SPEED;
            s_remote_forward_running = 0U;
            break;
        case IR_KEY_RIGHT:
            s_mode = CAR_MODE_REMOTE;
            Motor_SetSpeed(CAR_TURN_SPEED, (int16_t)-CAR_TURN_SPEED);
            s_motor_left = CAR_TURN_SPEED;
            s_motor_right = (int16_t)-CAR_TURN_SPEED;
            s_remote_forward_running = 0U;
            break;
        case IR_KEY_STOP:
            Motor_Stop();
            s_motor_left = 0;
            s_motor_right = 0;
            s_remote_forward_running = 0U;
            s_mode = CAR_MODE_STOP;
            break;
        case IR_KEY_MODE:
            s_mode = CAR_MODE_REMOTE;
            Motor_Stop();
            s_motor_left = 0;
            s_motor_right = 0;
            s_remote_forward_running = 0U;
            break;
        case IR_KEY_AUTO:
            s_mode = CAR_MODE_AUTO;
            Motor_Stop();
            s_motor_left = 0;
            s_motor_right = 0;
            s_remote_forward_running = 0U;
            break;
        case IR_KEY_FORWARD_TOGGLE:
            s_mode = CAR_MODE_REMOTE;
            if (s_remote_forward_running)
            {
                Motor_Stop();
                s_motor_left = 0;
                s_motor_right = 0;
                s_remote_forward_running = 0U;
            }
            else
            {
                Motor_SetSpeed(s_speed, s_speed);
                s_motor_left = s_speed;
                s_motor_right = s_speed;
                s_remote_forward_running = 1U;
            }
            break;
        case IR_KEY_SPEED_UP:
            s_speed = limit_speed((int16_t)(s_speed + 80));
            break;
        case IR_KEY_SPEED_DOWN:
            s_speed = limit_speed((int16_t)(s_speed - 80));
            break;
        default:
            break;
    }
}

static void handle_serial(void)
{
    int ch = Usart1_ReadCharNonBlock();
    if (ch < 0)
    {
        return;
    }

    s_mode = CAR_MODE_REMOTE;
    s_last_remote_ms = Bsp_Millis();
    switch ((char)ch)
    {
        case 'w': drive_from_key(IR_KEY_FORWARD); break;
        case 's': drive_from_key(IR_KEY_BACKWARD); break;
        case 'a': drive_from_key(IR_KEY_LEFT); break;
        case 'd': drive_from_key(IR_KEY_RIGHT); break;
        case 'x': drive_from_key(IR_KEY_STOP); break;
        case 'm': drive_from_key(IR_KEY_MODE); break;
        case 'o': drive_from_key(IR_KEY_AUTO); break;
        case '+': drive_from_key(IR_KEY_SPEED_UP); break;
        case '-': drive_from_key(IR_KEY_SPEED_DOWN); break;
        default: break;
    }
}

static void handle_ir(void)
{
    uint32_t raw;
    IrKey key;
    if (IR_ReadRaw(&raw))
    {
        key = IR_MapKey(raw);
        if (key != IR_KEY_NONE)
        {
            s_last_remote_ms = Bsp_Millis();
            drive_from_key(key);
        }
    }
}

static void auto_drive(void)
{
    static uint8_t turn_side;

    if (s_us_status != ULTRASONIC_OK)
    {
        s_close_confirm = 0U;
        if (s_us_status == ULTRASONIC_NO_ECHO)
        {
            Motor_SetSpeed(s_speed, s_speed);
            s_motor_left = s_speed;
            s_motor_right = s_speed;
        }
        else
        {
            Motor_Stop();
            s_motor_left = 0;
            s_motor_right = 0;
        }
    }
    else if (s_distance_cm < CAR_OBSTACLE_STOP_CM)
    {
        if (s_close_confirm < CAR_US_CLOSE_CONFIRM)
        {
            s_close_confirm++;
            Motor_Stop();
            s_motor_left = 0;
            s_motor_right = 0;
        }
        else
        {
            Motor_SetSpeed((int16_t)-300, (int16_t)-300);
            s_motor_left = -300;
            s_motor_right = -300;
            Bsp_DelayMs(180U);
            if (turn_side)
            {
                Motor_SetSpeed(CAR_TURN_SPEED, (int16_t)-CAR_TURN_SPEED);
                s_motor_left = CAR_TURN_SPEED;
                s_motor_right = (int16_t)-CAR_TURN_SPEED;
            }
            else
            {
                Motor_SetSpeed((int16_t)-CAR_TURN_SPEED, CAR_TURN_SPEED);
                s_motor_left = (int16_t)-CAR_TURN_SPEED;
                s_motor_right = CAR_TURN_SPEED;
            }
            turn_side ^= 1U;
            Bsp_DelayMs(260U);
        }
    }
    else if (s_distance_cm > CAR_OBSTACLE_CLEAR_CM)
    {
        s_close_confirm = 0U;
        Motor_SetSpeed(s_speed, s_speed);
        s_motor_left = s_speed;
        s_motor_right = s_speed;
    }
    else
    {
        s_close_confirm = 0U;
        Motor_SetSpeed(220, 220);
        s_motor_left = 220;
        s_motor_right = 220;
    }
}

static void update_display(int16_t enc)
{
    OLED_Clear();
    OLED_ShowString(0U, 0U, "SMALL CAR");
    OLED_ShowString(0U, 1U, "MODE:");
    if (s_mode == CAR_MODE_AUTO) OLED_ShowString(36U, 1U, "AUTO");
    else if (s_mode == CAR_MODE_REMOTE) OLED_ShowString(36U, 1U, "REM");
    else OLED_ShowString(36U, 1U, "STOP");
    OLED_ShowString(0U, 2U, "DIST:");
    if (s_us_status == ULTRASONIC_OK)
    {
        OLED_ShowUInt(36U, 2U, s_distance_cm);
        OLED_ShowString(72U, 2U, "CM");
    }
    else if (s_us_status == ULTRASONIC_NO_ECHO)
    {
        OLED_ShowString(36U, 2U, "NOECHO");
    }
    else if (s_us_status == ULTRASONIC_ECHO_STUCK_HIGH)
    {
        OLED_ShowString(36U, 2U, "HIGH");
    }
    else if (s_us_status == ULTRASONIC_TOO_CLOSE)
    {
        OLED_ShowString(36U, 2U, "SHORT");
    }
    else
    {
        OLED_ShowString(36U, 2U, "TIMEOUT");
    }
    OLED_ShowString(0U, 3U, "SPD:");
    OLED_ShowUInt(30U, 3U, (uint16_t)s_speed);
    OLED_ShowString(0U, 4U, "ENC:");
#if CAR_USE_ENCODER
    OLED_ShowUInt(30U, 4U, (uint16_t)((enc < 0) ? -enc : enc));
#else
    (void)enc;
    OLED_ShowString(30U, 4U, "OFF");
#endif
    OLED_ShowString(0U, 5U, "IR:");
    OLED_ShowUInt(18U, 5U, g_ir_cmd_code);
    OLED_ShowString(42U, 5U, "B:");
    OLED_ShowUInt(54U, 5U, g_ir_bits);
    OLED_ShowString(78U, 5U, "S:");
    OLED_ShowUInt(90U, 5U, g_ir_state);
    if (s_tilted)
    {
        OLED_ShowString(0U, 6U, "TILT STOP");
    }
    else
    {
        OLED_ShowString(0U, 6U, "ST:");
        OLED_ShowUInt(18U, 6U, (uint16_t)s_us_status);
        OLED_ShowString(42U, 6U, "E:");
        OLED_ShowUInt(54U, 6U, g_ultrasonic_echo_level);
        OLED_ShowString(72U, 6U, "W:");
        OLED_ShowUInt(84U, 6U, (uint16_t)g_ultrasonic_echo_width_us);
    }
    OLED_ShowString(0U, 7U, "T:");
    OLED_ShowUInt(12U, 7U, (uint16_t)(g_ultrasonic_trig_count % 10000U));
    OLED_ShowString(48U, 7U, "M:");
    OLED_ShowUInt(60U, 7U, (uint16_t)((s_motor_left < 0) ? -s_motor_left : s_motor_left));
}

static void send_ultrasonic_debug(void)
{
    Usart1_SendString(" us_st=");
    Usart1_SendInt((int32_t)s_us_status);
    Usart1_SendString(" echo_lvl=");
    Usart1_SendInt(g_ultrasonic_echo_level);
    Usart1_SendString(" echo_us=");
    Usart1_SendInt((int32_t)g_ultrasonic_echo_width_us);
    Usart1_SendString(" trig=");
    Usart1_SendInt((int32_t)g_ultrasonic_trig_count);
    Usart1_SendString(" motor=");
    Usart1_SendInt(s_motor_left);
    Usart1_SendChar('/');
    Usart1_SendInt(s_motor_right);
    Usart1_SendString(" ir_cmd=");
    Usart1_SendInt(g_ir_cmd_code);
    Usart1_SendString(" ir_key=");
    Usart1_SendInt(g_ir_key);
    Usart1_SendString(" ir_cnt=");
    Usart1_SendInt((int32_t)g_ir_rx_count);
    Usart1_SendString(" ir_edge=");
    Usart1_SendInt((int32_t)g_ir_edge_count);
    Usart1_SendString(" ir_level=");
    Usart1_SendInt(g_ir_level);
    Usart1_SendString(" ir_dt=");
    Usart1_SendInt((int32_t)g_ir_last_dt_us);
    Usart1_SendString(" ir_bits=");
    Usart1_SendInt(g_ir_bits);
    Usart1_SendString(" ir_state=");
    Usart1_SendInt(g_ir_state);
}

void App_CarInit(void)
{
    Bsp_GpioInit();
    Bsp_TimeInit();
    Usart1_Init();
    Motor_Init();
    Ultrasonic_Init();
#if CAR_USE_ENCODER
    Encoder_Init();
#endif
    IR_Init();
    OLED_Init();
    s_mpu_ok = MPU6050_Init();

    Usart1_SendString("\r\nsmall car ready\r\n");
    OLED_ShowString(0U, 0U, "SMALL CAR READY");
    Bsp_DelayMs(600U);
}

void App_CarTask(void)
{
    static uint32_t last_us_ms;
    static uint32_t last_mpu_ms;
    static uint32_t last_ctrl_ms;
    static uint32_t last_disp_ms;
    static int16_t enc_delta;
    Mpu6050Raw mpu;

    handle_serial();
    handle_ir();

    if ((uint32_t)(Bsp_Millis() - last_us_ms) >= 80U)
    {
        last_us_ms = Bsp_Millis();
        s_us_status = Ultrasonic_ReadCmEx(&s_distance_cm);
    }

    if ((uint32_t)(Bsp_Millis() - last_mpu_ms) >= 40U)
    {
        last_mpu_ms = Bsp_Millis();
        if (s_mpu_ok && MPU6050_ReadRaw(&mpu))
        {
            s_tilted = MPU6050_IsTilted(&mpu);
        }
    }

    if ((uint32_t)(Bsp_Millis() - last_ctrl_ms) >= 20U)
    {
        last_ctrl_ms = Bsp_Millis();
#if CAR_USE_ENCODER
        enc_delta = Encoder_ReadDelta();
#else
        enc_delta = 0;
#endif

        if (s_tilted)
        {
            Motor_Brake();
            s_motor_left = 0;
            s_motor_right = 0;
            s_mode = CAR_MODE_STOP;
        }
        else if (s_mode == CAR_MODE_AUTO)
        {
            auto_drive();
        }
        else if (s_mode == CAR_MODE_REMOTE &&
                 !s_remote_forward_running &&
                 (uint32_t)(Bsp_Millis() - s_last_remote_ms) > CAR_REMOTE_TIMEOUT_MS)
        {
            Motor_Stop();
            s_motor_left = 0;
            s_motor_right = 0;
        }
    }

    if ((uint32_t)(Bsp_Millis() - last_disp_ms) >= 300U)
    {
        last_disp_ms = Bsp_Millis();
        Bsp_GpioToggle(LED_PORT, LED_PIN);
        update_display(enc_delta);
        Usart1_SendString("mode=");
        Usart1_SendInt((int32_t)s_mode);
        Usart1_SendString(" dist=");
        Usart1_SendInt(s_distance_cm);
        Usart1_SendString(" enc=");
        Usart1_SendInt(enc_delta);
        send_ultrasonic_debug();
        Usart1_SendString("\r\n");
    }
}
