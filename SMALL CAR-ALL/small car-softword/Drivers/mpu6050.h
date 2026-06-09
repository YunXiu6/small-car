#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f10x.h"

/* Raw 16-bit sensor samples in the register order returned by MPU6050. */
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

/* Initialize I2C2 and wake/configure the MPU6050. Returns 1 when WHO_AM_I matches. */
uint8_t MPU6050_Init(void);
/* Read accelerometer, temperature, and gyro raw registers. */
uint8_t MPU6050_ReadRaw(Mpu6050Raw *data);
/* Simple tilt detector based on the Z-axis acceleration magnitude. */
uint8_t MPU6050_IsTilted(const Mpu6050Raw *data);

#endif
