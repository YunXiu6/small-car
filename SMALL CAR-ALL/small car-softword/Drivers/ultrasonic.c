#include "ultrasonic.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "car_config.h"

volatile uint16_t g_ultrasonic_distance_cm;
volatile uint32_t g_ultrasonic_echo_width_us;
volatile uint32_t g_ultrasonic_trig_count;
volatile uint8_t g_ultrasonic_status = ULTRASONIC_NO_ECHO;
volatile uint8_t g_ultrasonic_echo_level;

static uint32_t elapsed_systick(uint32_t start, uint32_t now)
{
    uint32_t reload = SysTick->LOAD + 1UL;

    if (start >= now)
    {
        return start - now;
    }
    return start + reload - now;
}

static uint32_t systick_ticks_to_us(uint32_t ticks)
{
    return ticks / (CAR_SYSCLK_HZ / 1000000UL);
}

static uint32_t elapsed_us(uint32_t *last_tick)
{
    uint32_t now = SysTick->VAL;
    uint32_t ticks = elapsed_systick(*last_tick, now);
    *last_tick = now;
    return systick_ticks_to_us(ticks);
}

void Ultrasonic_Init(void)
{
    Bsp_GpioConfig(US_PORT, US_TRIG_PIN, GPIO_MODE_OUT_PP_2MHZ);
    Bsp_GpioConfig(US_PORT, US_ECHO_PIN, GPIO_MODE_IN_PULL);
    Bsp_GpioWrite(US_PORT, US_ECHO_PIN, 0U);
    Bsp_GpioWrite(US_PORT, US_TRIG_PIN, 0U);
}

UltrasonicStatus Ultrasonic_ReadCmEx(uint16_t *distance_cm)
{
    uint32_t width_us = 0U;
    uint32_t start_ms;
    uint32_t start_tick;

    *distance_cm = 0U;
    g_ultrasonic_distance_cm = 0U;
    g_ultrasonic_echo_width_us = 0U;
    g_ultrasonic_echo_level = Bsp_GpioRead(US_PORT, US_ECHO_PIN);

    if (g_ultrasonic_echo_level)
    {
        g_ultrasonic_status = ULTRASONIC_ECHO_STUCK_HIGH;
        return ULTRASONIC_ECHO_STUCK_HIGH;
    }

    Bsp_GpioWrite(US_PORT, US_TRIG_PIN, 0U);
    Bsp_DelayUs(3U);
    Bsp_GpioWrite(US_PORT, US_TRIG_PIN, 1U);
    Bsp_DelayUs(12U);
    Bsp_GpioWrite(US_PORT, US_TRIG_PIN, 0U);
    g_ultrasonic_trig_count++;

    start_ms = Bsp_Millis();
    while (Bsp_GpioRead(US_PORT, US_ECHO_PIN) == 0U)
    {
        if ((uint32_t)(Bsp_Millis() - start_ms) > 30U)
        {
            g_ultrasonic_status = ULTRASONIC_NO_ECHO;
            return ULTRASONIC_NO_ECHO;
        }
    }

    start_tick = SysTick->VAL;
    start_ms = Bsp_Millis();
    while (Bsp_GpioRead(US_PORT, US_ECHO_PIN))
    {
        width_us += elapsed_us(&start_tick);
        if (width_us > 30000U || (uint32_t)(Bsp_Millis() - start_ms) > 30U)
        {
            g_ultrasonic_echo_width_us = width_us;
            g_ultrasonic_status = ULTRASONIC_ECHO_TIMEOUT;
            return ULTRASONIC_ECHO_TIMEOUT;
        }
    }

    g_ultrasonic_echo_width_us = width_us;
    *distance_cm = (uint16_t)(width_us / 58U);
    if (*distance_cm < CAR_US_MIN_VALID_CM)
    {
        g_ultrasonic_distance_cm = *distance_cm;
        g_ultrasonic_status = ULTRASONIC_TOO_CLOSE;
        return ULTRASONIC_TOO_CLOSE;
    }
    g_ultrasonic_distance_cm = *distance_cm;
    g_ultrasonic_status = ULTRASONIC_OK;
    return ULTRASONIC_OK;
}

uint16_t Ultrasonic_ReadCm(void)
{
    uint16_t distance_cm;
    if (Ultrasonic_ReadCmEx(&distance_cm) == ULTRASONIC_OK)
    {
        return distance_cm;
    }
    return 0U;
}
