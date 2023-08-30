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
    uint8_t DISPLAY[32][64]; // 64x32 pixel display
    uint8_t KEYBOARD[15]; // 16-key hex keypad
} Chip_8;

uint8_t builtInSprites[16][5] = {
    {0xF0,0x90,0x90,0x90,0xF0}, // 0
    {0x20,0x60,0x20,0x20,0x70}, // 1
    {0xF0,0x10,0xF0,0x80,0xF0}, // 2
    {0xF0,0x10,0xF0,0x10,0xF0}, // 3
    {0x90,0x90,0xF0,0x10,0x10}, // 4
    {0xF0,0x80,0xF0,0x10,0xF0}, // 5
    {0xF0,0x80,0xF0,0x90,0xF0}, // 6
    {0xF0,0x10,0x20,0x40,0x40}, // 7
    {0xF0,0x90,0xF0,0x90,0xF0}, // 8
    {0xF0,0x90,0xF0,0x10,0xF0}, // 9
    {0xF0,0x90,0xF0,0x90,0x90}, // A
    {0xE0,0x90,0xE0,0x90,0xE0}, // B
    {0xF0,0x80,0x80,0x80,0xF0}, // C
    {0xE0,0x90,0x90,0x90,0xE}, // D
    {0xF0,0x80,0xF0,0x80,0xF0}, // E
    {0xF0,0x80,0xF0,0x80,0x80} // F
};
uint8_t spriteAddresses[15] = {0x00, 0x05, 0x0A, 0x0F, 0x14, 0x19, 0x1E, 0x23, 0x28, 0x2D, 0x32, 0x37, 0x3C, 0x41, 0x46};
char validKeys[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
// Loads the Chip 8 ROM into memory(RAM) and sets PC to 0x200
bool loadRom(Chip_8* CPU, char* romName);

//Interpret Chip-8 ROM
void interpret(Chip_8* CPU);

// Initialize display
bool displayInit(void);
// updates the display

bool updateDisplay(Chip_8* CPU);

// load built in sprites into memory
bool loadBuiltInSprites(Chip_8* CPU);
// determines if a key press is valid;
bool isValidKeyPress(char K);