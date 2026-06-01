#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"

void Usart1_Init(void);
void Usart1_SendChar(char ch);
void Usart1_SendString(const char *str);
int Usart1_ReadCharNonBlock(void);
void Usart1_SendInt(int32_t value);

#endif
