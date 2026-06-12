#include "chip8.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FONTSET_SIZE 80
#define FONT_START 0x050
#define ROM_START  0x200

// font set for rendering
const unsigned char fontset[FONTSET_SIZE] = {
     0xF0,  0x90,  0x90,  0x90,  0xF0,		// 0
     0x20,  0x60,  0x20,  0x20,  0x70,		// 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
    0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
    0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
    0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
    0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
    0xF0, 0x80, 0xF0, 0x80, 0x80		// F
};

// Initialize memory
void chip8_init(Chip8 *chip8) 
{
    memset(chip8, 0, sizeof(Chip8)); // Set everything in chip8 struct to 0
    chip8->PC = ROM_START; // Programs start at memory location 0x200

    memcpy(&chip8->memory[FONT_START], fontset, sizeof(fontset)); // Insert fontset into memory

}

// Main cycle
void chip8_cycle(Chip8 *chip8) 
{
    // Fetch opcode
    // First half at PC and shift 8 bits to the left. Second at PC+1. Bitwise OR to create 16 bit opcode
    uint16_t opcode = (chip8->memory[chip8->PC] << 8) | chip8->memory[chip8->PC + 1];
    
    /*Break opcode into parts using a mask.
    Opcode:  0xD  A  B  3
               |  |  |  |
               |  x  y  n
               |
               top nibble (the switch key)

        kk  = lower 8 bits  (y + n combined)  →  0xB3
        nnn = lower 12 bits (x + y + n)       →  0xAB3*/
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;
    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;


    // Switch on first nibble
    switch ((opcode & 0xF000) >> 12)  
    {
        case 0x0:
        // Clear display and set draw flag.
            if (kk == 0xE0){
                memset(chip8->display, 0, sizeof(chip8->display));
                chip8->draw_flag = 1;
            }
        // Return from subroutine
            if (kk == 0xEE){
                chip8->SP--;
                chip8->PC = chip8->stack[chip8->SP];
            }
            chip8->PC += 2;
            break;
        case 0x1:
        // Jump - set PC to address nnn
            chip8->PC = nnn;
            break;
        case 0x2:
        // Call subroutine at address nnn
            chip8->stack[chip8->SP] = chip8->PC;
            chip8->SP++;
            chip8->PC = nnn;
            break;
        case 0x3:
        // Skip if V[x] == kk
            if (chip8->V[x] == kk){
                chip8->PC += 4;
            } else {
                chip8->PC += 2;
            }
            break;
        case 0x4:
        // Skip if V[x] != kk
            if (chip8->V[x] != kk){
                chip8->PC += 4;
            } else {
                chip8->PC += 2;
            }
            break;
        case 0x5:
        // Skip if V[x] == V[y]
            if (chip8->V[x] == chip8->V[y]){
                chip8->PC += 4;
            } else {
                chip8->PC += 2;
            }
            break;
        case 0x6:
        // Set register V[x] to kk
            chip8->V[x] = kk;
            chip8->PC += 2;
            break;
        case 0x7:
        // Add to register V[x] += kk
            chip8->V[x] += kk;
            chip8->PC += 2;
            break;
        case 0x8:
        // ALU opcodes
        {
            int result;
            uint16_t shift_out;
            switch (n) {
                case 0x0:
                    chip8->V[x] = chip8->V[y];
                    break;
                case 0x1:
                    chip8->V[x] |= chip8->V[y];
                    chip8->V[0xF] = 0;
                    break;
                case 0x2:
                    chip8->V[x] &= chip8->V[y];
                    chip8->V[0xF] = 0;
                    break;
                case 0x3:
                    chip8->V[x] ^= chip8->V[y];
                    chip8->V[0xF] = 0;
                    break;
                case 0x4:
                    result = chip8->V[x] + chip8->V[y];
                    // Store lower 8 bits
                    chip8->V[x] = (chip8->V[x] + chip8->V[y]) & 0x00FF;
                    chip8->V[0xF] = 0;
                    // Check for overflow and set flag
                    if (result > 255){
                        chip8->V[0xF] = 1;
                    }                    
                    break;
                case 0x5:
                    result = chip8->V[x] - chip8->V[y];
                    chip8->V[x] -= chip8->V[y];
                    chip8->V[0xF] = 1;
                    if (result < 0){
                        chip8->V[0xF] = 0;
                    }
                    break;
                case 0x6:
                    chip8->V[x] = chip8->V[y];
                    // Get shifted out bit
                    shift_out = chip8->V[x] & 0x01;
                    // shift right
                    chip8->V[x] >>= 1;
                    // store shifted out bit
                    chip8->V[0xF] = shift_out;
                    break;
                case 0x7:
                    result = chip8->V[y] - chip8->V[x];
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    chip8->V[0xF] = 1;
                    if (result < 0){
                        chip8->V[0xF] = 0;
                    }
                    break;
                case 0xE:
                    chip8->V[x] = chip8->V[y];
                    // Get shifted out bit
                    shift_out = chip8->V[x] & 0x80;
                    // shift left
                    chip8->V[x] <<= 1;
                    // store shifted out bit
                    chip8->V[0xF] = shift_out >> 7;
                    break;
            }
            chip8->PC += 2;
            break;
        }
        case 0x9:
        // if vX != vY, skip next opcode
            if (chip8->V[x] != chip8->V[y]){
                chip8->PC += 4;
            } else {
                chip8->PC += 2;
            }
            break;
        case 0xA:
        // Set index register I = nnn
            chip8->I = nnn;
            chip8->PC += 2;
            break;
        case 0xB:
        // Jump to address at V[0]
            chip8->PC = nnn + chip8->V[0];
            break;
        case 0xC:
        // Generate a random byte, AND it with kk and store in V[x]
            chip8->V[x] = (rand() & 0xFF) & kk;
            chip8->PC += 2;
            break;
        case 0xD:
        // Draw sprite
        {
            // Get coords
            int disp_x = chip8->V[x] % 64;
            int disp_y = chip8->V[y] % 32;

            // Reset draw flag
            chip8->V[0xF] = 0;

            // Loop over sprite rows
            for (int i=0; i<n; i++){
                // fetch sprite row byte from memory
                uint8_t byte = (uint8_t)chip8->memory[chip8->I + i];
                // loop over row pixels
                for(int j=0; j<8; j++){
                    // detect collision (if drawn pixel is set by sprite, collision has happened)
                    
                    if (disp_x + j <= 63 && disp_y + i <= 31)
                    {
                        if (chip8->display[(disp_y + i) * 64 + disp_x + j] && (byte & (0x80 >> j)))
                        {
                        chip8->V[0xF] = 1;
                        }
                        if (byte & (0x80 >> j))
                        { // XOR display pixel with sprite pixel
                        chip8->display[(disp_y + i) * 64 + disp_x + j] = (chip8->display[(disp_y + i) * 64 + disp_x + j] ^ 1);
                        }
                    }
                }
            }
            chip8->PC += 2;
            break;
        }
        case 0xE:
            switch (kk){
                case 0x9E:
                // Skip if key V[x] is pressed
                    if (chip8->keypad[chip8->V[x]]){
                        chip8->PC += 4;
                    }
                    else{
                        chip8->PC += 2;
                    }                                    
                    break;
                case 0xA1:
                // Skip if key V[x] is NOT pressed
                    if (!chip8->keypad[chip8->V[x]]){
                        chip8->PC += 4;
                    }
                    else {
                        chip8->PC += 2;
                    }
                    break;                     
            }        
            break;
        case 0xF:
            switch (kk){
                case 0x07:
                // Set V[x] to value of delay timer
                    chip8->V[x] = chip8->DT;
                    chip8->PC += 2;
                    break;
                case 0x0A:
                // wait for a keypress, store the key in V[x]
                if (chip8->key_wait == 0) 
                {
                    for (int i=0; i<16; i++)
                    {
                        if (chip8->keypad[i] == 1)
                        {
                            chip8->V[x] = i;
                            chip8->key_wait = 1;
                            break;
                        }
                          
                    }
                }
                if (chip8->key_wait == 1)
                {
                    if (chip8->keypad[chip8->V[x]] == 0)
                    {
                        chip8->key_wait = 0;
                        chip8->PC += 2;
                    }
                } 
                    break;
                case 0x15:
                // Set delay timer to V[x]
                    chip8->DT = chip8->V[x];
                    chip8->PC += 2;
                    break;
                case 0x18:
                // Set sound timer to V[x]
                    chip8->ST = chip8->V[x];
                    chip8->PC += 2;                
                    break;
                case 0x1E:
                // Set I = I + V[x]
                    chip8->I = chip8->I + chip8->V[x];
                    chip8->PC += 2;                
                    break;
                case 0x29:
                // Set I = address of sprite for digit Vx (font sprites)
                    chip8->I = FONT_START + chip8->V[x] * 5;
                    chip8->PC += 2;
                    break;
                case 0x33:
                // Store BCD representation of Vx in memory[I], memory[I+1], memory[I+2]
                    chip8->memory[chip8->I] = chip8->V[x] / 100;
                    chip8->memory[chip8->I+1] = chip8->V[x] % 100 / 10;
                    chip8->memory[chip8->I+2] = chip8->V[x] % 10;
                    chip8->PC += 2;
                    break;
                case 0x55:
                // Store V0–Vx in memory starting at I
                    for (int i = 0; i <= x; i++)
                    {
                        chip8->memory[chip8->I+i] = chip8->V[i];
                    }
                    chip8->I += x+1;
                    chip8->PC += 2;                    
                    break;
                case 0x65: 
                // Read V0–Vx from memory starting at I
                    for (int i = 0; i <= x; i++)
                    {
                        chip8->V[i] = chip8->memory[chip8->I+i];
                    }
                    chip8->I += x+1;
                    chip8->PC += 2;
                    break;
            }
            break;
        default: 
            printf("Unknown opcode: %04X\n", opcode);
    }
}
// Load rom
int chip8_load_rom(Chip8 *chip8, const char *path)
{
    // Open file
    FILE *rom_file = fopen(path, "rb");
    if (rom_file == NULL) {
        printf("Error opening file");
        return -1;
    }

    // Get file size
    fseek(rom_file, 0, SEEK_END);
    unsigned int file_sz = ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET);

    // Check rom fits in memory
    if(file_sz > 4096-0x200) {
        printf("File size too big");
        fclose(rom_file);
        return -1;
    }

    // Read file into memory
    if (fread(&chip8->memory[ROM_START], 1, file_sz, rom_file) < file_sz ) {
        printf("Error reading file");
        fclose(rom_file);
        return -1;
    }

    fclose(rom_file);
    return 0;
}