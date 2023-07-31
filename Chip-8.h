#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    uint8_t Chip_8_Ram[4096]; // 4kb RAM
    uint8_t Chip_8_Reg[16]; // 16 general purpose 8-bit registers
    uint8_t DT_Reg; // Delay timer register
    uint8_t ST_Reg; // Sound timer register
    uint16_t Index_Reg[2]; // Index register used to store memory addresses
    uint16_t PC; // Program counter
    uint8_t SP; // stack pointer
    uint16_t STACK[16]; // the stack
} Chip_8;

// Loads the Chip 8 ROM into memory(RAM)
bool loadRom(Chip_8* CPU);