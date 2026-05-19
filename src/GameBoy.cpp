#include "GameBoy.h"
#include <bitset>
#include <iostream>

GameBoy::GameBoy() : MMU(), CPU(MMU), PPU(MMU), CART(), io(), timer() {
  MMU.ppu = &PPU;
  MMU.cart = &CART;
  // MMU.timer = &timer;
  io.timer = &timer;
  MMU.io = &io;
}

void GameBoy::printCPUState() {
  std::cout << "A: " << int(CPU.A) << " | B: " << int(CPU.B)
            << " | C: " << int(CPU.C) << std::endl;
  std::cout << "D: " << int(CPU.D) << " | E: " << int(CPU.E)
            << " | HL: " << int(CPU.getHL()) << std::endl;
  std::cout << "PC: " << std::hex << CPU.PC << " | SP: " << std::dec << CPU.SP
            << std::endl;

  std::bitset<8> flags(CPU.F);
  std::cout << "Flags: " << flags << std::endl;

  // Optional: label the bits
  std::cout << "Z=" << flags[7] << " N=" << flags[6] << " H=" << flags[5]
            << " C=" << flags[4] << std::endl;
  std::cout << std::endl << "----------------------" << std::endl;
}

void GameBoy::loadROM(const std::string &path) { CART.loadROM(path); }

void GameBoy::run() {
  std::cout << "RUNNING" << std::endl;
  try {
    while (true) {
      // std::cout << "CPU RAN" << std::endl;
      tick();
      // std::cin.get();
    }
  } catch (const std::exception &e) {
    std::cout << "CRASH: " << e.what() << std::endl;
    std::cout << "PC: " << std::hex << CPU.PC << std::endl;
  }
}

void GameBoy::tick(void) {
  for (int i = 0; i < 4; i++) { // T cycle components
    timer.tick();
  }
  CPU.cpu_tick();
}
