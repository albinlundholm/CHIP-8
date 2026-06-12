#pragma once

#include <stdint.h>

typedef struct {
    uint8_t memory[4096];      // 4K memory
    uint8_t V[16];             // 16 registers (V0 to VF)
    uint16_t I;                // Index register
    uint16_t PC;               // Program counter
    uint16_t stack[16];        // 16 bit stack for subroutine calls
    uint8_t SP;                // Stack pointer
    uint8_t DT;                // Delay timer
    uint8_t ST;                // Sound timer
    uint8_t display[64 * 32];  // Monochrome display (64x32 pixels)
    uint8_t keypad[16];        // Hexadecimal keypad (16 keys)
    uint8_t draw_flag;         // Flag to indicate when to redraw the screen
    uint8_t key_wait;          // Flag to indicate waiting for keypress
} Chip8;

void chip8_init(Chip8 *chip8);

void chip8_cycle(Chip8 *chip8);

int chip8_load_rom(Chip8 *chip8, const char *path);