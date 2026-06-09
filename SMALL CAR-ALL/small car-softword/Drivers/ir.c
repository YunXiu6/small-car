#include "ir.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "car_config.h"

/* Public debug state for checking IR reception without a logic analyzer. */
volatile uint32_t g_ir_raw_code;
volatile uint8_t g_ir_cmd_code;
volatile uint8_t g_ir_key;
volatile uint32_t g_ir_rx_count;
volatile uint32_t g_ir_edge_count;
volatile uint8_t g_ir_level;
volatile uint32_t g_ir_last_dt_us;
volatile uint8_t g_ir_bits;
volatile uint8_t g_ir_state;

/* Private NEC decoder state updated from EXTI1_IRQHandler. */
static volatile uint32_t s_last_edge_us;
static volatile uint32_t s_rx_data;
static volatile uint8_t s_rx_bits;
static volatile uint8_t s_rx_active;
static volatile uint8_t s_raw_ready;
static volatile uint32_t s_ready_code;

void IR_Init(void)
{
    uint32_t shift;

    /* IR receiver output idles high and toggles low for marks. */
    Bsp_GpioConfig(IR_PORT, IR_PIN, GPIO_MODE_IN_PULL);
    Bsp_GpioWrite(IR_PORT, IR_PIN, 1U);

    /* Route PB1 to EXTI1 and capture both edges for pulse-width decoding. */
    shift = (IR_PIN & 0x03U) * 4U;
    AFIO->EXTICR[IR_PIN >> 2U] &= ~(0x0FUL << shift);
    AFIO->EXTICR[IR_PIN >> 2U] |= (0x01UL << shift);

    EXTI->IMR |= PIN_MASK(IR_PIN);
    EXTI->RTSR |= PIN_MASK(IR_PIN);
    EXTI->FTSR |= PIN_MASK(IR_PIN);
    EXTI->PR = PIN_MASK(IR_PIN);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

uint8_t IR_ReadRaw(uint32_t *code)
{
    if (s_raw_ready)
    {
        /* Copy-and-clear is protected because the ISR can publish a new frame. */
        __disable_irq();
        *code = s_ready_code;
        s_raw_ready = 0U;
        __enable_irq();
        return 1U;
    }
    return 0U;
}

IrKey IR_MapKey(uint32_t code)
{
    /* NEC frame layout places the command byte in bits 16..23 for this decoder. */
    uint8_t cmd = (uint8_t)((code >> 16) & 0xFFU);
    IrKey key;
    switch (cmd)
    {
        case 22: key = IR_KEY_MODE; break;
        case 13: key = IR_KEY_AUTO; break;
        case 24: key = IR_KEY_FORWARD; break;
        case 82: key = IR_KEY_BACKWARD; break;
        case 8: key = IR_KEY_LEFT; break;
        case 90: key = IR_KEY_RIGHT; break;
        case 28: key = IR_KEY_FORWARD_TOGGLE; break;
        default: key = IR_KEY_NONE; break;
    }
    g_ir_key = (uint8_t)key;
    return key;
}

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & PIN_MASK(IR_PIN))
    {
        uint32_t now;
        uint32_t dt;
        uint8_t level;

        EXTI->PR = PIN_MASK(IR_PIN);
        now = Bsp_Micros();
        dt = now - s_last_edge_us;
        s_last_edge_us = now;
        level = Bsp_GpioRead(IR_PORT, IR_PIN);
        g_ir_level = level;
        g_ir_edge_count++;

        g_ir_last_dt_us = dt;

        /* Decode on falling edges; rising edges only update timing diagnostics. */
        if (level == 1U)
        {
            return;
        }

        /* A long idle gap resets the decoder between frames. */
        if (dt > 20000U)
        {
            s_rx_active = 0U;
            s_rx_bits = 0U;
            g_ir_bits = 0U;
            g_ir_state = 0U;
            return;
        }

        /* NEC leader low-to-low interval is about 4.5 ms after the initial mark. */
        if (dt > 3500U && dt < 5500U)
        {
            s_rx_active = 1U;
            s_rx_bits = 0U;
            s_rx_data = 0U;
            g_ir_bits = 0U;
            g_ir_state = 1U;
            return;
        }

        if (!s_rx_active)
        {
            return;
        }

        /* Short interval represents 0; long interval represents 1. */
        if (dt > 300U && dt < 900U)
        {
            s_rx_data >>= 1;
            s_rx_bits++;
        }
        else if (dt > 1200U && dt < 2200U)
        {
            s_rx_data >>= 1;
            s_rx_data |= 0x80000000UL;
            s_rx_bits++;
        }
        else
        {
            s_rx_active = 0U;
            s_rx_bits = 0U;
            g_ir_bits = 0U;
            g_ir_state = 2U;
            return;
        }
        g_ir_bits = s_rx_bits;

        if (s_rx_bits >= 32U)
        {
            /* Publish the complete frame for the foreground task. */
            s_ready_code = s_rx_data;
            g_ir_raw_code = s_rx_data;
            g_ir_cmd_code = (uint8_t)((s_rx_data >> 16) & 0xFFU);
            g_ir_rx_count++;
            s_raw_ready = 1U;
            s_rx_active = 0U;
            g_ir_state = 3U;
        }
    }
}
