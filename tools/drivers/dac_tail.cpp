// The four DAC8565 channels of the virtual O&C (see tools/port.py): codes go to ochw::hw.dac.
void set8565_CHA(uint32_t data) {
  ochw::hw.dac[0] = data;
}

void set8565_CHB(uint32_t data) {
  ochw::hw.dac[1] = data;
}

void set8565_CHC(uint32_t data) {
  ochw::hw.dac[2] = data;
}

void set8565_CHD(uint32_t data) {
  ochw::hw.dac[3] = data;
}

void SPI_init() {
}
