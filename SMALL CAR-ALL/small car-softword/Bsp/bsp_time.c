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

uint32_t Bsp_Micros(void)
{
    uint32_t ms1;
    uint32_t ms2;
    uint32_t val;
    uint32_t load = SysTick->LOAD + 1UL;

    do
    {
        ms1 = s_ms;
        val = SysTick->VAL;
        ms2 = s_ms;
    } while (ms1 != ms2);

    return (ms1 * 1000UL) + ((load - val) / (CAR_SYSCLK_HZ / 1000000UL));
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
    uint32_t target_ticks = us * (CAR_SYSCLK_HZ / 1000000UL);
    uint32_t reload = SysTick->LOAD + 1UL;
    uint32_t last = SysTick->VAL;
    uint32_t elapsed = 0UL;

    while (elapsed < target_ticks)
    {
        uint32_t now = SysTick->VAL;
        if (last >= now)
        {
            elapsed += last - now;
        }
        else
        {
            elapsed += last + reload - now;
        }
        last = now;
    }
}

void SysTick_Handler(void)
{
    s_ms++;
}
