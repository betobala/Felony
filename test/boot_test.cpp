// Boots the firmware in real time, presses a few controls and prints the screen as text.
#include "../src/OcRunner.hpp"
#include <Arduino.h>
#include <chrono>
#include <thread>
#include <cstdio>
#include <algorithm>

static void run(double seconds) {
	auto t0 = std::chrono::steady_clock::now();
	for (int i = 0; i < (int) (seconds * 1000); i++) {
		ocrun::advance(1000.0);
		std::this_thread::sleep_until(t0 + std::chrono::microseconds(1000 * (i + 1)));
	}
}

static int dumps = 0;
static void dump(const char* title) {
	char name[64];
	snprintf(name, sizeof(name), "build/fw/screen%02d.pbm", dumps++);
	if (FILE* f = fopen(name, "w")) {
		fprintf(f, "P1\n# %s\n128 64\n", title);
		for (int y = 0; y < 64; y++) {
			for (int x = 0; x < 128; x++)
				fputs(ochw::hw.screen[y / 8][x] >> (y % 8) & 1 ? "1 " : "0 ", f);
			fputc('\n', f);
		}
		fclose(f);
	}
	printf("---- %s (version %u)\n", title, ochw::hw.screenVersion.load());
	for (int y = 0; y < 64; y += 2) {
		for (int x = 0; x < 128; x++) {
			bool a = ochw::hw.screen[y / 8][x] >> (y % 8) & 1, b = ochw::hw.screen[(y + 1) / 8][x] >> ((y + 1) % 8) & 1;
			putchar(a && b ? '#' : a ? '\'' : b ? '.' : ' ');
		}
		putchar('\n');
	}
}

static void press(ocrun::ButtonId b, double seconds) {
	ocrun::setButton(b, true);
	run(seconds);
	ocrun::setButton(b, false);
	run(0.3);
}

int main(int argc, char** argv) {
	ocrun::start();
	run(4.0);
	dump("after boot");
	press(ocrun::BUTTON_RIGHT, 1.3);  // long press: app list
	dump("app list");
	for (int i = 0; i < 7; i++) {
		ocrun::turn(ocrun::ENC_RIGHT, 1);
		run(0.15);
	}
	dump("app list +7");
	press(ocrun::BUTTON_RIGHT, 0.05);
	run(1.0);
	dump("selected app");
	// edit the attack of channel A: press right to edit, turn, press again
	press(ocrun::BUTTON_RIGHT, 0.05);
	for (int i = 0; i < 6; i++) {
		ocrun::turn(ocrun::ENC_RIGHT, 1);
		run(0.12);
	}
	press(ocrun::BUTTON_RIGHT, 0.05);
	dump("attack changed");
	for (int k = 0; k < 3; k++) {
		ocrun::setGate(0, true);
		run(0.25);
		ocrun::setGate(0, false);
		run(0.15);
	}
	ocrun::setGate(0, true);
	run(0.12);
	dump("envelope running (gate on)");
	float peak = 0;
	for (int i = 0; i < 300; i++) {
		run(0.001);
		peak = std::max(peak, ocrun::outputVolts(0));
	}
	printf("out A peak during gate: %.3f V\n", peak);
	ocrun::setGate(0, false);
	// save as the module does before writing the patch, then power cycle with that EEPROM
	ochw::hw.saveRequest = true;
	run(0.3);
	printf("save done: %d\n", !ochw::hw.saveRequest);
	ocrun::stop();
	ocrun::start();
	run(4.0);
	dump("after reboot");
	ocrun::stop();
	printf("stopped, running=%d\n", ocrun::running());
	return 0;
}
