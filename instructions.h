#ifndef INSTRUCTIONS
#define INSTRUCTIONS


#include "chip8.h"
#include <stdint.h>
#include <stdbool.h>

//For the Add instruction
#define CARRY_MODE 1
#define NO_CARRY 2

//For the sub instruction
#define SUB_1 0
#define SUB_2 1

//SHIFT
#define LEFT 0
#define RIGHT 1

//For the Set instruction
#define V_MODE 0
#define I_MODE 1
#define V_V_MODE 2

//JUMP MODES
#define JP_1 1
#define JP_3 3
#define JP_4 4
#define JP_5 5
#define JP_9 9
#define SUBROUTINE 2


//DATA ACCESS
#define ADDRESS 0
#define VALUE 1
#define X_D 2
#define Y_D 3
#define N_D 4


#define OR(a, b) (a | b)
#define AND(a, b) (a & b)
#define XOR(a, b) (a ^ b)

uint16_t mergeNibbles(uint16_t sliced[], int number);

void Clear(CHIP8 *c8);
void Jump(CHIP8 *c8, uint16_t data[], uint16_t offset, int mode);
void Call(CHIP8 *c8);
void Return(CHIP8 *c8);
void Skip(CHIP8 *c8, bool condition, int mode);
void Set(CHIP8 *c8, uint16_t data[], int mode);
void Add(CHIP8 *c8, uint16_t data[], int mode);
void Sub(CHIP8 *c8, uint16_t data[], int mode);
void Shift(CHIP8 *c8, uint16_t data[], int direction);
void Random(CHIP8 *c8, uint16_t data[]);
void Draw(CHIP8 *c8, uint8_t reg_x, uint8_t reg_y, uint8_t nibble);


#endif
