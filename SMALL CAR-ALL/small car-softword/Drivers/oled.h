#ifndef OLED_H
#define OLED_H

#include "stm32f10x.h"

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowString(uint8_t x, uint8_t page, const char *str);
void OLED_ShowUInt(uint8_t x, uint8_t page, uint16_t value);

#endif
