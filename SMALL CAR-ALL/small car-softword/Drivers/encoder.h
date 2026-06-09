#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f10x.h"

/* Configure TIM3 in encoder interface mode for quadrature pulse counting. */
void Encoder_Init(void);
/* Return pulses accumulated since the previous call and reset the counter. */
int16_t Encoder_ReadDelta(void);

#endif
