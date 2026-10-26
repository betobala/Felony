#include "OcRunner.hpp"
#include <Arduino.h>
#include <thread>
#include <mutex>
#include <cmath>
#include "oc/OC_gpio.h"

void setup();
void loop();

namespace ocrun {

static std::thread thread;
static std::atomic<bool> alive{false};
static double clockUs = 0.0;

static const uint8_t BUTTON_PINS[NUM_BUTTONS] = {but_top, but_bot, butL, butR};
static const uint8_t TR_PINS[4] = {TR1, TR2, TR3, TR4};

// Quadrature generator per encoder: one pin state per UI poll (1 kHz), four per detent
struct Quadrature {
	uint8_t pinA, pinB;
	std::atomic<int> pending{0};
	int phase = 0;  // 0 = rest (A=1,B=1)
	int dir = 0;

	void step() {
		if (phase == 0) {
			int p = pending.load();
			if (p == 0)
				return;
			dir = p > 0 ? 1 : -1;
			pending -= dir;
		}
		// +1: (1,1) -> (1,0) -> (0,0) -> (0,1) -> (1,1); -1 swaps the pins
		static const uint8_t A[4] = {1, 1, 0, 0}, B[4] = {1, 0, 0, 1};
		phase = (phase + 1) % 4;
		uint8_t a = A[phase], b = B[phase];
		if (dir < 0)
			std::swap(a, b);
		ochw::hw.pins[pinA] = a;
		ochw::hw.pins[pinB] = b;
	}
};
static Quadrature encoders[2] = {{encL1, encL2}, {encR1, encR2}};

static void firmwareMain() {
	try {
		setup();
		loop();
	}
	catch (const ochw::Stop&) {
	}
	alive = false;
}

void start() {
	if (alive)
		return;
	ochw::hw.stop = false;
	for (auto& t : ochw::hw.timers)
		t.fn = nullptr;
	alive = true;
	thread = std::thread(firmwareMain);
}

void stop() {
	if (!thread.joinable())
		return;
	ochw::hw.stop = true;
	thread.join();
	for (auto& t : ochw::hw.timers)
		t.fn = nullptr;
	alive = false;
}

bool running() {
	return alive;
}

void advance(double dtUs) {
	clockUs += dtUs;
	// run every timer that falls due up to the new time, in time order (the UI timer at 1 kHz
	// and the core timer at 16.666 kHz never coincide closely enough for the order to matter)
	while (true) {
		int next = -1;
		uint64_t nextUs = (uint64_t) clockUs + 1;
		for (int i = 0; i < ochw::Hardware::MAX_TIMERS; i++) {
			auto& t = ochw::hw.timers[i];
			if (t.fn.load() && t.nextUs < nextUs) {
				next = i;
				nextUs = t.nextUs;
			}
		}
		if (next < 0)
			break;
		auto& t = ochw::hw.timers[next];
		ochw::hw.micros = t.nextUs;
		t.nextUs += t.periodUs;
		if (t.periodUs >= 1000) {  // the UI poll: feed the encoder pins first
			encoders[0].step();
			encoders[1].step();
		}
		if (void (*fn)() = t.fn.load())
			fn();
	}
	ochw::hw.micros = (uint64_t) clockUs;
}

void turn(Control enc, int detents) {
	Quadrature& q = encoders[enc];
	int p = q.pending.load() + detents;
	q.pending = std::max(-24, std::min(24, p));
}

void setButton(ButtonId b, bool pressed) {
	ochw::hw.pins[BUTTON_PINS[b]] = pressed ? 0 : 1;
}

void setGate(int input, bool high) {
	uint8_t pin = TR_PINS[input];
	uint8_t level = high ? 0 : 1;  // the input stage inverts
	uint8_t old = ochw::hw.pins[pin].exchange(level);
	if (old == 1 && level == 0 && ochw::hw.isr[pin])
		ochw::hw.isr[pin]();  // FALLING edge interrupt
}

float outputVolts(int channel) {
	// default calibration: octave 3 of the table is 0 V, 6553 codes per volt
	static const float ZERO = 19661.f + 4890.f, PER_VOLT = 6553.f;
	return ((float) ochw::hw.dac[channel].load() - ZERO) / PER_VOLT;
}

}  // namespace ocrun
