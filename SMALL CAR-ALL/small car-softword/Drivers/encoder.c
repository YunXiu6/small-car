#include "encoder.h"
#include "bsp_gpio.h"
#include "car_config.h"

void Encoder_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    Bsp_GpioConfig(ENCODER_PORT, ENCODER_A_PIN, GPIO_MODE_IN_PULL);
    Bsp_GpioConfig(ENCODER_PORT, ENCODER_B_PIN, GPIO_MODE_IN_PULL);
    Bsp_GpioWrite(ENCODER_PORT, ENCODER_A_PIN, 1U);
    Bsp_GpioWrite(ENCODER_PORT, ENCODER_B_PIN, 1U);

    TIM3->PSC = 0U;
    TIM3->ARR = 0xFFFFU;
    TIM3->SMCR = 3U;
    TIM3->CCMR1 = 1U | (1U << 8);
    TIM3->CCER = 0U;
    TIM3->CNT = 0U;
    TIM3->CR1 = TIM_CR1_CEN;
}

int16_t Encoder_ReadDelta(void)
{
    int16_t delta = (int16_t)TIM3->CNT;
    TIM3->CNT = 0U;
    return delta;
}
