# Previous Applications Memory

This file records the application functions that existed before the project was reduced to a basic demo.

## Hardware Platform

- MCU: STM32F103C8T6.
- Power path: Type-C VBUS 5V -> TP4056 lithium charger -> 18650 battery VBAT+ -> MT3608 boost to VCC 5V -> 1117-3.3V regulators for 3V3 logic rails.
- Motor driver: TB6612FNG dual H-bridge for two DC motors.
- Display: 0.96 inch I2C OLED.
- Sensors: MPU6050 six-axis IMU, HC-SR04-style ultrasonic ranging module, HX1838 IR receiver.
- Debug/control interfaces: USART1 TX/RX header, battery connector, motor connectors, ultrasonic header.

## Remembered Application Modules

- OLED display:
  - Software I2C on PB8/PB9.
  - Supports strings, numbers, formatted text, images and basic drawing.
  - Used for PWM, infrared data, distance, attitude and debug status display.

- Key input:
  - Four-key scan with debounce.
  - Original pins: PB10, PB11, PA11, PA12 in the older documentation.
  - Used for simple speed increase, speed decrease and stop commands.

- Motor control:
  - Simple single-motor path used PA0 PWM with PB12/PB13 direction pins.
  - PWM range in the test program was -100 to +100.
  - K1 increased speed, K2 decreased speed, K3 stopped the motor.

- TB6612FNG dual-motor driver:
  - PWMA on PA0, PWMB on PA1.
  - AIN1/AIN2 on PB12/PB13.
  - BIN1/BIN2 on PB14/PB15.
  - Intended for left/right differential drive.

- Encoder:
  - TIM3 encoder interface.
  - Inputs on PA6 and PA7.
  - Used for speed or position feedback.

- Ultrasonic ranging:
  - Trig on PA4, Echo on PA5.
  - Trigger pulse longer than 10 us.
  - Echo pulse converted to distance using sound speed timing.
  - Had timeout and filtered distance reading.

- MPU6050:
  - I2C2 on PB10/PB11.
  - Address 0xD0.
  - Reads accelerometer, gyroscope and temperature raw data.
  - Intended for attitude, tilt and motion-state detection.

- HX1838 infrared receiver:
  - Data pin was defined as PB1 in code.
  - Intended to decode remote-control data and show values on OLED.

- USART debug:
  - USART1 on PA9/PA10.
  - Baud rate was 9600.
  - Supported sending bytes, arrays, strings, numbers, printf-style output and RX interrupt reception.

- ADC/RP module:
  - Analog inputs on PA2, PA3, PA4 and PA5.
  - Intended for potentiometer or analog sensor readings.

- Timer service:
  - TIM1 update interrupt used as a 1 ms tick.
  - Drove key debounce scanning and could be reused for simple scheduling.

## Intended Higher-Level Applications

- Remote-control mode using infrared commands.
- Automatic obstacle avoidance using ultrasonic distance.
- Differential-drive movement: forward, backward, left turn, right turn, stop and brake.
- OLED status display for mode, speed, distance, attitude, IR key and errors.
- Safety behavior: motor stop on startup, emergency stop command, PWM limiting, sensor failure handling.

## Important Consistency Notes

- The schematic matches the dual-motor TB6612FNG design better than the old active `main.c`, which only exercised one motor.
- PB10/PB11 conflict between older key documentation and MPU6050 I2C2 usage. The schematic/code for MPU6050 used PB10/PB11 as I2C2.
- If a 5V ultrasonic module is used, Echo level protection should be checked before connecting to the STM32 3.3V input.
- LDO heat and current margin should be checked if motor-related loads draw high current from the 3.3V regulator.
