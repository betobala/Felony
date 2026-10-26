#pragma once
// Runs the O&C firmware: setup()/loop() on their own thread, the timer ISRs from the caller's
// clock (the Rack engine), and the panel controls as pin activity.
#include <stdint.h>

namespace ocrun {

enum Control { ENC_LEFT, ENC_RIGHT };
enum ButtonId { BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT, NUM_BUTTONS };

/** Starts the firmware thread (boots like a power-up). */
void start();
/** Stops and joins the firmware thread. */
void stop();
bool running();

/** Advances the virtual clock by dtUs microseconds, running the timer ISRs that fall due. */
void advance(double dtUs);

/** Queues encoder detents (positive = clockwise). */
void turn(Control enc, int detents);
void setButton(ButtonId b, bool pressed);
/** Gate at trigger input 0..3 (high = gate on). */
void setGate(int input, bool high);
/** Output code of DAC channel 0..3 converted to volts. */
float outputVolts(int channel);

}  // namespace ocrun
