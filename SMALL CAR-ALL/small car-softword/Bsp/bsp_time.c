#include "bsp_time.h"
#include "car_config.h"

static volatile uint32_t s_ms;

void Bsp_TimeInit(void)
{
    SysTick_Config(CAR_SYSCLK_HZ / 1000UL);
}

uint32_t Bsp_Millis(void)
{
    return s_ms;
}

void Bsp_DelayMs(uint32_t ms)
{
    uint32_t start = s_ms;
    while ((uint32_t)(s_ms - start) < ms)
    {
    }
}

void Bsp_DelayUs(uint32_t us)
{
    uint32_t ticks = (CAR_SYSCLK_HZ / 1000000UL) * us / 5UL;
    while (ticks--)
    {
        __NOP();
    }
}

void SysTick_Handler(void)
{
    s_ms++;
}
