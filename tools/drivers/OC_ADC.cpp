// OC::ADC for the virtual O&C: the four CV inputs are sampled from ochw::hw.cv with the same
// scaling as the hardware (inverting stage, 0 V at 2/3 of the 12 bit range, 409.6 counts per volt)
// and then smoothed exactly like the firmware does.
#include "OC_ADC.h"
#include "OC_gpio.h"
#include <algorithm>

namespace OC {

/*static*/ ::ADC ADC::adc_;
/*static*/ ADC::CalibrationData *ADC::calibration_data_;
/*static*/ uint32_t ADC::raw_[ADC_CHANNEL_LAST];
/*static*/ uint32_t ADC::smoothed_[ADC_CHANNEL_LAST];
/*static*/ volatile bool ADC::ready_;
constexpr uint16_t ADC::SCA_CHANNEL_ID[DMA_NUM_CH];

static const float kZeroCode = 4096.f * 0.6666667f;
static const float kCountsPerVolt = 4096.f / 10.f;

/** 16 bit scan value (as the DMA buffer would hold it) for a voltage */
static uint32_t to_scan(float volts) {
  float code = (kZeroCode - volts * kCountsPerVolt) * 16.f;
  if (code < 0.f) code = 0.f;
  if (code > 65535.f) code = 65535.f;
  return (uint32_t) code;
}

/*static*/ void ADC::Init(CalibrationData *calibration_data) {
  calibration_data_ = calibration_data;
  std::fill(raw_, raw_ + ADC_CHANNEL_LAST, 0);
  std::fill(smoothed_, smoothed_ + ADC_CHANNEL_LAST, 0);
  ready_ = true;
}

/*static*/ void ADC::DMA_ISR() {
  ready_ = true;
}

/*static*/ void ADC::Init_DMA() {
}

/*static*/ void FASTRUN ADC::Scan_DMA() {
  // the hardware completes a scan at ~5.5 kHz; every third core tick is close enough
  static uint32_t div = 0;
  if (++div < 3)
    return;
  div = 0;
  update<ADC_CHANNEL_1>(to_scan(ochw::hw.cv[0]));
  update<ADC_CHANNEL_2>(to_scan(ochw::hw.cv[1]));
  update<ADC_CHANNEL_3>(to_scan(ochw::hw.cv[2]));
  update<ADC_CHANNEL_4>(to_scan(ochw::hw.cv[3]));
}

/*static*/ void ADC::CalibratePitch(int32_t c2, int32_t c4) {
  // This is the method used by the Mutable Instruments calibration and
  // extrapolates from two octaves. I guess an alternative would be to get the
  // lowest (-3v) and highest (+6v) and interpolate between them
  // *vague handwaving*
  if (c2 < c4) {
    int32_t scale = (24 * 128 * 4096L) / (c4 - c2);
    calibration_data_->pitch_cv_scale = scale;
  }
}

};  // namespace OC
