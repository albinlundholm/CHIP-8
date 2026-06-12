#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.h"

void draw_display(Chip8 *chip8, SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++){
            if (chip8->display[x+y*64]) {
                SDL_RenderFillRect(renderer,&(SDL_Rect){x*10, y*10, 10, 10});
            }
        }
    }
    SDL_RenderPresent(renderer);
    chip8->draw_flag = 0;
}

void set_key(Chip8 *chip8, SDL_Keycode key, int val)
{
    switch (key) {
        case SDLK_x:
            chip8->keypad[0] = val;
            break;
        case SDLK_1:
            chip8->keypad[1] = val;
            break;
        case SDLK_2:
            chip8->keypad[2] = val;
            break;
        case SDLK_3:
            chip8->keypad[3] = val;
            break;
        case SDLK_q:
            chip8->keypad[4] = val;
            break;
        case SDLK_w:
            chip8->keypad[5] = val;
            break;
        case SDLK_e:
            chip8->keypad[6] = val;
            break;
        case SDLK_a:
            chip8->keypad[7] = val;
            break;
        case SDLK_s:
            chip8->keypad[8] = val;
            break;
        case SDLK_d:
            chip8->keypad[9] = val;
            break;
        case SDLK_z:
            chip8->keypad[10] = val;
            break;
        case SDLK_c:
            chip8->keypad[11] = val;
            break;
        case SDLK_4:
            chip8->keypad[12] = val;
            break;
        case SDLK_r:
            chip8->keypad[13] = val;
            break;
        case SDLK_f:
            chip8->keypad[14] = val;
            break;
        case SDLK_v:
            chip8->keypad[15] = val;
            break;
        default: 
            printf("Unknown key: %04X\n", key);
        
    }
}

int main(int argc, char *argv[])
{
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Create and initialize
    Chip8 chip8;
    chip8_init(&chip8);

    // Check correct number of arguments
    if(argc <= 1) {
        printf("Invalid number of arguments. Expected: 1");
        return -1;
    }

    // Load rom
    if (chip8_load_rom(&chip8, argv[1]) == -1) {
        printf("Error loading rom");
        return -1;
    }

    int running = 1;
    uint32_t last_decrement_tick = 0;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN){
                SDL_Keycode key = e.key.keysym.sym;
                set_key(&chip8, key, 1);
            }
            if (e.type == SDL_KEYUP){
                SDL_Keycode key = e.key.keysym.sym;
                set_key(&chip8, key, 0);
            }
        }
        if (SDL_GetTicks() - last_decrement_tick >= 1000/60)
        {
            if (chip8.ST > 0)
            {
                chip8.ST--;
            }
            if (chip8.DT > 0)
            {
                chip8.DT--;
            }
            last_decrement_tick = SDL_GetTicks();
        }
        
        
        chip8_cycle(&chip8);
        draw_display(&chip8, renderer);
        SDL_Delay(1);
    }

    // SDL Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}