// Felony: the Ornament & Crime firmware (mxmxmx/O_C, MIT) running in VCV Rack.
// The firmware in src/oc is the original code; only the hardware drivers were swapped for the
// virtual ones in shim/ (see tools/port.py). This file is the panel and the I/O.
#include <rack.hpp>
#include <thread>
#include <chrono>
#include "OcRunner.hpp"
#include "oc_hw.h"
#include "Controls4ms.hpp"
#include "Stencil.hpp"

using namespace rack;

Plugin* pluginInstance;

// The firmware keeps its state in globals, so only one module can own it per Rack process.
static std::atomic<void*> owner{nullptr};

struct Felony : Module {
	enum ParamId { PARAMS_LEN };
	enum InputId { ENUMS(TR_INPUT, 4), ENUMS(CV_INPUT, 4), INPUTS_LEN };
	enum OutputId { ENUMS(OUT_OUTPUT, 4), OUTPUTS_LEN };
	enum LightId { LIGHTS_LEN };

	bool active = false;     // this instance owns the firmware
	bool started = false;
	dsp::SchmittTrigger gates[4];
	double sinceTr4 = 0.0;   // for the TR4 frequency meter (tuner)
	bool tr4WasHigh = false;
	int theme = 0;
	int screenColor = 0;
	// EEPROM image loaded from the patch, applied when the firmware (re)boots
	std::vector<uint8_t> pendingEeprom;

	Felony() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int i = 0; i < 4; i++) {
			configInput(TR_INPUT + i, string::f("TR%d (gate/trigger)", i + 1));
			configInput(CV_INPUT + i, string::f("CV%d (-3.3..+6.6 V)", i + 1));
			configOutput(OUT_OUTPUT + i, std::string(1, (char) ('A' + i)));
		}
		void* expected = nullptr;
		active = owner.compare_exchange_strong(expected, this);
	}

	~Felony() {
		if (active) {
			ocrun::stop();
			owner = nullptr;
		}
	}

	void boot() {
		ocrun::stop();
		if (!pendingEeprom.empty()) {
			memcpy(ochw::hw.eeprom, pendingEeprom.data(), std::min(pendingEeprom.size(), sizeof(ochw::hw.eeprom)));
			pendingEeprom.clear();
		}
		ocrun::start();
		started = true;
	}

	void process(const ProcessArgs& args) override {
		if (!active)
			return;
		if (!started)
			boot();

		for (int i = 0; i < 4; i++) {
			ochw::hw.cv[i] = inputs[CV_INPUT + i].getVoltage();
			float v = inputs[TR_INPUT + i].getVoltage();
			// the O&C input stage switches around 1 V
			gates[i].process(v, 0.6f, 1.2f);
			ocrun::setGate(i, gates[i].isHigh());
		}
		// period of TR4 for the tuner (REFS app), in F_BUS cycles like the Teensy FTM capture
		sinceTr4 += args.sampleTime;
		bool tr4 = gates[3].isHigh();
		if (tr4 && !tr4WasHigh && ochw::hw.freqEnabled) {
			ochw::hw.freqCount = (uint32_t) (sinceTr4 * 60e6);
			sinceTr4 = 0.0;
		}
		tr4WasHigh = tr4;

		ocrun::advance(1e6 * args.sampleTime);

		for (int i = 0; i < 4; i++)
			outputs[OUT_OUTPUT + i].setVoltage(ocrun::outputVolts(i));
	}

	json_t* dataToJson() override {
		json_t* root = json_object();
		json_object_set_new(root, "theme", json_integer(theme));
		json_object_set_new(root, "screenColor", json_integer(screenColor));
		if (active && started && ocrun::running()) {
			// let the firmware store the current app and its settings first (like a long press
			// in the app list), so the patch reopens exactly where it was
			ochw::hw.saveRequest = true;
			for (int i = 0; i < 500 && ochw::hw.saveRequest; i++)
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		if (active)
			json_object_set_new(root, "eeprom", json_string(string::toBase64(ochw::hw.eeprom, sizeof(ochw::hw.eeprom)).c_str()));
		return root;
	}

	void dataFromJson(json_t* root) override {
		if (json_t* t = json_object_get(root, "theme"))
			theme = clamp((int) json_integer_value(t), 0, 1);
		if (json_t* c = json_object_get(root, "screenColor"))
			screenColor = clamp((int) json_integer_value(c), 0, 3);
		if (json_t* e = json_object_get(root, "eeprom")) {
			pendingEeprom = string::fromBase64(json_string_value(e));
			started = false;  // reboot with the saved EEPROM on the next sample
		}
	}
};

// ---------------------------------------------------------------- themes
// betobullet stencil (docs/art/STYLE.md): theme 0 = black panel, 1 = white panel
using bb::THEME_NAMES;

static const char* const SCREEN_COLOR_NAMES[4] = {"White", "Blue", "Yellow", "Green"};
static NVGcolor screenColor(int i) {
	switch (i) {
		case 1: return nvgRGB(0x7F, 0xC8, 0xFF);
		case 2: return nvgRGB(0xFF, 0xD8, 0x4A);
		case 3: return nvgRGB(0x7C, 0xFF, 0x9A);
		default: return nvgRGB(0xF2, 0xF6, 0xFF);
	}
}

namespace fms {
const Colors& colors(const engine::Module* m) {
	static const Colors black = bb::controlColors(0), white = bb::controlColors(1);
	int t = m ? static_cast<const Felony*>(m)->theme : 0;
	return t == 1 ? white : black;
}
}  // namespace fms

static std::shared_ptr<window::Font> pluginFont(const char* file) {
	std::shared_ptr<window::Font> f = APP->window->loadFont(asset::plugin(pluginInstance, std::string("res/fonts/") + file));
	if (!f)
		f = APP->window->loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
	return f;
}

// ---------------------------------------------------------------- layout (mm), 14 HP, after concept light v2
static const float W = 71.12f;
static const float CX = W / 2.f;
static const Rect SCREEN(Vec(5.56f, 9.6f), Vec(60.0f, 30.0f));
static const Rect ENC_GROUP(Vec(2.6f, 43.4f), Vec(W - 5.2f, 26.4f));
static const Rect BTN_GROUP(Vec(28.8f, 45.4f), Vec(13.52f, 22.4f));
static const Vec ENC_L(15.4f, 54.6f), ENC_R(55.72f, 54.6f);
static const float ENC_PILL_Y = 65.4f;
static const Vec BTN_UP(CX, 49.6f), BTN_DOWN(CX, 59.0f);
static const float COLS[4] = {11.4f, 27.5f, 43.62f, 59.72f};
static const float GROUP_TOP[3] = {72.0f, 89.0f, 106.0f};
static const float GROUP_H = 15.6f;
static const float PILL_DY = 3.0f, JACK_DY = 10.2f;  // below the group top
static const float ENC_R_MM = 8.2f;

// ---------------------------------------------------------------- widgets
/** The 128 x 64 OLED, read straight from the virtual SH1106 RAM. */
struct OledDisplay : TransparentWidget {
	Felony* module = nullptr;

	void draw(const DrawArgs& args) override {
		NVGcontext* vg = args.vg;
		// bezel and glass
		nvgBeginPath(vg);
		nvgRoundedRect(vg, -mm2px(1.6f), -mm2px(1.6f), box.size.x + mm2px(3.2f), box.size.y + mm2px(3.2f), mm2px(1.4f));
		nvgFillColor(vg, nvgRGB(0x0A, 0x0B, 0x0D));
		nvgFill(vg);
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, nvgRGB(0x02, 0x03, 0x05));
		nvgFill(vg);
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1)
			return;
		NVGcontext* vg = args.vg;
		float px = box.size.x / 128.f, py = box.size.y / 64.f;
		NVGcolor c = screenColor(module ? module->screenColor : 0);
		if (!module) {
			// browser preview: the app name as the firmware would show it
			nvgFillColor(vg, c);
			nvgFontSize(vg, py * 10);
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgText(vg, box.size.x / 2, box.size.y / 2, "O & C", NULL);
			return;
		}
		if (!module->active) {
			nvgFillColor(vg, c);
			nvgFontSize(vg, py * 7);
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgText(vg, box.size.x / 2, box.size.y * 0.4f, "one Felony per rack:", NULL);
			nvgText(vg, box.size.x / 2, box.size.y * 0.6f, "the firmware is global", NULL);
			return;
		}
		nvgFillColor(vg, c);
		nvgBeginPath(vg);
		for (int page = 0; page < 8; page++) {
			for (int x = 0; x < 128; x++) {
				uint8_t bits = ochw::hw.screen[page][x];
				if (!bits)
					continue;
				for (int b = 0; b < 8; b++) {
					if (bits & (1 << b)) {
						int y = page * 8 + b;
						// pixels a hair smaller than the grid, like a real OLED
						nvgRect(vg, x * px + px * 0.06f, y * py + py * 0.06f, px * 0.88f, py * 0.88f);
					}
				}
			}
		}
		nvgFill(vg);
	}
};

/** Shared tap/hold/turn handling: a click is a button press, a drag or wheel turns the encoder. */
struct EncoderWidget : OpaqueWidget {
	Felony* module = nullptr;
	ocrun::Control control = ocrun::ENC_LEFT;
	ocrun::ButtonId button = ocrun::BUTTON_LEFT;
	float acc = 0.f, angle = 0.f;
	bool down = false, moved = false, pressSent = false;
	double downTime = 0.0, releaseAt = -1.0;

	bool live() {
		return module && module->active;
	}

	void turn(int detents) {
		if (!live() || detents == 0)
			return;
		ocrun::turn(control, detents);
		angle += detents * (float) M_PI / 12.f;
	}

	void onButton(const ButtonEvent& e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
			down = true;
			moved = pressSent = false;
			acc = 0.f;
			downTime = system::getTime();
			e.consume(this);
			return;
		}
		OpaqueWidget::onButton(e);
	}

	void onDragMove(const DragMoveEvent& e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT || pressSent)
			return;
		acc += (e.mouseDelta.x - e.mouseDelta.y) / getAbsoluteZoom();
		const float STEP = 8.f;
		int steps = (int) (acc / STEP);
		if (steps != 0) {
			moved = true;
			acc -= steps * STEP;
			turn(steps);
		}
	}

	void onDragEnd(const DragEndEvent& e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;
		down = false;
		if (pressSent) {
			ocrun::setButton(button, false);
			pressSent = false;
		}
		else if (!moved && live()) {
			// quick click: hold the pin long enough for the 8 ms debounce
			ocrun::setButton(button, true);
			releaseAt = system::getTime() + 0.04;
		}
	}

	void onHoverScroll(const HoverScrollEvent& e) override {
		turn(e.scrollDelta.y > 0 ? 1 : (e.scrollDelta.y < 0 ? -1 : 0));
		e.consume(this);
	}

	void step() override {
		double now = system::getTime();
		// held still: it is a press (long presses need this)
		if (down && !moved && !pressSent && live() && now - downTime > 0.18) {
			ocrun::setButton(button, true);
			pressSent = true;
		}
		if (releaseAt >= 0.0 && now >= releaseAt) {
			ocrun::setButton(button, false);
			releaseAt = -1.0;
		}
		OpaqueWidget::step();
	}

	void draw(const DrawArgs& args) override {
		const fms::Colors& c = fms::colors(module);
		NVGcontext* vg = args.vg;
		float cx = box.size.x / 2, cy = box.size.y / 2, r = mm2px(ENC_R_MM);
		fms::softShadow(vg, cx, cy, r, r * 0.1f, 0.3f);
		fms::circle(vg, cx, cy, r, c.knobEdge, c.knob, 0.f);
		for (int i = 0; i < 40; i++) {
			float a = angle + i * 2.f * (float) M_PI / 40.f;
			nvgBeginPath(vg);
			nvgMoveTo(vg, cx + std::sin(a) * r * 0.86f, cy - std::cos(a) * r * 0.86f);
			nvgLineTo(vg, cx + std::sin(a) * r, cy - std::cos(a) * r);
			nvgStrokeColor(vg, c.knob);
			nvgStrokeWidth(vg, std::max(1.f, r * 0.06f));
			nvgStroke(vg);
		}
		bool pressed = pressSent || releaseAt >= 0.0;
		fms::circle(vg, cx, cy, r * (pressed ? 0.76f : 0.8f), c.knob, c.knob, 0.f);
		nvgBeginPath(vg);
		nvgMoveTo(vg, cx + std::sin(angle) * r * 0.18f, cy - std::cos(angle) * r * 0.18f);
		nvgLineTo(vg, cx + std::sin(angle) * r * 0.7f, cy - std::cos(angle) * r * 0.7f);
		nvgStrokeColor(vg, c.pointer);
		nvgStrokeWidth(vg, std::max(1.5f, r * 0.11f));
		nvgLineCap(vg, NVG_BUTT);
		nvgStroke(vg);
	}
};

/** The up/down buttons: pressed while the mouse is held (at least 40 ms). */
struct PanelButton : OpaqueWidget {
	Felony* module = nullptr;
	ocrun::ButtonId button = ocrun::BUTTON_UP;
	bool down = false;
	double downTime = 0.0, releaseAt = -1.0;

	void onButton(const ButtonEvent& e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
			if (module && module->active)
				ocrun::setButton(button, true);
			down = true;
			downTime = system::getTime();
			e.consume(this);
			return;
		}
		OpaqueWidget::onButton(e);
	}

	void onDragEnd(const DragEndEvent& e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;
		down = false;
		releaseAt = std::max(system::getTime(), downTime + 0.04);
	}

	void step() override {
		if (releaseAt >= 0.0 && system::getTime() >= releaseAt) {
			if (module && module->active)
				ocrun::setButton(button, false);
			releaseAt = -1.0;
		}
		OpaqueWidget::step();
	}

	void draw(const DrawArgs& args) override {
		const fms::Colors& c = fms::colors(module);
		NVGcontext* vg = args.vg;
		float cx = box.size.x / 2, cy = box.size.y / 2, r = box.size.x / 2 * 0.86f;
		bool on = down || releaseAt >= 0.0;
		// black cap like the encoders; it sinks a little while pressed
		fms::softShadow(vg, cx, cy, r, r * (on ? 0.04f : 0.14f), 0.3f);
		fms::circle(vg, cx, cy, r, c.knob, c.knobEdge, std::max(1.f, r * 0.08f));
		NVGpaint hi = nvgRadialGradient(vg, cx - r * 0.25f, cy - r * 0.3f, r * 0.05f, r * 0.9f,
		                                nvgRGBAf(1.f, 1.f, 1.f, on ? 0.04f : 0.10f), nvgRGBAf(1.f, 1.f, 1.f, 0.f));
		nvgBeginPath(vg);
		nvgCircle(vg, cx, cy, r * 0.9f);
		nvgFillPaint(vg, hi);
		nvgFill(vg);
	}
};

/** Black panel, stencil frames, hazard band and the knocked-out label plates. */
struct PanelArt : Widget {
	Felony* module = nullptr;

	void draw(const DrawArgs& args) override {
		bb::Ink k = bb::ink(module ? module->theme : 0);
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, k.bg);
		nvgFill(vg);

		bb::bracketFrame(vg, Rect(SCREEN.pos.minus(Vec(1.4f, 1.4f)), SCREEN.size.plus(Vec(2.8f, 2.8f))), k, 3.4f);
		bb::notchedFrame(vg, Rect(Vec(2.4f, 43.2f), Vec(25.4f, 25.2f)), k);
		bb::notchedFrame(vg, Rect(Vec(W - 27.8f, 43.2f), Vec(25.4f, 25.2f)), k);
		bb::frame(vg, Rect(Vec(30.2f, 43.2f), Vec(W - 60.4f, 25.2f)), k, 1.6f, 0.45f);
		// the band between the controls and the jacks: cartridges and hazard stripes
		bb::bullets(vg, 4.0f, 14.0f, 71.4f, 2.2f, 5, k);
		bb::bullets(vg, W - 14.0f, W - 4.0f, 71.4f, 2.2f, 5, k);
		bb::hazard(vg, Rect(Vec(17.0f, 69.3f), Vec(W - 34.0f, 2.1f)), k);
		for (int r = 0; r < 3; r++) {
			Rect row(Vec(2.4f, GROUP_TOP[r] + 0.3f), Vec(W - 4.8f, GROUP_H - 0.6f));
			if (r == 2) {
				// the outputs: a solid plate in the panel ink
				bb::chamferPath(vg, row, 2.0f);
				nvgFillColor(vg, k.fg);
				nvgFill(vg);
			}
			else {
				bb::frame(vg, row, k);
				for (int i = 0; i < 3; i++)
					bb::link(vg, COLS[i], COLS[i + 1], GROUP_TOP[r] + JACK_DY, k);
			}
		}

		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		if (!stencil)
			return;
		bb::title(vg, Rect(Vec(14.0f, 1.4f), Vec(W - 28.0f, 6.2f)), "FELONY", stencil->handle, k, 4.6f);
		nvgFontFaceId(vg, stencil->handle);
		nvgFontSize(vg, mm2px(2.5f));
		nvgTextLetterSpacing(vg, mm2px(0.12f));
		bb::plate(vg, Vec(ENC_L.x, ENC_PILL_Y), "LEFT", k, 12.0f);
		bb::plate(vg, Vec(ENC_R.x, ENC_PILL_Y), "RIGHT", k, 12.0f);
		bb::plate(vg, Vec(CX, BTN_UP.y + 4.6f), "UP", k, 7.6f);
		bb::plate(vg, Vec(CX, BTN_DOWN.y + 4.6f), "DOWN", k, 7.6f);
		bb::Ink inv = {k.fg, k.bg};
		for (int i = 0; i < 4; i++) {
			bb::plate(vg, Vec(COLS[i], GROUP_TOP[0] + PILL_DY), string::f("TR%d", i + 1).c_str(), k);
			bb::plate(vg, Vec(COLS[i], GROUP_TOP[1] + PILL_DY), string::f("CV%d", i + 1).c_str(), k);
			bb::plate(vg, Vec(COLS[i], GROUP_TOP[2] + PILL_DY), std::string(1, (char) ('A' + i)).c_str(), inv);
		}
		nvgTextLetterSpacing(vg, 0.f);
	}
};

struct FelonyWidget : ModuleWidget {
	FelonyWidget(Felony* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Felony.svg")));

		PanelArt* art = new PanelArt;
		art->module = module;
		art->box.size = box.size;
		addChild(art);

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		OledDisplay* oled = new OledDisplay;
		oled->module = module;
		oled->box.pos = mm2px(SCREEN.pos);
		oled->box.size = mm2px(SCREEN.size);
		addChild(oled);

		for (int i = 0; i < 2; i++) {
			EncoderWidget* enc = new EncoderWidget;
			enc->module = module;
			enc->control = i == 0 ? ocrun::ENC_LEFT : ocrun::ENC_RIGHT;
			enc->button = i == 0 ? ocrun::BUTTON_LEFT : ocrun::BUTTON_RIGHT;
			enc->box.size = mm2px(Vec(2 * ENC_R_MM + 2.f, 2 * ENC_R_MM + 2.f));
			enc->box.pos = mm2px(i == 0 ? ENC_L : ENC_R).minus(enc->box.size.div(2));
			addChild(enc);

			PanelButton* b = new PanelButton;
			b->module = module;
			b->button = i == 0 ? ocrun::BUTTON_UP : ocrun::BUTTON_DOWN;
			b->box.size = mm2px(Vec(6.8f, 6.8f));
			b->box.pos = mm2px(i == 0 ? BTN_UP : BTN_DOWN).minus(b->box.size.div(2));
			addChild(b);
		}

		for (int i = 0; i < 4; i++) {
			addInput(createInputCentered<fms::Jack>(mm2px(Vec(COLS[i], GROUP_TOP[0] + JACK_DY)), module, Felony::TR_INPUT + i));
			addInput(createInputCentered<fms::Jack>(mm2px(Vec(COLS[i], GROUP_TOP[1] + JACK_DY)), module, Felony::CV_INPUT + i));
			addOutput(createOutputCentered<fms::Jack>(mm2px(Vec(COLS[i], GROUP_TOP[2] + JACK_DY)), module, Felony::OUT_OUTPUT + i));
		}
	}

	void appendContextMenu(Menu* menu) override {
		Felony* m = getModule<Felony>();
		if (!m)
			return;
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Encoders: click = press, drag/wheel = turn, hold = long press"));
		menu->addChild(createIndexPtrSubmenuItem("Screen colour", std::vector<std::string>(SCREEN_COLOR_NAMES, SCREEN_COLOR_NAMES + 4), &m->screenColor));
		menu->addChild(createIndexPtrSubmenuItem("Theme", std::vector<std::string>(THEME_NAMES, THEME_NAMES + 2), &m->theme));
		if (m->active)
			menu->addChild(createMenuItem("Power cycle", "", [=]() { m->started = false; }));
	}
};

Model* modelFelony = createModel<Felony, FelonyWidget>("Felony");

void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(modelFelony);
}
