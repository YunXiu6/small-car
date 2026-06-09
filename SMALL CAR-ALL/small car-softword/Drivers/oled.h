#ifndef OLED_H
#define OLED_H

#include "stm32f10x.h"

/* Initialize the SSD1306-compatible 128x64 OLED over software I2C. */
void OLED_Init(void);
/* Clear all 8 display pages. */
void OLED_Clear(void);
/* Draw an ASCII string using a 6x8 font at pixel x and page row. */
void OLED_ShowString(uint8_t x, uint8_t page, const char *str);
/* Draw an unsigned decimal number using OLED_ShowString. */
void OLED_ShowUInt(uint8_t x, uint8_t page, uint16_t value);

#endif
