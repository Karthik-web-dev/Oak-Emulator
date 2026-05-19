#ifndef GAMEBOYEMULATOR_TIMER_H
#define GAMEBOYEMULATOR_TIMER_H
#include <cstdint>
class Timer {
public:
  uint16_t t_cycles; // or div_counter

  uint16_t TIMA;
  uint8_t TAC;
  uint8_t TMA;

  Timer();

  void tick(void);

  uint8_t readDIV(void);
  void writeDIV(void);

  bool overflow_pending = false;

  uint8_t readTIMA();
  void writeTIMA(uint8_t val);

  uint8_t readTMA();
  void writeTMA(uint8_t val);

  uint8_t readTAC();
  void writeTAC(uint8_t val);
};

#endif
