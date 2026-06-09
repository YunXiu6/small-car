#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "stm32f10x.h"

/* STM32F1 GPIO CRL/CRH mode values used by Bsp_GpioConfig. */
typedef enum
{
    GPIO_MODE_ANALOG = 0x0,
    GPIO_MODE_IN_FLOATING = 0x4,
    GPIO_MODE_IN_PULL = 0x8,
    GPIO_MODE_OUT_PP_2MHZ = 0x2,
    GPIO_MODE_OUT_OD_2MHZ = 0x6,
    GPIO_MODE_AF_PP_50MHZ = 0xB,
    GPIO_MODE_AF_OD_50MHZ = 0xF
} GpioMode;

/* Enable GPIO clocks and configure board-level pins used by multiple modules. */
void Bsp_GpioInit(void);
/* Configure one STM32F1 pin by writing its 4-bit CRL/CRH mode field. */
void Bsp_GpioConfig(GPIO_TypeDef *port, uint8_t pin, GpioMode mode);
/* Write a digital output using BSRR/BRR so bit updates are atomic. */
void Bsp_GpioWrite(GPIO_TypeDef *port, uint8_t pin, uint8_t value);
/* Toggle a digital output by XORing the output data register bit. */
void Bsp_GpioToggle(GPIO_TypeDef *port, uint8_t pin);
/* Read a digital input and normalize it to 0 or 1. */
uint8_t Bsp_GpioRead(GPIO_TypeDef *port, uint8_t pin);

#endif
