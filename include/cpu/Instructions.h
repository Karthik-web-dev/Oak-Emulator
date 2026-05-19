//
// Created by karth on 31-01-2026.
//
#include<bits/stdc++.h>
#include <cstdint>
#ifndef GAMEBOYEMULATOR_INSTRUCTIONS_H
#define GAMEBOYEMULATOR_INSTRUCTIONS_H

struct ExecState;
class SM83;

struct Instruction {
    uint8_t opcode; //operation code in hex
    const char* mnemonic; //for developer reference
    uint8_t bytes; //instruction length in bytes.
    uint8_t cycles; //base cycles for this instruction
    uint8_t alt_cycles; //extra cycles if required.
    void (SM83::*execute)(ExecState&); //pointer to function
    bool wrapper_overrides_pc;
};


extern Instruction prefixed_instruction_table[256];
extern Instruction unprefixed_instruction_table[256];


#endif //GAMEBOYEMULATOR_INSTRUCTIONS_H
