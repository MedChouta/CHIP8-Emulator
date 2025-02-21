#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include "chip8.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 480

int delay = 0;
uint16_t *breakpoint_list;

//SDL_mutex *mut;
//SDL_cond *cond;
atomic_bool run = false;

SDL_Window* window;
SDL_Renderer* renderer;

size_t program_size(FILE *stream); // returns the number of instructions
void load_program(uint8_t memory[], uint8_t raw_bytes[], size_t size);
void* display(SDL_Renderer *renderer, char display[64][32]);
int command(void *c8);
int loop(void *c8);

void add_break(uint16_t breakpoint, uint16_t *breakpoint_list, size_t size);

void clean_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

int main(int argc, char *argv[]){

    CHIP8 chip8;
    memset(chip8.V, 0, 16);
 //   memset(chip8.stack, 0, 16);
    chip8.SP = 0;

    for(int i = 0; i < 16; i++){
        chip8.stack[i] = 0;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL ERROR: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    //display init (temporary):
    init_display(chip8.display);

    FILE *program = fopen(argv[1], "rb");

    if (program == NULL){
        perror("File not found!\n");
        return EXIT_FAILURE;
    }

    size_t size = program_size(program);
    chip8.size = size;

    uint8_t raw_bytes[size*2]; //This will be loaded into memory

    fread(raw_bytes, sizeof(uint8_t), size*2, program);

    load_program(chip8.memory, raw_bytes, size*2);

    char *test = (char*)malloc(sizeof(char)*100);
    chip8.assembly = (char**)malloc(sizeof(char*)*size);
    for(int i = 0; i < size; i++){
        chip8.assembly[i] = NULL;
    }
    printf("size: %d\n", size);
    chip8.PC = 0x200;

    for(int i = 0; i < size; i++){ //disabling execution to add assembly table
        uint16_t instruction = Fetch(&chip8);
        Decode(&chip8, instruction, chip8.assembly, size, DISABLE);
    }

    chip8.PC = 0x200;

    for(int k = 0; k < size; k++){
        uint16_t instruction = Fetch(&chip8);
    }

    chip8.PC = 0x200;

    window = SDL_CreateWindow("CHIP8 EMULATOR",
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    breakpoint_list = (uint16_t*)malloc(sizeof(uint16_t)*size);

    memset(breakpoint_list, 0, size);

    SDL_Thread *com_tr = SDL_CreateThread(&command, "comms", (void*)&chip8);
    SDL_Thread *loop_tr = SDL_CreateThread(&loop, "loop", (void*)&chip8);

    SDL_WaitThread(com_tr, NULL);
    SDL_WaitThread(loop_tr, NULL);

    SDL_Quit();

    for(int i = 0; i < size; i++){
        free(chip8.assembly[i]);
    }
    free(chip8.assembly);
    fclose(program);

    return EXIT_SUCCESS;
}

size_t program_size(FILE *stream){
    size_t i = 0;
    uint8_t temp;
    while (fread(&temp, sizeof(uint8_t), 1, stream)){
        i++;
    }

    rewind(stream);

    return i/2;
}

void load_program(uint8_t memory[], uint8_t raw_bytes[], size_t size){
    uint8_t font[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F
    };

    memset(memory, 0, MEMORY_SIZE);

    for(int i = 0; i < 80; i++){
    //exit    memory[i] = font[i];
    }


    for(int i = 0; i < size; i+=2){

        //we load the instructions while taking the endianness into consideration
        memory[PROGRAM_START + i] = raw_bytes[i];
        memory[PROGRAM_START + i + 1] = raw_bytes[i + 1];
    }
}

void* display(SDL_Renderer *renderer, char display[64][32]){

       SDL_Rect pixels[64][32];

       SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        for(int y = 0; y < 32; y++){
            for(int x = 0; x < 64; x++){
                if(display[x][y] != 0){
                    pixels[x][y].x = x*15;
                    pixels[x][y].y = y*15;
                    pixels[x][y].w = 15;
                    pixels[x][y].h = 15;
                    SDL_RenderFillRect(renderer, &pixels[x][y]);
                }
            }
        }

        SDL_RenderPresent(renderer);

    return NULL;
}

int loop(void *c8){
    SDL_Event e;
    bool quit = false;
    CHIP8 *chip8 = (CHIP8*)c8;

    while(!quit){
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

            if (atomic_load(&run)){
                uint16_t instruction = Fetch(chip8);
                Decode(chip8, instruction, NULL, chip8->size, ENABLE);
            }
            SDL_RenderClear(renderer);

            display(renderer, chip8->display);

            while (SDL_PollEvent(&e)) {
                // Handle quit event (e.g., close button)
                if (e.type == SDL_QUIT) {
                    quit = true;
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                }
            }

            SDL_Delay(delay);

    }

    return 0;

}

int command(void *c8){
    CHIP8 *chip8 = (CHIP8*)c8;

    bool quit = false;
    while(!quit){

    char command[200];
    memset(command, 0, sizeof(command));

    printf("\ndbg> ");
    fgets(command, sizeof(command), stdin);
        if(strcmp(command, "run\n") == 0){
            atomic_store(&run, true);
        }
        else if(strcmp(command, "delay\n") == 0){
            if (!atomic_load(&run)){
                printf("delay: ");
                scanf("%d", &delay);
            }
        }
        else if (strcmp(command, "$V\n") == 0){
            for(int i = 0; i < 16; i++){
                printf("->V%X: 0x%X\n", i,chip8->V[i]);
            }
        }
        else if (strcmp(command, "stack\n") == 0){
            for(int i = 0; i < 16; i++){
                printf("->%d: 0x%X\n", i,chip8->stack[i]);
            }
        }
        else if (strcmp(command, "$PC\n") == 0){
            printf("PC: 0x%02X\n", chip8->PC);
        }
        else if (strcmp(command, "$SP\n") == 0){
            printf("SP: %d\n", chip8->SP);
        }
        else if (strcmp(command, "disas\n") == 0){
            char select;
            for(int i = 0; i < chip8->size; i++){
                int pc = i*2+0x200;
                if (chip8->PC == pc)
                    select = '>';
                else
                    select = '\0';
                printf("%c 0x%X\t%02X%02X\t%s\n", select,pc, chip8->memory[pc],chip8->memory[i*2+1+0x200], chip8->assembly[i]);
            }
        }
        else if (strcmp(command, "exit\n") == 0){
            quit = true;
        }
        else{
            printf("Unknown command\n");
        }
    }

    return 0;
}
