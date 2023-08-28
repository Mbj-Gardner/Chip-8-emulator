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
    128,                      // Width, 12 * original chip-8 width (truncated)
    64,                      // Height 18 * original chip-8 height (truncated)
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

bool updateDisplay(Chip_8* CPU){
    bool draw = false;
    SDL_Event event;
    SDL_PollEvent(&event);
    if(event.type == SDL_QUIT){
        RUN = false;
        drawFlag = false;
        return true;
    }
    else if(drawFlag){
        SDL_Rect rect;
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0x00,0xFF,0x00,0xFF);
        for(int row= 0; row< 64; row++){
            for(int col=0; col<32; col++){
                if(CPU->DISPLAY[row][col] == 1){
                    draw = true;
                    rect.x = col;
                    rect.y = row;
                    rect.w = 1;
                    rect.h = 1;
                    SDL_RenderDrawRect(renderer, &rect);
                    //col+=12;
                }
            }
            //row+=18;
        }
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderPresent(renderer);
        drawFlag = false;
    }
    return true;
};

bool loadRom(Chip_8* CPU){
    // open the chip_8 file 
    // iterate through the file and add each line into memory starting at 0x200
    // set PC to 0x200
    FILE* ROM;
    char romString[3];
    ROM = fopen("IBM Logo.ch8", "rb");
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
    if(inst){
        printf("First Nibble:%d Inst Value:%d\n", MSN(inst), inst);
        switch (MSN(inst))
        {
        case 0x0:
            // Clear Instruction
            printf("Instruction found\n"); // debugging
            if((inst & 0x00ff) == 0x00e0){
                memset(CPU->DISPLAY, 0, sizeof(CPU->DISPLAY)); // set the display to zero
            }
            else if((inst & 0x00ff) == 0x00ee){ // RET instruction
                CPU->PC = CPU->STACK[CPU->SP];
                CPU->SP--;
            }
            break;
        case 0x1:
            printf("Instruction found\n"); // debugging
            CPU->PC = (inst & 0x0FFF) -2; // Jmp Instruction
            break;
        case 0x2:
            printf("Instruction found\n"); // debugging
            CPU->STACK[++CPU->SP] = CPU->PC; // Call instruction
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
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] += CPU->Chip_8_Reg[(inst & 0x0F0) >> 4];
                CPU->Chip_8_Reg[15] = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] > 255 ? 1 : 0;
            }

            else if((inst & 0x000F) == 5){
                // 8xy5 - SUB Vx, Vy Set Vx = Vx - Vy, set VF = NOT borrow.
                CPU->Chip_8_Reg[15] = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] > CPU->Chip_8_Reg[(inst & 0x0F0) >> 4] ? 1 : 0;
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] -= CPU->Chip_8_Reg[(inst & 0x0F0) >> 4];
            }

            else if((inst & 0x000F) == 6){
                // 8xy6 - SHR Vx {, Vy} Set Vx = Vx SHR 1.
                CPU->Chip_8_Reg[15] = (CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] & 0x01) == 1 ? 1 : 0;
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] >> 1;
            }

            else if((inst & 0x000F) == 7){
                // 8xy7 - SUBN Vx, Vy Set Vx = Vy - Vx, set VF = NOT borrow.
                CPU->Chip_8_Reg[15] = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] < CPU->Chip_8_Reg[(inst & 0x0F0) >> 4] ? 1 : 0;
                CPU->Chip_8_Reg[(inst & 0x0F0) >> 4] -= CPU->Chip_8_Reg[(inst & 0x0F00) >> 8];
            }

            else if((inst & 0x000F) == 0xE){
                // 8xyE - SHL Vx {, Vy} Set Vx = Vx SHL 1.
                CPU->Chip_8_Reg[15] = (CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] & 0x80) == 1 ? 1 : 0;
                CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] << 1;
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
            CPU->PC = (inst & 0x0FFF) + CPU->Chip_8_Reg[0] - 2;
            break;

        case 0xC:
            // Cxkk - RND Vx, byte Set Vx = random byte AND kk.
            printf("Instruction found\n"); // debugging
            random_Number = rand()%((rand_max+1)-rand_min) - 1;
            CPU->PC = CPU->Chip_8_Reg[(inst & 0x0F00) >> 8] = (random_Number &  (inst & 0x00FF));
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
                xCoord = initXCoord;
                for(int j = 0; j < 8; j++){
                    pixel = (pixelBuffer[i] & 0x80) >> 7;
                    if(CPU->DISPLAY[yCoord][xCoord] == 1 && pixel == 1){
                        CPU->DISPLAY[yCoord][xCoord] = 0;
                        CPU->Chip_8_Reg[0xF] = 1;
                    }
                    else if(CPU->DISPLAY[yCoord][xCoord] == 0 && pixel == 1){
                        CPU->DISPLAY[yCoord][xCoord] = 1;
                    }
                    pixelBuffer[i] = pixelBuffer[i] << 1;
                    xCoord++;
                    if(xCoord > 63){
                        break;
                    }
                }
                yCoord++;
                if(yCoord > 31){
                    break;
                }

            }
            drawFlag = true;
            break;

        case 0xE:
            /* code */
            break;
        case 0xF:
            /* code */
            break;
        
        default:
            printf("Instruction not found\n");
        }
        CPU->PC = CPU->PC + 2;
        inst = (CPU->Chip_8_Ram[CPU->PC] << 8) | (CPU->Chip_8_Ram[CPU->PC + 1]);
    }
};

int main(){
    bool romStatus;
    bool displayStatus;
    Chip_8* CPU = (Chip_8*)malloc(sizeof(Chip_8));
    romStatus = loadRom(CPU);
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
    }
    return 0;
};