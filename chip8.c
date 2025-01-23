#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include "chip8.h"
#include "instructions.h"

void init_display(char display[64][32]){
    for(int i = 0; i < 64; i++){
        for (int j = 0; j < 32; j++){
            display[i][j] = 0;
        }
    }
}

void push(uint16_t element, uint16_t stack[]){

    for(int i = 0; i < STACK_SIZE; i++){

        if(stack[i] == 0){

            stack[i] = element;

        }
    }

}

uint16_t pop(uint16_t stack[]){

    uint16_t el;

    for(int i = 0; i < STACK_SIZE; i++){

        if(stack[i] == 0){
            el = stack[i - 1];
            stack[i - 1] = 0;
        }
        else if(stack[i] != 0 && i == STACK_SIZE - 1){
            el = stack[i];
            stack[i] = 0;
        }
    }

    return el;

}


uint16_t Fetch(CHIP8 *c8){

    uint16_t PC = c8->PC;

    uint16_t instruction = ((uint16_t)c8->memory[PC] << 8)  + (uint16_t)c8->memory[PC + 1];

    c8->PC+=2;

   // getch();

    return instruction;
}

int msleep(long msec) //NOT MY IMPLEMENTATION
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void Decode(CHIP8 *c8, uint16_t instruction, char **assembly, size_t size, int enable){

    //if (enable)
        //msleep(200);
    uint16_t sliced[4];
    slice_instruction(instruction, sliced);

    uint16_t address = mergeNibbles(sliced, 3); //0x0NNN
    uint16_t value = mergeNibbles(sliced, 2); //0x00NN
    uint8_t X = sliced[1]; //0xnXnn
    uint8_t Y = sliced[2]; //0xnnYn
    uint8_t N = sliced[3]; //last nibble

    uint16_t data[] = {address, value, X, Y, N, '\0'};

   // printf("%X \t ", instruction);

    char **disassembly = assembly;

    if(enable == ENABLE)
        disassembly = NULL; //Disables writing into the assembly buffer

    switch(sliced[0]){
        case(0x0): //0XXX
            switch(sliced[3]){
                case(0x0):
                if(sliced[2] == 0xE){
                    add_assembly(disassembly, size, "CLS");
                }
                else
                    add_assembly(disassembly, size, "SYS 0x%X%X%X", sliced[1], sliced[2], sliced[3]);
                if (enable)
                    Clear(c8);
                break;
                case(0xE):
             //   if (enable)
                 //   Return(c8);
                add_assembly(disassembly, size, "RET");
                break;
                default: //0NNN (Machine language routine. NOT implemented)
                    add_assembly(disassembly, size, "SYS 0x%X%X%X", sliced[1], sliced[2], sliced[3]);
            }
        break;
        case(0x1): //1nnn
        add_assembly(disassembly, size, "JP 0x%X", address);
        if (enable){
            Jump(c8, data, 0x0, JP_1);
        }
        break;
        case(0x2): //2nnn
       // if (enable)
       //     Jump(c8, data, 0, SUBROUTINE);
        add_assembly(disassembly, size, "CALL 0x%X%X%X", sliced[1], sliced[2], sliced[3]);
        break;
        case(0x3): //3xkk
    //    if (enable)
        //    Jump(c8, data, 0, JP_3);
        add_assembly(disassembly, size, "SE V%X, 0x%X%X", sliced[1], sliced[2], sliced[3]);
        break;
        case(0x4): //4xkk
      //  if (enable)
       //     Jump(c8, data, 0, JP_4);
        add_assembly(disassembly, size, "SNE V%X, 0x%X%X", sliced[1], sliced[2], sliced[3]);
        break;
        case(0x5): //5xy0
    //    if (enable)
        //    Jump(c8, data, 0, JP_5);
        add_assembly(disassembly, size, "SE V%X, V%X", sliced[1], sliced[2]);
        break;
        case(0x6): //6XXX
        if (enable)
            Set(c8, data, V_MODE);
        add_assembly(disassembly, size, "LD V%X, 0x%X%X", sliced[1], sliced[2], sliced[3], sliced[1]);
        break;
        case(0x7): //7XXX
        add_assembly(disassembly, size, "ADD V%X, 0x%X", X, value);
        if (enable)
            Add(c8, data, NO_CARRY);
        break;
        case(0x8):
            switch(sliced[3]){
                case(0x0):
          //      if(enable)
         //           Set(c8, data, V_V_MODE);
                add_assembly(disassembly, size, "LD V%X, V%X", sliced[1], sliced[2]);
                break;
                case(0x1):
             //   c8->V[X] = OR(c8->V[X], c8->V[Y]);
                add_assembly(disassembly, size, "OR V%X, V%X", sliced[1], sliced[2]);
                break;
                case(0x2):
            //    c8->V[X] = AND(c8->V[X], c8->V[Y]);
                add_assembly(disassembly, size, "AND V%X, V%X", sliced[1], sliced[2]);
                break;
                case(0x3):
              //  c8->V[X] = XOR(c8->V[X], c8->V[Y]);
                add_assembly(disassembly, size, "XOR V%X, V%X", sliced[1], sliced[2]);
                break;
                case(0x4):
             //   if(enable)
                 //   Add(c8, data, CARRY_MODE);
                add_assembly(disassembly, size, "ADD V%X, V%X", sliced[1], sliced[2]);
                break;
                case(0x5):
               // if(enable)
              //      Sub(c8, data, SUB_1);
                add_assembly(disassembly, size, "SUB V%X, V%X", sliced[1], sliced[2]);
                break;
                case(0x6):
             //   if(enable)
            //        Shift(c8, data, RIGHT);
                add_assembly(disassembly, size, "SHR V%X, {, V%X}", sliced[1], sliced[2]);
                break;
                case(0x7):
             //   if(enable)
            //        Sub(c8, data, SUB_2);
                add_assembly(disassembly, size, "SUBN V%X, V%X", sliced[1], sliced[2]);
                break;
                case(0xE):
             //   if(enable)
              //      Shift(c8, data, LEFT);
                add_assembly(disassembly, size, "SHL V%X, {, V%X}", sliced[1], sliced[2]);
                break;
                default:
                add_assembly(disassembly, size, "N/A");
            }
        break;
        case(0x9):
        //if (enable)
       //     Jump(c8, data, 0, JP_9);
        add_assembly(disassembly, size, "SNE V%X, V%X", sliced[1], sliced[2]);
        break;
        case(0xA):
        if (enable)
            Set(c8, data, I_MODE);
        add_assembly(disassembly, size, "LD I, 0x%X%X%X", sliced[1], sliced[2], sliced[3]);
        break;
        case(0xB):
      //  if (enable)
        //    Jump(c8, data, c8->V[X], JP_1);
        add_assembly(disassembly, size, "JP V0, 0x%X%X%X", sliced[1], sliced[2], sliced[3]);
        break;
        case(0xC):
        //if (enable)
          //  Random(c8, data);
        add_assembly(disassembly, size, "RND V%X, 0x%X%X", sliced[1], sliced[2], sliced[3]);
        break;
        case(0xD):
        add_assembly(disassembly, size, "DRW V%X, V%X, 0x%X", sliced[1], sliced[2], sliced[3]);
        if (enable)
            Draw(c8, X, Y, N);
        break;
        case(0xE):
            switch(sliced[2]){
                case(0x9):
                add_assembly(disassembly, size, "SKP V%X", sliced[1]);
                break;
                case(0xA):
                add_assembly(disassembly, size, "SKNP V%X", sliced[1]);
                break;
                default:
                add_assembly(disassembly, size, "N/A");
            }
        break;
        case(0xF):
            switch(sliced[3]){
                case(0x7):
                add_assembly(disassembly, size, "LD V%X, DT", sliced[1]);
                break;
                case(0xA):
                add_assembly(disassembly, size, "LD V%X, K", sliced[1]);
                break;
                case(0x8):
                add_assembly(disassembly, size, "LD ST, V%X", sliced[1]);
                break;
                case(0xE):
                add_assembly(disassembly, size, "ADD I, V%X", sliced[1]);
                break;
                case(0x9):
                add_assembly(disassembly, size, "LD F, V%X", sliced[1]);
                break;
                case(0x3):
                add_assembly(disassembly, size, "LD B, V%X", sliced[1]);
                break;
                case(0x5):
                    switch(sliced[2]){
                        case(0x1):
                        add_assembly(disassembly, size, "LD DT, V%X", sliced[1]);
                        break;
                        case(0x5):
                        add_assembly(disassembly, size, "LD [I], V%X", sliced[1]);
                        break;
                        case(0x6):
                        add_assembly(disassembly, size, "LD V%X, [I]", sliced[1]);
                        break;
                    }
                break;
                default:
                add_assembly(disassembly, size, "N/A");
            }
        break;
        default:
        add_assembly(disassembly, size, "N/A");
    }
}

void slice_instruction(uint16_t instruction, uint16_t dest[4]){

    dest[0] = instruction >> 12;
    dest[1] = (instruction >> 8) - (dest[0] << 4);
    dest[2] = (instruction >> 4) - ((dest[0] << 8) + (dest[1] << 4));

    uint16_t temp = instruction << 12; //temporary variable for the last slice
    dest[3] = temp >> 12;
}

void add_assembly(char **assembly, size_t program_size, char assembly_instruction[], ...){

    size_t string_size;
    for (string_size = 0; assembly_instruction[string_size] != '\0'; string_size++){
    }
    string_size++; //for the \'0' character

    int i;
    if (assembly != NULL){
    for(i = 0; i < program_size; i++){
            if(assembly[i] == NULL){
                assembly[i] = (char *)malloc(sizeof(char)*string_size); //Allocating enough memory for the assembly instruction

                va_list args;
                va_start(args, assembly_instruction);
                vsnprintf(assembly[i], string_size, assembly_instruction, args);
                va_end(args);
                //printf("%s", assembly[i]);
                break;
            }
        }
    }
    else{
        char temp_buffer[string_size];

        va_list args;
        va_start(args, assembly_instruction);
        vsnprintf(temp_buffer, string_size, assembly_instruction, args);
        va_end(args);
     //   printf("%s", temp_buffer);
    }
}
