#include "memory/IORegisters.h"

IO::IO() { registers.fill(0); }

uint8_t Joypad::read() {
  uint8_t d_pad = (right ? 0 : (1 << 0)) | (left ? 0 : (1 << 1)) |
                  (up ? 0 : (1 << 2)) | (down ? 0 : (1 << 3));

  uint8_t btn = (a ? 0 : (1 << 0)) | (b ? 0 : (1 << 1)) |
                (select ? 0 : (1 << 2)) | (start ? 0 : (1 << 3));

  switch (select) {
  case 0x00: // Both selected
    return (btn & d_pad);
  case 0x10: // Buttons selected
    return btn;
  case 0x20: // D-pad selected
    return d_pad;
  case 0x30: // Both not selected
    return 0x0F;
  default:
    return 0x0F;
  }
}

void Joypad::write(uint8_t val) { select = val & 0x30; }

uint8_t IO::read(uint16_t addr) {
  switch (addr) {
  case 0xFF00:
    return joypad.read();
  case 0xFF01: // stubbed
    return 0xFF;
  case 0xFF02: // stubbed
    return 0x00;
  case 0xFF04:
    return timer->readDIV();
  case 0xFF05:
    return timer->readTIMA();
  case 0xFF06:
    return timer->readTMA();
  case 0xFF07:
    return timer->readTAC();
  case 0xFF0F:
    return IF | 0xE0; // upper 3 bits always read as 3. They are not
                      // implemetned in real hardware and left
                      // disconnected. Gameboy motherboard has pull up
                      // resistors to fill in the 8 bit data bus, hence
                      // the top 3 bits become 1.

  default:
    return 0xFF;
  }
}

void IO::write(uint16_t addr, uint8_t val) {
  switch (addr) {
  case 0xFF00:
    joypad.write(val);
    break;
  case 0xFF01: // ignore writes
  case 0xFF02: // ignore writes
    break;
  case 0xFF04:
    timer->writeDIV();
    break;
  case 0xFF05:
    timer->writeTIMA(val);
    break;
  case 0xFF06:
    timer->writeTMA(val);
    break;
  case 0xFF07:
    timer->writeTAC(val);
    break;
  case 0xFF0F:
    // WARNING: Since the CPU automatically sets and clears the bits in
    // the IF register, it is usually not necessary to write to the IF
    // register.
    IF = val & 0x1F; // First three bits are ignored
    break;
  }
}
