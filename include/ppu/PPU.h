#ifndef GAMEBOYEMULATOR_PPU_H
#define GAMEBOYEMULATOR_PPU_H
#include<cstdint>
#include<array>

//Forward Declaration
class MEMORY;

class PPUnit {
    public:
        std::array<uint8_t, 0x2000> vram;
        std::array<uint8_t, 0x00A0> oam;
        MEMORY& mmu;
        PPUnit(MEMORY& memory);
};

#endif
