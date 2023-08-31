#include "Chip-8.h"
#include <time.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
bool RUN = true;
bool drawFlag = false;
uint16_t inst = 0;
uint8_t pixelBuffer[15];
uint8_t xCoord;
uint8_t initXCoord;
uint8_t yCoord;
bool displayInit(void){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    // error initializing
    return false;
  }
    window = SDL_CreateWindow(
    "SDL2 Window",             // Title
    SDL_WINDOWPOS_CENTERED,   // X position
    SDL_WINDOWPOS_CENTERED,   // Y position
    960,                      // Width, 15 * original chip-8 width (truncated)
    480,                      // Height 15 * original chip-8 height (truncated)
    SDL_WINDOW_SHOWN          // Flags (e.g., SDL_WINDOW_SHOWN, SDL_WINDOW_FULLSCREEN, etc.)
);

    if (window == NULL) {
        // Handle window creation error
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer){
        return false;
    }

    return true;
};

int isValidKeyPress(char K){
    for(int i = 0; i<15; i++){
        if(K == validKeys[i]) return i;
    }
    return -1;
}
bool updateDisplay(Chip_8* CPU){
    SDL_Event event;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            quit = true;
            drawFlag = false;
            RUN = false;
        }
        // Handle other event types here (e.g., keyboard, mouse)
    }
    if(drawFlag){
        SDL_Delay(1);
        SDL_Rect rect;
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);
        for(int row= 0; row< 32; row++){ // height
            for(int col=0; col<64; col++){ // width
                if(CPU->DISPLAY[row][col] == 1){
                    rect.x = col * 15;
                    rect.y = row * 15;
                    rect.w = 15;
                    rect.h = 15;
                    SDL_SetRenderDrawColor(renderer, 0x00,0xFF,0x00,0xFF);
                    SDL_RenderDrawRect(renderer, &rect);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderPresent(renderer);
    }
    const char* errorMsg = SDL_GetError();
    if(strlen(errorMsg) > 0){
        printf(">>>%s\n", errorMsg);
    }
    drawFlag = false;
    return true;
}
}

bool loadBuiltInSprites(Chip_8* CPU){
    int k = 0;
    for(int i = 0; i < 15; i++){
        for(int j = 0; j < 5; j++){
            CPU->Chip_8_Ram[k] = builtInSprites[i][j];
            k++;
        }
    }
    return true;
}
bool loadRom(Chip_8* CPU, char* romName){
    // open the chip_8 file 
    // iterate through the file and add each line into memory starting at 0x200
    // set PC to 0x200
    FILE* ROM;
    char romString[3];
    ROM = fopen(romName, "rb");
    if(ROM == NULL)return false;
    uint32_t index = 0;
    while(fgets(romString, 3, ROM)){
        CPU->Chip_8_Ram [0x200 + index] = romString[0];
        CPU->Chip_8_Ram [0x200 + index + 1] = romString[1];
        index+=2;
    }
    CPU->PC = 0x200;
    return true;
}

void interpret(Chip_8* CPU){
    inst = (CPU->Chip_8_Ram[CPU->PC] << 8) | (CPU->Chip_8_Ram[CPU->PC + 1]);
    srand(time(NULL));
    uint8_t n; // number of bytes for sprite;
    uint8_t random_Number;
    uint8_t pixel; // used for draw instruction
    CPU->PC = CPU->PC + 2;
    if(inst){
        printf("First Nibble:%d Inst Value:%d\n", MSN(inst), inst);
        switch (MSN(inst))
        {
        case 0x0:
            // Clear Instruction
            printf("Instruction found\n"); // debugging
            if((inst & 0xfff) == 0x00e0){
                for(int row= 0; row< 32; row++){ // height
                    for(int col=0; col<64; col++){ // width
                        CPU->DISPLAY[row][col] = 0;
                    } // set the display to zero
                }
            }
            else if((inst & 0xfff) == 0x00ee){ // RET instruction
                CPU->PC = CPU->STACK[--CPU->SP];
            }
            break;
        case 0x1:
            printf("Instruction found\n"); // debugging
            CPU->PC = (inst & 0x0FFF); // Jmp Instruction
            break;
        case 0x2:
            //2nnn - CALL addr Call subroutine at nnn.
            printf("Instruction found\n"); // debugging
            CPU->STACK[CPU->SP++] = CPU->PC; // Call instruction
            CPU->PC = (inst & 0x0fff);
            break;
        case 0x3:
            // SE Vx, byte: Skip next instruction if Vx = kk. (3xkk)
            printf("Instruction found\n"); // debugging
            CPU->PC = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] == (inst & 0x00FF) ? CPU->PC + 2 : CPU->PC; 
            break;
        case 0x4:
            //4xkk - SNE Vx, byte: Skip next instruction if Vx != kk.
            printf("Instruction found\n"); // debugging
            CPU->PC = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] != (inst & 0x00FF) ? CPU->PC + 2 : CPU->PC; 
            break;
        case 0x5:
            //5xy0 - SE Vx, Vy: Skip next instruction if Vx = Vy.
            printf("Instruction found\n"); // debugging
            CPU->PC = CPU->Chip_8_Reg[(inst & 0x0F00 ) >> 8] == CPU->Chip_8_Reg[(inst & 0x00F0) >> 4]? CPU->PC + 2 : CPU->PC; 
            break;
        case 0x6:
            //6xkk - LD Vx, byte: Set Vx = kk.
            printf("Instruction found\n"); // debugging
            CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = (inst & 0x00FF);
            break;
        case 0x7:
            //7xkk - ADD Vx, byte: Set Vx = Vx + kk.
            printf("Instruction found\n"); // debugging
            CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] += (inst & 0x00FF);
            break;
        
        case 0x8:
            printf("Instruction found\n"); // debugging
            if((inst & 0x000F) == 0){
                // 8xy0 - LD Vx, Vy: Set Vx = Vy.
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = CPU->Chip_8_Reg[(inst & 0x0F0) >> 4];
            }

            else if((inst & 0x000F) == 1){
                // 8xy1 - OR Vx, Vy Set Vx = Vx OR Vy.
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] |= CPU->Chip_8_Reg[(inst & 0x0F0) >> 4];
            }

            else if((inst & 0x000F) == 2){
                //8xy2 - AND Vx, Vy  Set Vx = Vx AND Vy.
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] &= CPU->Chip_8_Reg[(inst & 0x0F0) >> 4];
            }

            else if((inst & 0x000F) == 3){
                // 8xy3 - XOR Vx, Vy Set Vx = Vx XOR Vy.
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] ^= CPU->Chip_8_Reg[(inst & 0x0F0) >> 4];
            }

            else if((inst & 0x000F) == 4){
                // 8xy4 - ADD Vx, Vy: Set Vx = Vx + Vy, set VF (Reg 15) = carry.
                int temp  = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] + CPU->Chip_8_Reg[(inst & 0x0F0) >> 4] > 255 ? 1 : 0;
                uint8_t res = (CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] + CPU->Chip_8_Reg[(inst & 0x0F0) >> 4]) & 0x00FF;
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = res;
                CPU->Chip_8_Reg[15] = temp;
            }

            else if((inst & 0x000F) == 5){
                // 8xy5 - SUB Vx, Vy Set Vx = Vx - Vy, set VF = NOT borrow.
                int temp = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] > CPU->Chip_8_Reg[(inst & 0x0F0) >> 4] ? 1 : 0;
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] -= CPU->Chip_8_Reg[(inst & 0x0F0) >> 4];
                CPU->Chip_8_Reg[15] = temp;
            }

            else if((inst & 0x000F) == 6){
                // 8xy6 - SHR Vx {, Vy} Set Vx = Vx SHR 1.
                int temp = (CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] & 0x01) == 1 ? 1 : 0;
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] >> 1;
                CPU->Chip_8_Reg[15] = temp;
            }

            else if((inst & 0x000F) == 7){
                // 8xy7 - SUBN Vx, Vy Set Vx = Vy - Vx, set VF = NOT borrow.
                int temp = CPU->Chip_8_Reg[(inst & 0x00F0) >> 4] > CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] ? 1 : 0;
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = CPU->Chip_8_Reg[(inst & 0x00F0) >> 4] -  CPU->Chip_8_Reg[(inst & 0x0F00) >> 8];
                CPU->Chip_8_Reg[15] = temp;
            }

            else if((inst & 0x000F) == 0xE){
                // 8xyE - SHL Vx {, Vy} Set Vx = Vx SHL 1.
                int temp = (CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] & 0x80) == 128 ? 1 : 0;
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] << 1;
                CPU->Chip_8_Reg[15] = temp;
            }
            
            break;
        
        case 0x9:
            // 9xy0 - SNE Vx, Vy Skip next instruction if Vx != Vy.
            printf("Instruction found\n"); // debugging
            CPU->PC = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] != CPU->Chip_8_Reg[(inst & 0x0F0) >> 4] ? CPU->PC + 2 : CPU->PC;
            break;

        case 0xA:
            //Annn - LD I, addr Set I = nnn. (index register)
            printf("Instruction found\n"); // debugging
            CPU->Index_Reg = (inst & 0x0FFF);
            break;

        case 0xB:
            //Bnnn - JP V0, addr Jump to location nnn + V0.
            printf("Instruction found\n"); // debugging
            CPU->PC = (inst & 0x0FFF) + CPU->Chip_8_Reg[0];
            break;

        case 0xC:
            // Cxkk - RND Vx, byte Set Vx = random byte AND kk.
            printf("Instruction found\n"); // debugging
            CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = (rand() &  (inst & 0x00FF));
            break;

        case 0xD:
            // Dxyn - DRW Vx, Vy, nibble Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            n = (inst & 0x000F);
            CPU->Chip_8_Reg[0xF] = 0;
            // get x and y coordinates
            initXCoord = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] % 64; // using modulo to account for wrapping
            yCoord = CPU->Chip_8_Reg[(inst & 0x00F0) >> 4] % 32;
            // clear the pixel buffer
            memset(pixelBuffer, 0, 15 * sizeof(char));
            // load the sprite into the pixelBuffer starting from the address in index_reg
            for(int i = 0; i < n; i++){
                pixelBuffer[i] = CPU->Chip_8_Ram[CPU->Index_Reg + i];
            }

            for(int i = 0; i < n; i++){
                if(yCoord > 31)break;
                xCoord = initXCoord;
                for(int j = 0; j < 8; j++){
                    if(xCoord > 63)break;
                    pixel = (pixelBuffer[i] & 0x80) >> 7;
                    if(CPU->DISPLAY[yCoord][xCoord] == 1 && pixel == 1){
                        CPU->Chip_8_Reg[0xF] = 1;
                    }
                    CPU->DISPLAY[yCoord][xCoord] = CPU->DISPLAY[yCoord][xCoord] ^ pixel;
                    pixelBuffer[i] = pixelBuffer[i] << 1;
                    xCoord++;
                }
                yCoord++;

            }
            drawFlag = true;
            break;

        case 0xE:
            if((inst & 0x00FF) == 0x009E){
                //Ex9E - SKP Vx Skip next instruction if key with the value of Vx is pressed.
                if(CPU->KEYBOARD[CPU->Chip_8_Reg[(inst & 0x0f00) >> 8] & 0x00FF] == 1)CPU->PC+=2;
            }
            else if((inst & 0x00FF) == 0x00A1){
                // ExA1 - SKNP Vx Skip next instruction if key with the value of Vx is not pressed.
                if(CPU->KEYBOARD[CPU->Chip_8_Reg[(inst & 0x0f00) >> 8] & 0x00FF] == 0)CPU->PC+=2;
            }
            break;

        case 0xF:
            if((inst & 0x00FF) == 0x0007){
                //Fx07 - LD Vx, DT Set Vx = delay timer value.
               CPU->Chip_8_Reg[(inst & 0x0f00) >> 8] = CPU->DT_Reg;
            }
            else if((inst & 0x00FF) == 0x000A){
                //Fx0A - LD Vx, K Wait for a key press, store the value of the key in Vx.
                char K = NULL;
                int keyIndex = -1;
                while(keyIndex < 0){
                    scanf(&K);
                    keyIndex = isValidKeyPress(K);
                }
                memset(CPU->KEYBOARD, 0, sizeof(char) * 16);
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = K;
                CPU->KEYBOARD[keyIndex] = 1;
            }
            else if((inst & 0x00FF) == 0x0015){
                //Fx15 - LD DT, Vx Set delay timer = Vx.
                CPU->DT_Reg = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8];
            }

            else if((inst & 0x00FF) == 0x0018){
                //Fx18 - LD ST, Vx Set sound timer = Vx.
                CPU->ST_Reg = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8];
            }

            else if((inst & 0x00FF) == 0x001E){
                //Fx1E - ADD I, Vx Set I = I + Vx.
                CPU->Index_Reg = CPU->Index_Reg + CPU->Chip_8_Reg[(inst & 0x0F00) >> 8];
            }

            else if((inst & 0x00FF) == 0x0029){
                //Fx29 - LD F, Vx Set I = location of sprite for digit corresponding to the value of Vx.
                CPU->Index_Reg = spriteAddresses[CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] & 0x00FF];
            }

            else if((inst & 0x00FF) == 0x0033){
                // Fx33 - LD B, Vx Store BCD representation of Vx in memory locations I, I+1, and I+2.
                uint8_t digits = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8];
                uint8_t hundrendsPlace = (digits / 100) % 10;
                uint8_t tensPlace = (digits / 10) % 10;
                uint8_t onesPlace = (digits / 1) % 10;
                CPU->Chip_8_Ram[CPU->Index_Reg] = hundrendsPlace;
                CPU->Chip_8_Ram[CPU->Index_Reg + 1] = tensPlace;
                CPU->Chip_8_Ram[CPU->Index_Reg + 2] = onesPlace;
            }

            else if((inst & 0x00FF) == 0x0055){
                // Fx55 - LD [I], Vx Store registers V0 through Vx in memory starting at location I.
                int n = (inst & 0x0F00) >> 8;
                for(int i =0; i <= n; i++){
                    CPU->Chip_8_Ram[CPU->Index_Reg + i] = CPU->Chip_8_Reg[i];
                }
            }
            else if((inst & 0x00FF) == 0x0065){
                // Fx65 - LD Vx, [I] Read registers V0 through Vx from memory starting at location I.
                int n = (inst & 0x0F00) >> 8;
                for(int i =0; i <= n; i++){
                    CPU->Chip_8_Reg[i] = CPU->Chip_8_Ram[CPU->Index_Reg + i];
                }

            }

            else printf("Instruction with prefix 0xF not found\n");
            break;
        
        default:
            printf("Instruction not found\n");
        }
    }
};

int main(int argc, char* argv[]){
    bool romStatus;
    bool displayStatus;
    bool spritesStatus;
    char* romName = argv[1];
    Chip_8* CPU = (Chip_8*)malloc(sizeof(Chip_8));
    spritesStatus = loadBuiltInSprites(CPU);
    if(!spritesStatus){
         printf("Error loading built-in sprites into memory");
         return 0;
    }
    romStatus = loadRom(CPU, romName);
    displayStatus = displayInit();
    if(!romStatus){
        printf("Error loading rom into memory");
        return 0;
    }

    if(!displayStatus){
        printf("Error initializing display");
        return 0;
    }
    while(RUN){
        // interpret
        //update the display if needed
        interpret(CPU);
        displayStatus = updateDisplay(CPU);
        if(!displayStatus){
            printf("Error updating display");
            break;
        }
         if (CPU->DT_Reg > 0) { 
            uint32_t count = 17200000;
            while(count > 0){
                count-=1;
            }
            CPU->DT_Reg--; 
        }
    }
    free(CPU);
    return 0;
};