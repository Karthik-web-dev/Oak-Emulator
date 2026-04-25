#include <bits/stdc++.h>
#include <cstdint>
#ifndef GAMEBOYEMULATOR_MEMORY_H
#define GAMEBOYEMULATOR_MEMORY_H
class MEMORY {
public:
    uint8_t ram[0x10000];
    MEMORY();

    //DATA BUS
    uint8_t read8(uint16_t addr);
    void write8(uint16_t addr, uint8_t val);
    uint16_t read16(uint16_t addr);
    void write16(uint16_t addr, uint16_t val);
};

#endif //GAMEBOYEMULATOR_MEMORY_H