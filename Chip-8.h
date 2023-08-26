#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>  
// Isolate the most signifcant nibble of an instruction
#define MSN(inst) (inst & 0xF000) >> 12
#define rand_min 0
#define rand_max 255
typedef struct {
    uint8_t Chip_8_Ram[4096]; // 4kb RAM
    uint8_t Chip_8_Reg[16]; // 16 general purpose 8-bit registers
    uint8_t DT_Reg; // Delay timer register
    uint8_t ST_Reg; // Sound timer register
    uint16_t Index_Reg; // Index register used to store memory addresses
    uint16_t PC; // Program counter
    uint8_t SP; // stack pointer
    uint16_t STACK[16]; // the stack
    uint8_t DISPLAY[64][32]; // 64x32 pixel display
} Chip_8;

// Loads the Chip 8 ROM into memory(RAM) and sets PC to 0x200
bool loadRom(Chip_8* CPU);

//Interpret Chip-8 ROM
void interpret(Chip_8* CPU);

// Initialize display
bool displayInit(void);
// updates the display
bool updateDisplay(Chip_8* CPU);