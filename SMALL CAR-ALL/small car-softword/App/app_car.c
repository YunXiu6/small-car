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
            Motor_SetSpeed(s_speed, s_speed);
            break;
        case IR_KEY_BACKWARD:
            Motor_SetSpeed((int16_t)-s_speed, (int16_t)-s_speed);
            break;
        case IR_KEY_LEFT:
            Motor_SetSpeed((int16_t)-CAR_TURN_SPEED, CAR_TURN_SPEED);
            break;
        case IR_KEY_RIGHT:
            Motor_SetSpeed(CAR_TURN_SPEED, (int16_t)-CAR_TURN_SPEED);
            break;
        case IR_KEY_STOP:
            Motor_Stop();
            s_mode = CAR_MODE_STOP;
            break;
        case IR_KEY_MODE:
            s_mode = (s_mode == CAR_MODE_AUTO) ? CAR_MODE_REMOTE : CAR_MODE_AUTO;
            Motor_Stop();
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
            s_mode = CAR_MODE_REMOTE;
            s_last_remote_ms = Bsp_Millis();
            drive_from_key(key);
        }
    }
}

static void auto_drive(void)
{
    static uint8_t turn_side;

    if (s_distance_cm > 0U && s_distance_cm < CAR_OBSTACLE_STOP_CM)
    {
        Motor_SetSpeed((int16_t)-300, (int16_t)-300);
        Bsp_DelayMs(180U);
        if (turn_side)
        {
            Motor_SetSpeed(CAR_TURN_SPEED, (int16_t)-CAR_TURN_SPEED);
        }
        else
        {
            Motor_SetSpeed((int16_t)-CAR_TURN_SPEED, CAR_TURN_SPEED);
        }
        turn_side ^= 1U;
        Bsp_DelayMs(260U);
    }
    else if (s_distance_cm == 0U || s_distance_cm > CAR_OBSTACLE_CLEAR_CM)
    {
        Motor_SetSpeed(s_speed, s_speed);
    }
    else
    {
        Motor_SetSpeed(220, 220);
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
    OLED_ShowUInt(36U, 2U, s_distance_cm);
    OLED_ShowString(72U, 2U, "CM");
    OLED_ShowString(0U, 3U, "SPD:");
    OLED_ShowUInt(30U, 3U, (uint16_t)s_speed);
    OLED_ShowString(0U, 4U, "ENC:");
#if CAR_USE_ENCODER
    OLED_ShowUInt(30U, 4U, (uint16_t)((enc < 0) ? -enc : enc));
#else
    (void)enc;
    OLED_ShowString(30U, 4U, "OFF");
#endif
    OLED_ShowString(0U, 5U, "MPU:");
    OLED_ShowString(30U, 5U, s_mpu_ok ? "OK" : "NO");
    if (s_tilted)
    {
        OLED_ShowString(0U, 6U, "TILT STOP");
    }
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
        s_distance_cm = Ultrasonic_ReadCm();
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
            s_mode = CAR_MODE_STOP;
        }
        else if (s_mode == CAR_MODE_AUTO)
        {
            auto_drive();
        }
        else if (s_mode == CAR_MODE_REMOTE &&
                 (uint32_t)(Bsp_Millis() - s_last_remote_ms) > CAR_REMOTE_TIMEOUT_MS)
        {
            Motor_Stop();
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
        Usart1_SendString("\r\n");
    }
}
