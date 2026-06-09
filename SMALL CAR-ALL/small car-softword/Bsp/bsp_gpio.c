#include "bsp_gpio.h"
#include "car_config.h"

/* Configure the STM32F1 4-bit mode field for a selected pin. */
void Bsp_GpioConfig(GPIO_TypeDef *port, uint8_t pin, GpioMode mode)
{
    volatile uint32_t *reg;
    uint32_t shift;

    if (pin < 8U)
    {
        reg = &port->CRL;
        shift = pin * 4U;
    }
    else
    {
        reg = &port->CRH;
        shift = (pin - 8U) * 4U;
    }

    *reg = (*reg & ~(0x0FUL << shift)) | ((uint32_t)mode << shift);
}

void Bsp_GpioWrite(GPIO_TypeDef *port, uint8_t pin, uint8_t value)
{
    if (value)
    {
        port->BSRR = PIN_MASK(pin);
    }
    else
    {
        port->BRR = PIN_MASK(pin);
    }
}

void Bsp_GpioToggle(GPIO_TypeDef *port, uint8_t pin)
{
    port->ODR ^= PIN_MASK(pin);
}

uint8_t Bsp_GpioRead(GPIO_TypeDef *port, uint8_t pin)
{
    return (port->IDR & PIN_MASK(pin)) ? 1U : 0U;
}

void Bsp_GpioInit(void)
{
    /* All project GPIOs live on ports A/B/C; AFIO is needed for EXTI remapping. */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |
                    RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_IOPCEN;

    /* Keep SWD enabled and release JTAG pins when needed. */
    AFIO->MAPR &= ~(7UL << 24);
    AFIO->MAPR |=  (2UL << 24);

    Bsp_GpioConfig(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_2MHZ);
    /* PC13 LED is commonly active-low on STM32F103C8 boards, so start with it off. */
    Bsp_GpioWrite(LED_PORT, LED_PIN, 1U);
}
