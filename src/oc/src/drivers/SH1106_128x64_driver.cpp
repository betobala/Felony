// SH1106 driver for the virtual O&C: pages land in ochw::hw.screen instead of going out on SPI.
#include <Arduino.h>
#include "SH1106_128x64_driver.h"

/*static*/
void SH1106_128x64_Driver::Init() {
  Clear();
}

/*static*/
void SH1106_128x64_Driver::Clear() {
  memset(ochw::hw.screen, 0, sizeof(ochw::hw.screen));
  ochw::hw.screenVersion++;
}

/*static*/
void SH1106_128x64_Driver::Flush() {
}

/*static*/
void SH1106_128x64_Driver::SendPage(uint_fast8_t index, const uint8_t *data) {
  if (index >= kNumPages)
    return;
  memcpy(ochw::hw.screen[index], data, kPageSize);
  if (index == kNumPages - 1)
    ochw::hw.screenVersion++;
}

/*static*/
void SH1106_128x64_Driver::SPI_send(void *, size_t) {
}

/*static*/
void SH1106_128x64_Driver::AdjustOffset(uint8_t) {
  // the virtual screen has no column offset to trim
}
