#ifndef GAMEBOYEMULATOR_GAMEBOY_H
#define GAMEBOYEMULATOR_GAMEBOY_H

#include "cpu/CPU.h"
#include "core/MMU.h"
#include "ppu/PPU.h"
#include "memory/IORegisters.h"
#include "memory/Cartridge.h"
#include "core/Timer.h"

class GameBoy {
    public:
        MEMORY MMU;
        SM83 CPU;
        PPUnit PPU;
        Cartridge CART;
        IO io;
        Timer timer;
        GameBoy();

        void tick(void);
        void printCPUState();
        void loadROM(const std::string& path);
        void run();
};


#endif
