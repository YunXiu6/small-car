# STM32 Smart Car Firmware

This repository contains firmware, project files, and hardware reference material for a two-wheel STM32 smart car.

The current firmware is based on the uploaded schematic (`小车原理图.png`) and hardware/software architecture mind map (`组织架构.xmind`). It targets an STM32F103C8T6 and drives a TB6612FNG dual motor driver, OLED display, ultrasonic sensor, MPU6050, infrared receiver, and USART debug interface.

## Repository Contents

```text
.
├── 小车原理图.png
├── 组织架构.xmind
├── 组织架构.backup-before-software-structure-update.xmind
├── SMALL CAR-ALL/
│   └── small car-softword/
│       ├── App/                 # Car application logic
│       ├── Bsp/                 # Board-level GPIO, timing, USART code
│       ├── Config/              # Central pin and behavior configuration
│       ├── Drivers/             # Motor, OLED, ultrasonic, MPU6050, IR, encoder drivers
│       ├── Start/               # STM32F10x startup and CMSIS device files
│       ├── User/                # main.c
│       ├── DebugConfig/         # Keil debug configuration
│       ├── Project.uvprojx      # Keil uVision project
│       ├── Project.uvoptx       # Keil uVision options
│       ├── Project_Documentation.md
│       └── Previous_Applications_Memory.md
└── .gitignore
```

## Firmware Structure

### `User/main.c`

Program entry point. It initializes the car application and continuously runs the application task:

```c
App_CarInit();
while (1) {
    App_CarTask();
}
```

### `Config/car_config.h`

Central hardware and behavior configuration. Important values include:

- Motor PWM range: `0` to `1000`
- Default speed: `450`
- Obstacle stop distance: `18 cm`
- Obstacle clear distance: `28 cm`
- Remote-control timeout: `900 ms`

This file is the first place to edit if hardware wiring or behavior thresholds change.

### `Bsp/`

Board support package:

- `bsp_gpio.*`: GPIO mode setup, read/write/toggle helpers
- `bsp_time.*`: SysTick 1 ms time base and blocking delay helpers
- `bsp_usart.*`: USART1 transmit/receive helpers

### `Drivers/`

Hardware drivers:

- `motor.*`: TB6612FNG dual-motor PWM and direction control
- `ultrasonic.*`: Trig/Echo distance measurement
- `oled.*`: SSD1306-style 0.96 inch OLED software I2C display
- `mpu6050.*`: MPU6050 initialization and raw accelerometer/gyro reads
- `ir.*`: HX1838/NEC infrared polling decoder
- `encoder.*`: optional TIM3 encoder driver, disabled by default

### `App/app_car.c`

Top-level car behavior:

- Starts in automatic obstacle-avoidance mode
- Uses ultrasonic distance to slow, reverse, or turn
- Switches to remote mode when IR or serial commands are received
- Stops the motors if MPU6050 tilt protection triggers
- Displays mode, distance, speed, encoder status, and MPU status on OLED
- Prints status over USART1

## Hardware Pin Mapping

| Module | Signal | STM32 Pin |
|---|---:|---:|
| TB6612FNG | PWMA | PA0 |
| TB6612FNG | PWMB | PA1 |
| TB6612FNG | AIN1 | PB12 |
| TB6612FNG | AIN2 | PB13 |
| TB6612FNG | BIN1 | PB14 |
| TB6612FNG | BIN2 | PB15 |
| OLED | SCL | PB8 |
| OLED | SDA | PB9 |
| MPU6050 | SCL | PB10 |
| MPU6050 | SDA | PB11 |
| Ultrasonic | Trig | PA5 |
| Ultrasonic | Echo | PA6 |
| USART1 | TX | PA9 |
| USART1 | RX | PA10 |
| Infrared receiver | DATA | PB1 |
| Status LED | LED | PC13 |

Encoder support is present but disabled by default with:

```c
#define CAR_USE_ENCODER 0
```

The schematic motor connectors appear to expose only two motor wires. Enable encoder support only after adding separate encoder signal wiring.

## How To Build

1. Install Keil MDK-ARM.
2. Install the STM32F1 device pack if Keil asks for it.
3. Open:

```text
SMALL CAR-ALL/small car-softword/Project.uvprojx
```

4. Select `Target 1`.
5. Build the project.
6. Connect an ST-LINK or compatible programmer.
7. Download the firmware to the STM32F103C8T6.

The project defines `STM32F10X_MD` and uses direct register access through `stm32f10x.h`; it does not require the STM32 Standard Peripheral Library source files.

## How To Use

### Power-On Behavior

After reset:

1. GPIO, SysTick, USART1, motors, ultrasonic, IR, OLED, and MPU6050 are initialized.
2. Motors are stopped during initialization.
3. OLED displays a startup message.
4. The car enters automatic obstacle-avoidance mode.

### Automatic Mode

The car moves forward by default. If the ultrasonic sensor detects an obstacle closer than the configured stop threshold, the car reverses briefly and turns. If the distance is between the stop and clear thresholds, the car slows down.

Relevant configuration:

```c
#define CAR_OBSTACLE_STOP_CM   18U
#define CAR_OBSTACLE_CLEAR_CM  28U
```

### Serial Control

Connect a USB-to-serial adapter to USART1:

- PA9: TX
- PA10: RX
- GND: common ground
- Baud rate: `9600`

Commands:

| Command | Action |
|---:|---|
| `w` | Forward |
| `s` | Backward |
| `a` | Turn left |
| `d` | Turn right |
| `x` | Stop |
| `m` | Switch between automatic and remote mode |
| `+` | Increase speed |
| `-` | Decrease speed |

### Infrared Control

The IR receiver is read from PB1. The decoder expects NEC-style timing. If your remote uses different command values, edit `IR_MapKey()` in:

```text
SMALL CAR-ALL/small car-softword/Drivers/ir.c
```

### OLED Display

The OLED shows:

- Current mode
- Ultrasonic distance
- Speed setting
- Encoder status
- MPU6050 status
- Tilt protection warning

## Important Safety Notes

- Confirm motor direction at low speed before putting the car on the ground.
- If left/right or forward/backward is reversed, adjust `Drivers/motor.c` or swap motor wires.
- If the ultrasonic module outputs 5 V on Echo, add level shifting or voltage division before connecting to the STM32.
- Keep motor power wiring away from I2C and IR signal wires to reduce noise.
- Check LDO and boost converter temperature under motor load.

## Verification Performed

The C source files were syntax-checked with `arm-none-eabi-gcc` for Cortex-M3 using the same include paths and `STM32F10X_MD` device define. Keil command-line build was not run because `UV4/UV5` was not available in the terminal path during development.

