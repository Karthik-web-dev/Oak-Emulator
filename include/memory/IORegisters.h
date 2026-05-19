#ifndef GAMEBOYEMULATOR_IO_H
#define GAMEBOYEMULATOR_IO_H
#include <array>
#include <cstdint>

#include "core/Timer.h"

class Joypad {
public:
  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;

  bool a = false;
  bool b = false;
  bool selectBtn = false;
  bool start = false;

  uint8_t select = 0x30;

  uint8_t read();
  void write(uint8_t val);
};

class IO {
public:
  std::array<uint8_t, 0x80> registers;
  IO();
  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t val);
  Timer *timer;
  Joypad joypad;

  uint8_t IF;
};

#endif
