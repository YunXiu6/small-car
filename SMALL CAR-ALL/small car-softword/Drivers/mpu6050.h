#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f10x.h"

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t temp;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} Mpu6050Raw;

uint8_t MPU6050_Init(void);
uint8_t MPU6050_ReadRaw(Mpu6050Raw *data);
uint8_t MPU6050_IsTilted(const Mpu6050Raw *data);

#endif
