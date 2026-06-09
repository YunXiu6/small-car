#include "bsp_usart.h"
#include "bsp_gpio.h"
#include "car_config.h"

void Usart1_Init(void)
{
    /* USART1 sits on APB2 and uses PA9/PA10 in the default pin mapping. */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;

    Bsp_GpioConfig(USART_TX_PORT, USART_TX_PIN, GPIO_MODE_AF_PP_50MHZ);
    Bsp_GpioConfig(USART_RX_PORT, USART_RX_PIN, GPIO_MODE_IN_PULL);
    Bsp_GpioWrite(USART_RX_PORT, USART_RX_PIN, 1U);

    /* Oversampling-by-16 baud divider; rounding keeps 9600 bps error small. */
    USART1->BRR = (CAR_SYSCLK_HZ + (USART_BAUDRATE / 2UL)) / USART_BAUDRATE;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void Usart1_SendChar(char ch)
{
    while ((USART1->SR & USART_SR_TXE) == 0U)
    {
    }
    USART1->DR = (uint16_t)ch;
}

void Usart1_SendString(const char *str)
{
    while (*str)
    {
        Usart1_SendChar(*str++);
    }
}

int Usart1_ReadCharNonBlock(void)
{
    if (USART1->SR & USART_SR_RXNE)
    {
        return (int)(USART1->DR & 0xFFU);
    }
    return -1;
}

void Usart1_SendInt(int32_t value)
{
    char buf[12];
    uint8_t i = 0U;
    uint8_t j;
    uint32_t v;

    if (value < 0)
    {
        Usart1_SendChar('-');
        v = (uint32_t)(-value);
    }
    else
    {
        v = (uint32_t)value;
    }

    /* Build digits in reverse order, then transmit them from most significant to least. */
    do
    {
        buf[i++] = (char)('0' + (v % 10U));
        v /= 10U;
    } while (v && (i < sizeof(buf)));

    for (j = 0U; j < i; j++)
    {
        Usart1_SendChar(buf[i - 1U - j]);
    }
}
