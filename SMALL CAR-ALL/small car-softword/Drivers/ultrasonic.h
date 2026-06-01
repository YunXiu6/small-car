#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "stm32f10x.h"

void Ultrasonic_Init(void);
uint16_t Ultrasonic_ReadCm(void);

#endif
