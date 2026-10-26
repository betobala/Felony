#ifndef OC_UTIL_ADC_H_
#define OC_UTIL_ADC_H_
// The Teensy ADC library, reduced to what OC::ADC touches (the virtual ADC is in OC_ADC.cpp).
#include <Arduino.h>

#define ADC_REF_3V3 0
#define ADC_HIGH_SPEED_16BITS 0
#define ADC_HIGH_SPEED 0

class ADC {
public:
  void setReference(int) {}
  void setResolution(int) {}
  void setConversionSpeed(int) {}
  void setSamplingSpeed(int) {}
  void setAveraging(int) {}
  void enableDMA() {}
};

#endif
