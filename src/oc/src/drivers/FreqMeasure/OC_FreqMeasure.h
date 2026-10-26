#ifndef OC_FREQMEASURE_H
#define OC_FREQMEASURE_H
// Frequency meter on TR4 for the virtual O&C: the Rack module measures the period of the gate at
// TR4 and posts it here in F_BUS cycles, like the Teensy FTM capture would.
#include <Arduino.h>

class FreqMeasureClass {
public:
  static void begin(void) { ochw::hw.freqEnabled = true; ochw::hw.freqCount = 0; }
  static uint8_t available(void) { return ochw::hw.freqCount.load() ? 1 : 0; }
  static uint32_t read(void) { return ochw::hw.freqCount.exchange(0); }
  static float countToFrequency(uint32_t count) { return count ? (float) F_BUS / (float) count : 0.f; }
  static void end(void) { ochw::hw.freqEnabled = false; }
};

static FreqMeasureClass FreqMeasure;

#endif
