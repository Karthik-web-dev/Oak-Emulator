//
// Created by karth on 31-01-2026.
//
#include<bits/stdc++.h>
#include "Memory.h"

MEMORY::MEMORY() : ram(0) {};

uint8_t MEMORY::read8(uint16_t addr) {
    return ram[addr];
}
void MEMORY::write8(uint16_t addr, uint8_t val) {
    ram[addr] = val;
}

uint16_t MEMORY::read16(uint16_t addr) {
    return ((ram[addr+1] << 8) | ram[addr]);
}

void MEMORY::write16(uint16_t addr, uint16_t val) {
    ram[addr + 1] = (val >> 8) & 0xFF;
    ram[addr] = (val) & 0x0FF;
}