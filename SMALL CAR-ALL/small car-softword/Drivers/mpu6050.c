#include "mpu6050.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "car_config.h"

#define MPU_REG_SMPLRT_DIV     0x19U
#define MPU_REG_CONFIG         0x1AU
#define MPU_REG_GYRO_CONFIG    0x1BU
#define MPU_REG_ACCEL_CONFIG   0x1CU
#define MPU_REG_ACCEL_XOUT_H   0x3BU
#define MPU_REG_PWR_MGMT_1     0x6BU
#define MPU_REG_WHO_AM_I       0x75U

static uint8_t wait_flag(volatile uint16_t *reg, uint16_t mask, uint8_t set)
{
    uint32_t timeout = 50000U;
    while (timeout--)
    {
        if (((*reg & mask) ? 1U : 0U) == set)
        {
            return 1U;
        }
    }
    return 0U;
}

static void i2c2_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

    Bsp_GpioConfig(MPU_PORT, MPU_SCL_PIN, GPIO_MODE_AF_OD_50MHZ);
    Bsp_GpioConfig(MPU_PORT, MPU_SDA_PIN, GPIO_MODE_AF_OD_50MHZ);

    I2C2->CR1 = I2C_CR1_SWRST;
    I2C2->CR1 = 0U;
    I2C2->CR2 = 36U;
    I2C2->CCR = 180U;    /* 100kHz on 36MHz PCLK1. */
    I2C2->TRISE = 37U;
    I2C2->CR1 = I2C_CR1_PE;
}

static uint8_t i2c_write(uint8_t dev, uint8_t reg, uint8_t value)
{
    volatile uint16_t tmp;

    if (I2C2->SR2 & I2C_SR2_BUSY)
    {
        return 0U;
    }
    I2C2->CR1 |= I2C_CR1_START;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_SB, 1U)) return 0U;
    I2C2->DR = dev;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_ADDR, 1U)) return 0U;
    tmp = I2C2->SR2; (void)tmp;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_TXE, 1U)) return 0U;
    I2C2->DR = reg;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_TXE, 1U)) return 0U;
    I2C2->DR = value;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_BTF, 1U)) return 0U;
    I2C2->CR1 |= I2C_CR1_STOP;
    return 1U;
}

static uint8_t i2c_read(uint8_t dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    volatile uint16_t tmp;
    uint8_t i;

    if (len == 0U || (I2C2->SR2 & I2C_SR2_BUSY))
    {
        return 0U;
    }

    I2C2->CR1 |= I2C_CR1_START;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_SB, 1U)) return 0U;
    I2C2->DR = dev;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_ADDR, 1U)) return 0U;
    tmp = I2C2->SR2; (void)tmp;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_TXE, 1U)) return 0U;
    I2C2->DR = reg;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_BTF, 1U)) return 0U;

    I2C2->CR1 |= I2C_CR1_START;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_SB, 1U)) return 0U;
    I2C2->DR = dev | 1U;
    if (!wait_flag(&I2C2->SR1, I2C_SR1_ADDR, 1U)) return 0U;
    I2C2->CR1 |= I2C_CR1_ACK;
    tmp = I2C2->SR2; (void)tmp;

    for (i = 0U; i < len; i++)
    {
        if (i == (uint8_t)(len - 1U))
        {
            I2C2->CR1 &= (uint16_t)~I2C_CR1_ACK;
            I2C2->CR1 |= I2C_CR1_STOP;
        }
        if (!wait_flag(&I2C2->SR1, I2C_SR1_RXNE, 1U)) return 0U;
        buf[i] = (uint8_t)I2C2->DR;
    }
    I2C2->CR1 |= I2C_CR1_ACK;
    return 1U;
}

uint8_t MPU6050_Init(void)
{
    uint8_t id = 0U;
    i2c2_init();
    Bsp_DelayMs(100U);
    (void)i2c_read(MPU6050_ADDR, MPU_REG_WHO_AM_I, &id, 1U);
    if (!i2c_write(MPU6050_ADDR, MPU_REG_PWR_MGMT_1, 0x00U)) return 0U;
    (void)i2c_write(MPU6050_ADDR, MPU_REG_SMPLRT_DIV, 0x07U);
    (void)i2c_write(MPU6050_ADDR, MPU_REG_CONFIG, 0x03U);
    (void)i2c_write(MPU6050_ADDR, MPU_REG_GYRO_CONFIG, 0x18U);
    (void)i2c_write(MPU6050_ADDR, MPU_REG_ACCEL_CONFIG, 0x10U);
    return (id == 0x68U || id == 0x69U) ? 1U : 0U;
}

uint8_t MPU6050_ReadRaw(Mpu6050Raw *data)
{
    uint8_t buf[14];
    if (!i2c_read(MPU6050_ADDR, MPU_REG_ACCEL_XOUT_H, buf, sizeof(buf)))
    {
        return 0U;
    }
    data->ax = (int16_t)((buf[0] << 8) | buf[1]);
    data->ay = (int16_t)((buf[2] << 8) | buf[3]);
    data->az = (int16_t)((buf[4] << 8) | buf[5]);
    data->temp = (int16_t)((buf[6] << 8) | buf[7]);
    data->gx = (int16_t)((buf[8] << 8) | buf[9]);
    data->gy = (int16_t)((buf[10] << 8) | buf[11]);
    data->gz = (int16_t)((buf[12] << 8) | buf[13]);
    return 1U;
}

uint8_t MPU6050_IsTilted(const Mpu6050Raw *data)
{
    int16_t z = data->az;
    return (z < 2500 && z > -2500) ? 1U : 0U;
}
