#include <array>
#include <cstdint>
#ifndef GAMEBOYEMULATOR_MEMORY_H
#define GAMEBOYEMULATOR_MEMORY_H

#include "./Timer.h"
// Forward declrations
class Cartridge;
class IO;
class PPUnit;

class MEMORY {
public:
  std::array<uint8_t, 0x2000> wram;
  std::array<uint8_t, 0x007F> hram;
  uint8_t ie;
  MEMORY();

  Cartridge *cart;
  PPUnit *ppu;
  // Timer* timer;
  IO *io;
  uint8_t &access_memory(uint16_t addr);

  // DATA BUS
  uint8_t read8(uint16_t addr);
  void write8(uint16_t addr, uint8_t val);
  uint16_t read16(uint16_t addr);
  void write16(uint16_t addr, uint16_t val);
};

#endif // GAMEBOYEMULATOR_MEMORY_H
