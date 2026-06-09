#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f10x.h"

/* Initialize TIM2 PWM outputs and TB6612FNG direction pins. */
void Motor_Init(void);
/* Set left/right motor speed from -CAR_SPEED_MAX to +CAR_SPEED_MAX. */
void Motor_SetSpeed(int16_t left, int16_t right);
/* Coast both motors by driving speed to zero and clearing direction inputs. */
void Motor_Stop(void);
/* Short-brake both motors by asserting both direction inputs on each channel. */
void Motor_Brake(void);

#endif
