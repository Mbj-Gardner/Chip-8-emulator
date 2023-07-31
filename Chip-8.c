#include "Chip-8.h"

bool loadRom(Chip_8* CPU){
    // open the chip_8 file 
    // iterate through the file and add each line into memory starting at 0x200
    FILE* ROM;
    char romString[3];
    ROM = fopen("test_opcode.ch8", "rb");
    uint32_t index = 0;
    while(fgets(romString, 3, ROM)){
        CPU->Chip_8_Ram [0x200 + index] = romString[0];
        CPU->Chip_8_Ram [0x200 + index + 1] = romString[1];
        index+=2;
    }
    return true;
}

void main(){
    bool status;
    Chip_8* CPU = (Chip_8*)malloc(sizeof(Chip_8));
    status = loadRom(CPU);
}