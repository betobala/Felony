// Runtime behind shim/Arduino.h: time, random numbers and the virtual hardware instance.
#include "Arduino.h"
#include "EEPROM.h"
#include <chrono>
#include <thread>

namespace ochw {

Hardware hw;

Hardware::Hardware() {
	for (auto& p : pins)
		p = 1;  // pulled up: buttons released, no gates
	for (auto& v : cv)
		v = 0.f;
	for (auto& d : dac)
		d = 0;
	memset(eeprom, 0xff, sizeof(eeprom));  // erased flash reads as 0xff
}

int Hardware::addTimer(void (*fn)(), uint32_t us) {
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (!timers[i].fn.load()) {
			timers[i].periodUs = us;
			timers[i].nextUs = micros.load() + us;
			timers[i].fn = fn;
			return i;
		}
	}
	return -1;
}

void Hardware::removeTimer(int slot) {
	if (slot >= 0 && slot < MAX_TIMERS)
		timers[slot].fn = nullptr;
}

}  // namespace ochw

volatile uint32_t ochw_dummy_reg = 0;
volatile uint32_t ochw_cyccnt = 0;
SerialStub Serial;
EEPROMClass EEPROM;

uint32_t micros() {
	ochw::checkStop();
	return (uint32_t) ochw::hw.micros.load(std::memory_order_relaxed);
}

uint32_t millis() {
	ochw::checkStop();
	return (uint32_t) (ochw::hw.micros.load(std::memory_order_relaxed) / 1000);
}

void delayMicroseconds(uint32_t us) {
	// waits on module time, so a paused engine pauses the firmware too
	uint64_t end = ochw::hw.micros.load() + us;
	while (ochw::hw.micros.load() < end) {
		ochw::checkStop();
		std::this_thread::sleep_for(std::chrono::microseconds(200));
	}
}

void delay(uint32_t ms) {
	delayMicroseconds(ms * 1000);
}

static uint32_t rng_state = 0x12345678u;

void randomSeed(unsigned long seed) {
	if (seed)
		rng_state = (uint32_t) seed;
}

static uint32_t rng() {
	// xorshift32
	uint32_t x = rng_state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return rng_state = x;
}

long random(long howbig) {
	return howbig <= 0 ? 0 : (long) (rng() % (uint32_t) howbig);
}

long random(long howsmall, long howbig) {
	return howsmall >= howbig ? howsmall : howsmall + random(howbig - howsmall);
}
