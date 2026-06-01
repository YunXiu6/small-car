#include "ir.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "car_config.h"

static uint16_t wait_level(uint8_t level, uint16_t timeout_us)
{
    uint16_t t = 0U;
    while (Bsp_GpioRead(IR_PORT, IR_PIN) == level)
    {
        Bsp_DelayUs(1U);
        if (++t >= timeout_us)
        {
            break;
        }
    }
    return t;
}

void IR_Init(void)
{
    Bsp_GpioConfig(IR_PORT, IR_PIN, GPIO_MODE_IN_PULL);
    Bsp_GpioWrite(IR_PORT, IR_PIN, 1U);
}

uint8_t IR_ReadRaw(uint32_t *code)
{
    uint8_t i;
    uint32_t data = 0U;
    uint16_t low;
    uint16_t high;

    if (Bsp_GpioRead(IR_PORT, IR_PIN))
    {
        return 0U;
    }

    low = wait_level(0U, 12000U);
    high = wait_level(1U, 7000U);
    if (low < 7500U || high < 3500U)
    {
        return 0U;
    }

    for (i = 0U; i < 32U; i++)
    {
        low = wait_level(0U, 900U);
        high = wait_level(1U, 2500U);
        if (low < 300U)
        {
            return 0U;
        }
        data >>= 1;
        if (high > 1000U)
        {
            data |= 0x80000000UL;
        }
    }

    *code = data;
    return 1U;
}

IrKey IR_MapKey(uint32_t code)
{
    uint8_t cmd = (uint8_t)((code >> 16) & 0xFFU);
    switch (cmd)
    {
        case 0x18: return IR_KEY_FORWARD;
        case 0x52: return IR_KEY_BACKWARD;
        case 0x08: return IR_KEY_LEFT;
        case 0x5A: return IR_KEY_RIGHT;
        case 0x1C: return IR_KEY_STOP;
        case 0x45: return IR_KEY_MODE;
        case 0x15: return IR_KEY_SPEED_UP;
        case 0x07: return IR_KEY_SPEED_DOWN;
        default: return IR_KEY_NONE;
    }
}
