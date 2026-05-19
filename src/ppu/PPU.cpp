#include "ppu/PPU.h"

PPUnit::PPUnit(MEMORY& memory)
    :mmu(memory)
{
    vram.fill(0);
    oam.fill(0);
}
