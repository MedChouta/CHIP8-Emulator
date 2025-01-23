#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "instructions.h"
#include "chip8.h"

uint16_t mergeNibbles(uint16_t sliced[], int number){

    //merges the nibbles starting from the least signficant bit
    uint16_t byte = 0;

    for(int i = 0; i < number; i++){

        byte += sliced[3 - i] << (i * 4);
    }

    return byte;
}


void Clear(CHIP8 *c8){

 //   printf("\t --Clears screen");

    for(int i = 0; i < 64; i++){
        memset(c8->display[i], 0, 32);
    }

}

void Jump(CHIP8 *c8, uint16_t data[], uint16_t offset, int mode){

    //1NNN: Jump
    //3XNN, 4XNN, 5XY0 and 9XY0: Skip conditionally

    c8->PC-=2;

    uint8_t x = data[X_D];
    uint8_t y = data[Y_D];


    switch(mode){
        case JP_1:
            c8->PC = (uint16_t)(data[ADDRESS] + offset);
        break;
        case JP_3:
            if (c8->V[x] == data[VALUE])
                c8->PC+=4;
        break;
        case JP_4:
            if (c8->V[x] != data[VALUE])
                c8->PC+=4;
        break;
        case JP_5:
            if(c8->V[x] == c8->V[y])
                c8->PC+=4;
        break;
        case JP_9:
            if(c8->V[x] != c8->V[y])
                c8->PC+=4;
        break;
        case SUBROUTINE:
            push(data[ADDRESS], c8->stack);
            c8->PC = data[ADDRESS];
        break;
    }

}


void Return(CHIP8 *c8){
    uint16_t rtrn = pop(c8->stack);
    c8->PC = rtrn;
}

void Set(CHIP8 *c8, uint16_t data[], int mode){

    //6XNN: Set

    uint8_t x = data[X_D];
    uint8_t y = data[Y_D];

    switch(mode){
        case V_MODE:
            c8->V[x] = data[VALUE];
        break;
        case I_MODE:
            c8->I = data[ADDRESS];
        break;
        case V_V_MODE:
            c8->V[x] = c8->V[y];
        break;
    }

}

void Add(CHIP8 *c8, uint16_t data[], int mode){

    //7XNN: Add

    uint8_t x = data[X_D];
    uint8_t y = data[Y_D];

    switch(mode){
        case CARRY_MODE:
            c8->V[0xF] = 0;
            if ((c8->V[x] + c8->V[y]) > 255)
                c8->V[0xF] = 1;
            c8->V[x] += c8->V[y];
        break;
        case NO_CARRY:
            c8->V[x] += data[VALUE];
        break;
    }

}

void Sub(CHIP8 *c8, uint16_t data[], int mode){
    uint8_t x = data[X_D];
    uint8_t y = data[Y_D];

    c8->V[0xF] = 1;

    switch(mode){
        case SUB_1:
            if(c8->V[x] < c8->V[y])
                c8->V[0xF] = 0;
            c8->V[x] = c8->V[x] - c8->V[y];
        break;
        case SUB_2:
            if(c8->V[y] < c8->V[x])
                c8->V[0xF] = 0;
            c8->V[x] = c8->V[y] - c8->V[x];
        break;
    }
}


void Shift(CHIP8 *c8, uint16_t data[], int direction){

    uint8_t x = data[X_D];
    uint8_t y = data[Y_D];

    c8->V[x] = c8->V[y];

    uint8_t bit = 0;

    switch(direction){
        case LEFT:
        bit = c8->V[x] & 0b00000001;
        c8->V[x] = c8->V[x] >> 1;
        break;
        case RIGHT:
        bit = c8->V[x] & 0b10000000;
        c8->V[x] = c8->V[x] << 1;
        break;
    }

    c8->V[0xF] = bit;
}

void Random(CHIP8 *c8, uint16_t *data){

    uint8_t x = data[X_D];
    srandom(time(NULL));
    uint8_t r = random();

    c8->V[x] = r & data[VALUE];

}

void Draw(CHIP8 *c8, uint8_t reg_x, uint8_t reg_y, uint8_t nibble){

    //printf("\t --Displays pixels");
   // printf("\n========V[%d] = %d============\n", reg_x, c8->V[reg_x]);
    uint8_t x; //VX modulo 64
    uint8_t y = (c8->V[reg_y] & 31); //VY modulo 32
    uint8_t rows = nibble;

    c8->V[0xF] = 0;

    for(int i = 0; i < rows; i++){
        uint8_t sprite = c8->memory[c8->I + i];
       // printf("\n========sprite = %X============\n", sprite);
        x = (c8->V[reg_x] & 63);
        for(int j = 0; j < 8; j++){
            //printf("\n\tX: %d, Y:%d\n", x, y);
            if (sprite & (0b10000000 >> j)){
                if(c8->display[x][y] == 1){
                    c8->display[x][y] = 0;
                }
                else{
                    c8->display[x][y] = 1;
                    c8->V[0xF] = 1;
                }
            }
            x++;
            if(x > 63)
                break;
        }
        y++;
        if(y > 31)
            break;
    }

}
