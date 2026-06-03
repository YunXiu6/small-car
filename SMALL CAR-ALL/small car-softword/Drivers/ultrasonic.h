#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "stm32f10x.h"

typedef enum
{
    ULTRASONIC_OK = 0,
    ULTRASONIC_NO_ECHO,
    ULTRASONIC_ECHO_STUCK_HIGH,
    ULTRASONIC_ECHO_TIMEOUT,
    ULTRASONIC_TOO_CLOSE
} UltrasonicStatus;

extern volatile uint16_t g_ultrasonic_distance_cm;
extern volatile uint32_t g_ultrasonic_echo_width_us;
extern volatile uint32_t g_ultrasonic_trig_count;
extern volatile uint8_t g_ultrasonic_status;
extern volatile uint8_t g_ultrasonic_echo_level;

void Ultrasonic_Init(void);
uint16_t Ultrasonic_ReadCm(void);
UltrasonicStatus Ultrasonic_ReadCmEx(uint16_t *distance_cm);

#endif
