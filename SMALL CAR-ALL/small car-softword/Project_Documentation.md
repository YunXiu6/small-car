# Small Car Firmware

This firmware drives the STM32F103C8T6 smart car hardware from the schematic and XMind architecture.

## Hardware Mapping

- TB6612FNG:
  - PA0 = PWMA
  - PA1 = PWMB
  - PB12 = AIN1
  - PB13 = AIN2
  - PB14 = BIN1
  - PB15 = BIN2
- OLED software I2C:
  - PB8 = SCL
  - PB9 = SDA
- MPU6050 I2C2:
  - PB10 = SCL
  - PB11 = SDA
- Ultrasonic:
  - PA5 = Trig
  - PA6 = Echo
- Encoder:
  - Disabled by default because the schematic motor connectors expose only two motor wires.
  - Set `CAR_USE_ENCODER` to `1` in `Config/car_config.h` only after wiring a separate encoder interface.
- USART1:
  - PA9 = TX
  - PA10 = RX
- Infrared receiver:
  - PB1 = DATA
- Status LED:
  - PC13

## Behavior

- Starts in automatic obstacle-avoidance mode.
- Stops or turns when the ultrasonic distance is below the configured threshold.
- IR or serial commands switch the car into remote-control mode.
- MPU6050 tilt detection stops the motors.
- OLED and USART show mode, distance and encoder status.

## Serial Commands

- `w`: forward
- `s`: backward
- `a`: turn left
- `d`: turn right
- `x`: stop
- `m`: switch mode
- `+`: increase speed
- `-`: decrease speed
