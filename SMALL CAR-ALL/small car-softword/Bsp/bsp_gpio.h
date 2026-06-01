#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "stm32f10x.h"

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

void Bsp_GpioInit(void);
void Bsp_GpioConfig(GPIO_TypeDef *port, uint8_t pin, GpioMode mode);
void Bsp_GpioWrite(GPIO_TypeDef *port, uint8_t pin, uint8_t value);
void Bsp_GpioToggle(GPIO_TypeDef *port, uint8_t pin);
uint8_t Bsp_GpioRead(GPIO_TypeDef *port, uint8_t pin);

#endif
