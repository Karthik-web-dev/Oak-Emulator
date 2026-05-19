#include "core/Timer.h"

Timer::Timer()
    : t_cycles(0), TIMA(0), TAC(0), TMA(0), overflow_pending(false) {}

static int clockBit(uint8_t tac) {
  switch (tac & 0x03) {
  case 0:
    return 3;
  case 1:
    return 5;
  case 2:
    return 7;
  case 3:
    return 9;
  default:
    return 9;
  }
}

/*TODO:Because your tick() function runs every T-cycle, checking if
  (overflow_pending) at the very start of tick() means you are only waiting 1
  T-cycle, not 1 M-cycle (4 T-cycles). To fix this so it matches real hardware,
  we should turn overflow_pending into a 2-bit cycle counter (or a countdown).
  The Overflow Timeline (in T-cycles):
    Cycle 0: TIMA increments from 0xFF to 0x00. overflow_cnt is set to 4.
    Cycles 1-3: TIMA reads as 0x00. A write to TIMA during this time cancels the
  overflow. Cycle 4 (1 M-cycle later): TIMA is loaded with TMA, and the IF flag
  (Interrupt Request) is set to 1. A write to TIMA on this exact cycle is
  ignored.
  */
void Timer::tick(void) {
  if (overflow_pending) {
    overflow_pending = false;
    TIMA = TMA;
    // request_interrupt();
  }

  uint16_t prev_tcycles = t_cycles++;

  if (TAC & 0x04) {
    int bit = clockBit(TAC);
    if ((prev_tcycles >> bit & 1) && !(t_cycles >> bit & 1)) {
      if (++TIMA == 0) {
        overflow_pending = true;
        TIMA = 0x00;
      }
    }
  }
}

uint8_t Timer::readDIV(void) { return t_cycles >> 8; }

void Timer::writeDIV(void) {
  bool oldSignal = (TAC & 0x04) && ((t_cycles >> clockBit(TAC)) & 1);
  if (oldSignal) {
    if (++TIMA == 0) {
      overflow_pending = true;
      TIMA = 0x00;
    }
  }
  t_cycles = 0;
}

uint8_t Timer::readTIMA() { return TIMA; }

void Timer::writeTIMA(uint8_t val) {
  if (overflow_pending) {
    return; // cycle B: ignored, TMA will reload
  }
  overflow_pending = false; // cycle A: cancel overflow
  TIMA = val;
}

uint8_t Timer::readTMA() { return TMA; }

void Timer::writeTMA(uint8_t val) {
  TMA = val;
  if (overflow_pending) {
    TIMA = val; // cycle B: mirrors into TIMA
  }
}

uint8_t Timer::readTAC() { return TAC; }

void Timer::writeTAC(uint8_t val) {
  bool oldSignal = (TAC & 0x04) && ((t_cycles >> clockBit(TAC)) & 1);
  bool newSignal = (val & 0x04) && ((t_cycles >> clockBit(val)) & 1);
  if (oldSignal && !newSignal) {
    if (++TIMA == 0) {
      overflow_pending = true;
      TIMA = 0x00;
    }
  }
  TAC = val;
}
