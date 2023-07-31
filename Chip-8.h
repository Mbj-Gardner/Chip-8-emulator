#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    uint8_t Chip_8_Ram[4096]; // 4kb RAM
    uint8_t Chip_8_Reg[16]; // 16 general purpose 8-bit registers
    uint8_t DT_Reg[1]; // Delay timer register
    uint8_t ST_Reg[1]; // Sound timer register
    uint16_t Index_Reg[2]; // Index register used to store memory addresses
    uint16_t PC[1]; // Program counter
    uint8_t SP[1]; // stack pointer
    uint16_t STACK[16]; // the stack
} Chip_8;