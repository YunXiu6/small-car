#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"

/* Configure USART1 for simple polling-based debug and command traffic. */
void Usart1_Init(void);
/* Send one byte, blocking until the transmit data register is empty. */
void Usart1_SendChar(char ch);
/* Send a null-terminated ASCII string. */
void Usart1_SendString(const char *str);
/* Return one received byte, or -1 if no byte is waiting. */
int Usart1_ReadCharNonBlock(void);
/* Send a signed decimal integer without using stdio. */
void Usart1_SendInt(int32_t value);

#endif
