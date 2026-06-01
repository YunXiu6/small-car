#include "ultrasonic.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "car_config.h"

void Ultrasonic_Init(void)
{
    Bsp_GpioConfig(US_PORT, US_TRIG_PIN, GPIO_MODE_OUT_PP_2MHZ);
    Bsp_GpioConfig(US_PORT, US_ECHO_PIN, GPIO_MODE_IN_FLOATING);
    Bsp_GpioWrite(US_PORT, US_TRIG_PIN, 0U);
}

uint16_t Ultrasonic_ReadCm(void)
{
    uint32_t wait;
    uint32_t width_us = 0U;

    Bsp_GpioWrite(US_PORT, US_TRIG_PIN, 0U);
    Bsp_DelayUs(3U);
    Bsp_GpioWrite(US_PORT, US_TRIG_PIN, 1U);
    Bsp_DelayUs(12U);
    Bsp_GpioWrite(US_PORT, US_TRIG_PIN, 0U);

    wait = 30000U;
    while ((Bsp_GpioRead(US_PORT, US_ECHO_PIN) == 0U) && wait--)
    {
        Bsp_DelayUs(1U);
    }
    if (wait == 0U)
    {
        return 0U;
    }

    while (Bsp_GpioRead(US_PORT, US_ECHO_PIN))
    {
        Bsp_DelayUs(1U);
        width_us++;
        if (width_us > 30000U)
        {
            return 0U;
        }
    }

    return (uint16_t)(width_us / 58U);
}
