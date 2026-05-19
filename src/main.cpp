#include "GameBoy.h"
#include <iostream>
#include <bits/stdc++.h>

int main() {
    GameBoy gb;

    std::vector<uint8_t> program = {

        // --------- Initialize registers ----------
        0x06, 0x10,  // LD B, 0x10
        0x0E, 0x20,  // LD C, 0x20
        0x16, 0x30,  // LD D, 0x30
        0x1E, 0x40,  // LD E, 0x40
        0x26, 0x50,  // LD H, 0x50
        0x2E, 0x60,  // LD L, 0x60
        0x3E, 0x70,  // LD A, 0x70

        // Set HL to point to memory
        0x26, 0x20,  // LD H, 0x20
        0x2E, 0x00,  // LD L, 0x00

        // Write value at (HL) so INC (HL) is visible
        0x36, 0x99,  // LD (HL), 0x99

        // --------- INC r ----------
        0x04,  // INC B
        0x0C,  // INC C
        0x14,  // INC D
        0x1C,  // INC E
        0x24,  // INC H
        0x2C,  // INC L
        0x3C,  // INC A

        // INC (HL)
        0x34,  // INC (HL)

        // --------- INC rr ----------
        0x03,  // INC BC
        0x13,  // INC DE
        0x23,  // INC HL
        0x33   // INC SP
    };

    gb.CPU.A = 0;
gb.CPU.B = 0;
gb.CPU.C = 0;
gb.CPU.D = 0;
gb.CPU.E = 0;
gb.CPU.H = 0;
gb.CPU.L = 0;
gb.CPU.F = 0;
gb.CPU.SP = 0xFFFE;
gb.CPU.PC = 0x0100;

    // Load program into memory
    for (int i = 0; i < program.size(); i++) {
        gb.MMU.write8(0x0100 + i, program[i]);
    }

    gb.CPU.PC = 0x0100;

    // Optional: initialize SP so INC SP is meaningful
    gb.CPU.SP = 0xFFFE;

    // Run all instructions
    for (int i = 0; i < program.size(); i++) {
        gb.CPU.execute();
        gb.printCPUState();
    }
}
