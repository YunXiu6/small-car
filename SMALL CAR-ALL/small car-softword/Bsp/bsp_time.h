#ifndef BSP_TIME_H
#define BSP_TIME_H

#include "stm32f10x.h"

/* Start SysTick at 1 kHz for millisecond scheduling and delay helpers. */
void Bsp_TimeInit(void);
/* Return elapsed milliseconds since Bsp_TimeInit. Wraps naturally at uint32_t. */
uint32_t Bsp_Millis(void);
/* Return an approximate microsecond timestamp derived from SysTick. */
uint32_t Bsp_Micros(void);
/* Busy-wait for coarse millisecond delays. */
void Bsp_DelayMs(uint32_t ms);
/* Busy-wait for short microsecond delays used by sensor and software I2C timing. */
void Bsp_DelayUs(uint32_t us);

#endif
