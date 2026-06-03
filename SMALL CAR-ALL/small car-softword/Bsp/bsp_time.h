#ifndef BSP_TIME_H
#define BSP_TIME_H

#include "stm32f10x.h"

void Bsp_TimeInit(void);
uint32_t Bsp_Millis(void);
uint32_t Bsp_Micros(void);
void Bsp_DelayMs(uint32_t ms);
void Bsp_DelayUs(uint32_t us);

#endif
