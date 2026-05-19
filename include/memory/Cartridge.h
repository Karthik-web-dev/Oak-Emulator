#ifndef GAMEBOYEMULATOR_CARTRIDGE_H
#define GAMEBOYEMULATOR_CARTRIDGE_H
#include<cstdint>
#include<string>
#include<array>

class Cartridge {
    public:
        std::array<uint8_t, 0x8000> rom;
        std::array<uint8_t, 0x2000> extram;
        Cartridge() {
            rom.fill(0);
            extram.fill(0);
        }
        void loadROM(const std::string& path);
};

#endif
