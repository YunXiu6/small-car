#ifndef IR_H
#define IR_H

#include "stm32f10x.h"

typedef enum
{
    IR_KEY_NONE = 0,
    IR_KEY_FORWARD,
    IR_KEY_BACKWARD,
    IR_KEY_LEFT,
    IR_KEY_RIGHT,
    IR_KEY_STOP,
    IR_KEY_MODE,
    IR_KEY_AUTO,
    IR_KEY_FORWARD_TOGGLE,
    IR_KEY_SPEED_UP,
    IR_KEY_SPEED_DOWN
} IrKey;

void IR_Init(void);
uint8_t IR_ReadRaw(uint32_t *code);
IrKey IR_MapKey(uint32_t code);

extern volatile uint32_t g_ir_raw_code;
extern volatile uint8_t g_ir_cmd_code;
extern volatile uint8_t g_ir_key;
extern volatile uint32_t g_ir_rx_count;
extern volatile uint32_t g_ir_edge_count;
extern volatile uint8_t g_ir_level;
extern volatile uint32_t g_ir_last_dt_us;
extern volatile uint8_t g_ir_bits;
extern volatile uint8_t g_ir_state;

#endif
