#include "motor.h"
#include "bsp_gpio.h"
#include "car_config.h"

static int16_t clamp_speed(int16_t speed)
{
    if (speed > CAR_SPEED_MAX)
    {
        return CAR_SPEED_MAX;
    }
    if (speed < -CAR_SPEED_MAX)
    {
        return -CAR_SPEED_MAX;
    }
    return speed;
}

static void set_channel_a(int16_t speed)
{
    uint16_t duty;
    speed = clamp_speed(speed);
    duty = (uint16_t)((speed >= 0) ? speed : -speed);

    if (speed > 0)
    {
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_AIN1_PIN, 1U);
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_AIN2_PIN, 0U);
    }
    else if (speed < 0)
    {
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_AIN1_PIN, 0U);
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_AIN2_PIN, 1U);
    }
    else
    {
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_AIN1_PIN, 0U);
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_AIN2_PIN, 0U);
    }

    TIM2->CCR1 = duty;
}

static void set_channel_b(int16_t speed)
{
    uint16_t duty;
    speed = clamp_speed(speed);
    duty = (uint16_t)((speed >= 0) ? speed : -speed);

    if (speed > 0)
    {
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_BIN1_PIN, 1U);
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_BIN2_PIN, 0U);
    }
    else if (speed < 0)
    {
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_BIN1_PIN, 0U);
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_BIN2_PIN, 1U);
    }
    else
    {
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_BIN1_PIN, 0U);
        Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_BIN2_PIN, 0U);
    }

    TIM2->CCR2 = duty;
}

void Motor_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    Bsp_GpioConfig(MOTOR_PWM_PORT, MOTOR_PWMA_PIN, GPIO_MODE_AF_PP_50MHZ);
    Bsp_GpioConfig(MOTOR_PWM_PORT, MOTOR_PWMB_PIN, GPIO_MODE_AF_PP_50MHZ);
    Bsp_GpioConfig(MOTOR_DIR_PORT, MOTOR_AIN1_PIN, GPIO_MODE_OUT_PP_2MHZ);
    Bsp_GpioConfig(MOTOR_DIR_PORT, MOTOR_AIN2_PIN, GPIO_MODE_OUT_PP_2MHZ);
    Bsp_GpioConfig(MOTOR_DIR_PORT, MOTOR_BIN1_PIN, GPIO_MODE_OUT_PP_2MHZ);
    Bsp_GpioConfig(MOTOR_DIR_PORT, MOTOR_BIN2_PIN, GPIO_MODE_OUT_PP_2MHZ);

    TIM2->PSC = 0U;
    TIM2->ARR = CAR_PWM_PERIOD;
    TIM2->CCR1 = 0U;
    TIM2->CCR2 = 0U;
    TIM2->CCMR1 = (6U << 4) | TIM_CCMR1_OC1PE | (6U << 12) | TIM_CCMR1_OC2PE;
    TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIM2->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;

    Motor_Stop();
}

void Motor_SetSpeed(int16_t left, int16_t right)
{
    set_channel_a(left);
    set_channel_b(right);
}

void Motor_Stop(void)
{
    set_channel_a(0);
    set_channel_b(0);
}

void Motor_Brake(void)
{
    TIM2->CCR1 = 0U;
    TIM2->CCR2 = 0U;
    Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_AIN1_PIN, 1U);
    Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_AIN2_PIN, 1U);
    Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_BIN1_PIN, 1U);
    Bsp_GpioWrite(MOTOR_DIR_PORT, MOTOR_BIN2_PIN, 1U);
}
