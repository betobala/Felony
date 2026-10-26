#pragma once
// The virtual O&C hardware: what the firmware sees through the Arduino/Teensy shims, and what the
// Rack module drives. One instance per process, because the firmware keeps its state in globals.
#include <stdint.h>
#include <atomic>

namespace ochw {

static const int NUM_PINS = 34;
static const int EEPROM_SIZE = 2048;

struct Hardware {
	// pin levels as the Teensy would read them (buttons and trigger inputs are active low)
	std::atomic<uint8_t> pins[NUM_PINS];
	void (*isr[NUM_PINS])() = {};

	// ADC: volts at the four CV inputs
	std::atomic<float> cv[4];
	// DAC: last 16 bit codes written to the four outputs
	std::atomic<uint32_t> dac[4];
	// display: the SH1106 RAM, 8 pages of 128 columns (one bit per pixel, LSB at the top)
	uint8_t screen[8][128] = {};
	std::atomic<uint32_t> screenVersion{0};

	// frequency meter on TR4 (APP_REFS tuner): period in F_BUS cycles, 0 = none pending
	std::atomic<bool> freqEnabled{false};
	std::atomic<uint32_t> freqCount{0};

	uint8_t eeprom[EEPROM_SIZE];
	std::atomic<bool> eepromDirty{false};

	// time, advanced by the core ISR (16.666 kHz)
	std::atomic<uint64_t> micros{0};

	// IntervalTimers started by the firmware (core ISR at 60 us, UI ISR at 1 ms)
	struct Timer {
		std::atomic<void (*)()> fn{nullptr};
		uint32_t periodUs = 0;
		uint64_t nextUs = 0;
	};
	static const int MAX_TIMERS = 4;
	Timer timers[MAX_TIMERS];
	int addTimer(void (*fn)(), uint32_t us);
	void removeTimer(int slot);

	// set to stop the firmware thread: the next millis()/delay() call unwinds it
	std::atomic<bool> stop{false};

	Hardware();
};

extern Hardware hw;

/** Thrown inside the firmware thread to unwind it when the module goes away. */
struct Stop {};

inline void checkStop() {
	if (hw.stop.load(std::memory_order_relaxed))
		throw Stop();
}

}  // namespace ochw
