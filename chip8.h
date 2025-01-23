#ifndef CHIP_8
#define CHIP_8

#include <stddef.h>
#include <stdint.h>

#define MEMORY_SIZE 4096
#define PROGRAM_START 0x200

#define ENABLE 1 //enables execution
#define DISABLE 0

#define STACK_SIZE 16

typedef struct arch{
    char display[64][32];
    uint8_t memory[MEMORY_SIZE];
    uint16_t PC; //program counter
    uint16_t I; //index register
    uint16_t stack[16]; //original interpreters had limited space on the stack; usually at least 16 two-byte entries
    uint8_t delay_timer; //delay timer which is decremented at a rate of 60 Hz (60 times per second) until it reaches 0
    uint8_t sound_timer; //sound timer which functions like the delay timer, but which also gives off a beeping sound as long as it’s not 0
    uint8_t V[16]; //16 8-bit (one byte) general-purpose variable registers numbered 0 through F hexadecimal, ie. 0 through 15 in decimal, called V0 through VF
    char **assembly;
    size_t size;
} CHIP8;

void push(uint16_t element, uint16_t stack[]);
uint16_t pop(uint16_t stack[]);


void init_display(char display[64][32]);

uint16_t Fetch(CHIP8 *c8);
void Decode(CHIP8 *c8,uint16_t instruction, char **assembly, size_t size, int enable);
//void Execute();

void slice_instruction(uint16_t instruction, uint16_t dest[4]); ////Separates each byte of a 4 bytes instruction
void add_assembly(char **assembly, size_t program_size, char assembly_instruction[], ...);

#endif
