#pragma once
// Teensy EEPROM backed by ochw::hw.eeprom (saved in the Rack patch).
#include "Arduino.h"

struct EERef {
	int index;
	operator uint8_t() const { return ochw::hw.eeprom[index]; }
	EERef& operator=(uint8_t v) {
		if (ochw::hw.eeprom[index] != v) {
			ochw::hw.eeprom[index] = v;
			ochw::hw.eepromDirty = true;
		}
		return *this;
	}
	EERef& update(uint8_t v) { return *this = v; }
};

struct EEPtr {
	int index;
	EEPtr(int i) : index(i) {}
	operator int() const { return index; }
	EEPtr& operator=(int i) { index = i; return *this; }
	EERef operator*() { return EERef{index}; }
	EEPtr& operator++() { ++index; return *this; }
	EEPtr operator++(int) { EEPtr p = *this; ++index; return p; }
	bool operator!=(const EEPtr& o) const { return index != o.index; }
};

struct EEPROMClass {
	uint8_t read(int i) { return ochw::hw.eeprom[i]; }
	void write(int i, uint8_t v) { EERef{i} = v; }
	void update(int i, uint8_t v) { EERef{i} = v; }
	EERef operator[](int i) { return EERef{i}; }
	uint16_t length() { return ochw::EEPROM_SIZE; }
	template <typename T> T& get(int i, T& t) {
		memcpy(&t, ochw::hw.eeprom + i, sizeof(T));
		return t;
	}
	template <typename T> const T& put(int i, const T& t) {
		const uint8_t* p = (const uint8_t*) &t;
		for (size_t k = 0; k < sizeof(T); k++)
			EERef{(int) (i + k)} = p[k];
		return t;
	}
};
extern EEPROMClass EEPROM;
