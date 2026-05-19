#include "memory/Cartridge.h"
#include <fstream>
#include <string>

void Cartridge::loadROM(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    file.read(reinterpret_cast<char*>(rom.data()), 0x8000);
}
