#ifndef CAR_CONFIG_H
#define CAR_CONFIG_H

#include "stm32f10x.h"

#define CAR_SYSCLK_HZ             72000000UL
#define CAR_APB1_TIM_HZ           72000000UL
#define CAR_PWM_PERIOD            999U
#define CAR_SPEED_MAX             1000
#define CAR_DEFAULT_SPEED         450
#define CAR_TURN_SPEED            420
#define CAR_OBSTACLE_STOP_CM      18U
#define CAR_OBSTACLE_CLEAR_CM     28U
#define CAR_REMOTE_TIMEOUT_MS     900U
#define CAR_US_MIN_VALID_CM       3U
#define CAR_US_CLOSE_CONFIRM      2U

#define PIN_MASK(pin)             (1U << (pin))

/* TB6612FNG: PA0/PA1 PWM, PB12-PB15 direction. */
#define MOTOR_PWM_PORT            GPIOA
#define MOTOR_PWMA_PIN            0U
#define MOTOR_PWMB_PIN            1U
#define MOTOR_DIR_PORT            GPIOB
#define MOTOR_AIN1_PIN            12U
#define MOTOR_AIN2_PIN            13U
#define MOTOR_BIN1_PIN            14U
#define MOTOR_BIN2_PIN            15U

/* Ultrasonic module: PA4 = Trig, PA5 = Echo on the current wiring. */
#define US_PORT                   GPIOA
#define US_TRIG_PIN               4U
#define US_ECHO_PIN               5U

/* Encoder is optional. The schematic motor connectors expose only two motor wires. */
#define CAR_USE_ENCODER           0
#define ENCODER_PORT              GPIOA
#define ENCODER_A_PIN             3U
#define ENCODER_B_PIN             4U

/* USART1 debug header. */
#define USART_TX_PORT             GPIOA
#define USART_RX_PORT             GPIOA
#define USART_TX_PIN              9U
#define USART_RX_PIN              10U
#define USART_BAUDRATE            9600UL

/* OLED: software I2C. */
#define OLED_PORT                 GPIOB
#define OLED_SCL_PIN              8U
#define OLED_SDA_PIN              9U
#define OLED_ADDR                 0x78U

/* MPU6050: I2C2. */
#define MPU_I2C                   I2C2
#define MPU_PORT                  GPIOB
#define MPU_SCL_PIN               10U
#define MPU_SDA_PIN               11U
#define MPU6050_ADDR              0xD0U

/* Infrared receiver. The schematic labels the signal Infrared ray on PB1. */
#define IR_PORT                   GPIOB
#define IR_PIN                    1U

/* On-board status LED. */
#define LED_PORT                  GPIOC
#define LED_PIN                   13U

/* Optional key inputs: PA11 and PA12 are free in the schematic. */
#define KEY_PORT                  GPIOA
#define KEY1_PIN                  11U
#define KEY2_PIN                  12U

#endif
