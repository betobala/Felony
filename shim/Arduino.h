#pragma once
// Arduino / Teensy 3.2 API for the O&C firmware running inside VCV Rack. Only what the firmware
// uses; pins, time, EEPROM and interrupts are backed by ochw::hw.
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "oc_hw.h"

#ifndef F_CPU
#define F_CPU 120000000
#endif
#define F_BUS 60000000
#define KINETISK 1
#define __MK20DX256__ 1
#define CORE_NUM_DIGITAL 34

#define FASTRUN
#define DMAMEM
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t*) (p))
#define pgm_read_word(p) (*(const uint16_t*) (p))
#define pgm_read_dword(p) (*(const uint32_t*) (p))

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#define OUTPUT_OPENDRAIN 4
#define INPUT_DISABLE 5
#define RISING 3
#define FALLING 2
#define CHANGE 4

typedef bool boolean;
typedef uint8_t byte;

// ---- pins
inline uint8_t digitalReadFast(uint8_t pin) {
	return pin < ochw::NUM_PINS ? ochw::hw.pins[pin].load(std::memory_order_relaxed) : 0;
}
inline uint8_t digitalRead(uint8_t pin) {
	return digitalReadFast(pin);
}
inline void digitalWriteFast(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline void pinMode(uint8_t, uint8_t) {}
inline void attachInterrupt(uint8_t pin, void (*fn)(), int) {
	if (pin < ochw::NUM_PINS)
		ochw::hw.isr[pin] = fn;
}
inline void detachInterrupt(uint8_t pin) {
	if (pin < ochw::NUM_PINS)
		ochw::hw.isr[pin] = nullptr;
}
inline int analogRead(uint8_t) {
	return 0;
}

// register level pin config used by OC::pinMode (no effect here)
extern volatile uint32_t ochw_dummy_reg;
#define portConfigRegister(pin) (&ochw_dummy_reg)
#define portModeRegister(pin) (&ochw_dummy_reg)
#define digitalPinToBitMask(pin) (1u)
#define PORT_PCR_DSE 0
#define PORT_PCR_MUX(n) 0
#define PORT_PCR_ODE 0
#define PORT_PCR_PE 0
#define PORT_PCR_PS 0

// ---- time
uint32_t millis();
uint32_t micros();
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);

// ---- interrupts (the core ISR runs on the audio thread; nothing to mask)
inline void __disable_irq() {}
inline void __enable_irq() {}
inline void noInterrupts() {}
inline void interrupts() {}
#define NVIC_SET_PRIORITY(irq, prio) ((void) 0)
#define NVIC_ENABLE_IRQ(irq) ((void) 0)
#define NVIC_DISABLE_IRQ(irq) ((void) 0)
#define IRQ_PORTB 0
extern volatile uint32_t ochw_cyccnt;
#define ARM_DWT_CYCCNT ochw_cyccnt
#define ARM_DEMCR ochw_dummy_reg
#define ARM_DEMCR_TRCENA 0
#define ARM_DWT_CTRL ochw_dummy_reg
#define ARM_DWT_CTRL_CYCCNTENA 0

// Timers are registered with the virtual hardware; the Rack module calls them on schedule.
class IntervalTimer {
	int slot = -1;
public:
	bool begin(void (*fn)(), uint32_t us) {
		slot = ochw::hw.addTimer(fn, us);
		return slot >= 0;
	}
	void priority(int) {}
	void end() {
		ochw::hw.removeTimer(slot);
		slot = -1;
	}
};

class elapsedMillis {
	uint32_t ms;
public:
	elapsedMillis() : ms(millis()) {}
	operator uint32_t() const { return millis() - ms; }
	elapsedMillis& operator=(uint32_t v) { ms = millis() - v; return *this; }
};

class elapsedMicros {
	uint32_t us;
public:
	elapsedMicros() : us(micros()) {}
	operator uint32_t() const { return micros() - us; }
	elapsedMicros& operator=(uint32_t v) { us = micros() - v; return *this; }
};

// ---- random
long random(long howbig);
long random(long howsmall, long howbig);
void randomSeed(unsigned long seed);

// ---- serial (debug output goes nowhere)
struct SerialStub {
	void begin(long) {}
#ifdef PRINT_DEBUG
	void print(const char* s) { fputs(s, stderr); }
	void print(char* s) { fputs(s, stderr); }
#endif
	template <typename... T> void print(T...) {}
	template <typename... T> void println(T...) {}
	template <typename... T> void printf(T...) {}
	operator bool() const { return false; }
};
extern SerialStub Serial;

// Arduino's min/max/constrain/abs are macros; the firmware relies on the generic versions
#ifndef constrain
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#endif
using std::min;
using std::max;
