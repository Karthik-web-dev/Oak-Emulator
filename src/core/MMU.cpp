#include "core/MMU.h"
#include "memory/Cartridge.h"
#include "memory/IORegisters.h"
#include "ppu/PPU.h"
#include <bits/stdc++.h>
#include <iostream> //temp

MEMORY::MEMORY() {
  hram.fill(0);
  wram.fill(0);
  ppu = nullptr;
  cart = nullptr;
  io = nullptr;
  ie = 0;
};

uint8_t MEMORY::read8(uint16_t addr) {
  if (addr <= 0x7FFF)
    return cart->rom[addr];
  else if (addr <= 0x9FFF)
    return ppu->vram[addr - 0x8000];
  else if (addr <= 0xBFFF)
    return cart->extram[addr - 0xA000];
  else if (addr <= 0xDFFF)
    return wram[addr - 0xC000];
  else if (addr <= 0xFDFF)
    return wram[addr - 0xE000];
  else if (addr <= 0xFE9F)
    return ppu->oam[addr - 0xFE00];
  else if (addr == 0xFF04)
    return io->read(addr);
  else if (addr <= 0xFF7F)
    return io->registers[addr - 0xFF00];
  else if (addr <= 0xFFFE)
    return hram[addr - 0xFF80];
  else
    return ie;
}

// if (addr <= 0x7FFF) return 0xFF;
void MEMORY::write8(uint16_t addr, uint8_t val) {
  // if (addr <= 0x7FFF) return;
  // if (addr >= 0xFEA0 && addr <= 0xFEFF) return;

  if (addr <= 0x7FFF)
    cart->rom[addr];
  else if (addr <= 0x9FFF)
    ppu->vram[addr - 0x8000];
  else if (addr <= 0xBFFF)
    cart->extram[addr - 0xA000];
  else if (addr <= 0xDFFF)
    wram[addr - 0xC000];
  else if (addr <= 0xFDFF)
    wram[addr - 0xE000];
  else if (addr <= 0xFE9F)
    ppu->oam[addr - 0xFE00];
  else if (addr <= 0xFF7F)
    io->registers[addr - 0xFF00];
  else if (addr <= 0xFFFE)
    hram[addr - 0xFF80];
  else
    ie;

  // For blargg tests
  if (addr == 0xFF02 && val == 0x81) {
    std::cout << "BLARG WORKS!" << std::endl;
    std::cout << "SB: " << std::hex << (int)read8(0xFF01)
              << " SC: " << (int)read8(0xFF02) << std::endl;

    std::cout << io->registers[0x01];
    io->registers[0x02] &= ~0x80;
    return;
  }
  // For blargg tests
}

uint16_t MEMORY::read16(uint16_t addr) {
  return (static_cast<uint16_t>((read8(addr + 1) << 8) | read8(addr)));
}

void MEMORY::write16(uint16_t addr, uint16_t val) {
  write8(addr, (val & 0x0FF));
  write8(addr + 1, (val >> 8) & 0xFF);
}
