// Felony: the Ornament & Crime firmware (mxmxmx/O_C, MIT) running in VCV Rack.
// The firmware in src/oc is the original code; only the hardware drivers were swapped for the
// virtual ones in shim/ (see tools/port.py). This file is the panel and the I/O.
#include <rack.hpp>
#include <thread>
#include <chrono>
#include "OcRunner.hpp"
#include "oc_hw.h"
#include "Controls4ms.hpp"

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
struct FTheme {
	NVGcolor panel, ink, grey, plate, plateInk, accent;
};
static const char* const THEME_NAMES[2] = {"Bone", "Slate"};

static FTheme fTheme(int t) {
	FTheme k;
	if (t == 1) {
		k.panel = nvgRGB(0x17, 0x19, 0x1B);
		k.ink = nvgRGB(0xE6, 0xE8, 0xE8);
		k.grey = nvgRGB(0x8A, 0x8D, 0x93);
		k.plate = nvgRGB(0x0E, 0x11, 0x13);
		k.plateInk = nvgRGB(0xE6, 0xE8, 0xE8);
		k.accent = nvgRGB(0xE8, 0xB3, 0x3C);
	}
	else {
		k.panel = nvgRGB(0xE8, 0xE6, 0xE1);
		k.ink = nvgRGB(0x0F, 0x12, 0x16);
		k.grey = nvgRGB(0x6E, 0x71, 0x77);
		k.plate = nvgRGB(0x1B, 0x1E, 0x24);
		k.plateInk = nvgRGB(0xF6, 0xF4, 0xEF);
		k.accent = nvgRGB(0xC8, 0x8A, 0x00);
	}
	return k;
}

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
	static const Colors bone = {
		nvgRGB(0x11, 0x11, 0x11), nvgRGB(0x3A, 0x3A, 0x3A), nvgRGB(0xF6, 0xF4, 0xEF), nvgRGB(0x0F, 0x12, 0x16),
		nvgRGB(0xD3, 0xD3, 0xD0), nvgRGB(0xF7, 0xF7, 0xF5), nvgRGB(0x8E, 0x8E, 0x8A), nvgRGB(0x5C, 0x5C, 0x5A), nvgRGB(0x05, 0x05, 0x05),
		nvgRGB(0xF4, 0xF2, 0xEA), nvgRGB(0xFF, 0xFF, 0xFF), nvgRGB(0xE8, 0xB3, 0x3C), nvgRGB(0xFF, 0xDD, 0x8A), nvgRGB(0x0F, 0x12, 0x16),
		nvgRGB(0xFB, 0xFB, 0xF9)};
	static const Colors slate = {
		nvgRGB(0x0B, 0x0B, 0x0D), nvgRGB(0x3A, 0x3B, 0x3F), nvgRGB(0xF6, 0xF4, 0xEF), nvgRGB(0xE6, 0xE8, 0xE8),
		nvgRGB(0xD3, 0xD3, 0xD0), nvgRGB(0xF7, 0xF7, 0xF5), nvgRGB(0x8E, 0x8E, 0x8A), nvgRGB(0x5C, 0x5C, 0x5A), nvgRGB(0x05, 0x05, 0x05),
		nvgRGB(0x2A, 0x2C, 0x30), nvgRGB(0x40, 0x42, 0x46), nvgRGB(0xE8, 0xB3, 0x3C), nvgRGB(0xFF, 0xDD, 0x8A), nvgRGB(0x6B, 0x6F, 0x75),
		nvgRGB(0xFB, 0xFB, 0xF9)};
	int t = m ? static_cast<const Felony*>(m)->theme : 0;
	return t == 1 ? slate : bone;
}
}  // namespace fms

static std::shared_ptr<window::Font> pluginFont(const char* file) {
	std::shared_ptr<window::Font> f = APP->window->loadFont(asset::plugin(pluginInstance, std::string("res/fonts/") + file));
	if (!f)
		f = APP->window->loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
	return f;
}

// ---------------------------------------------------------------- layout (mm), 14 HP like the O&C
static const float W = 71.12f;
static const float CX = W / 2.f;
static const Rect SCREEN(Vec(5.56f, 11.0f), Vec(60.0f, 30.0f));
static const Vec ENC_L(13.6f, 54.0f), ENC_R(57.52f, 54.0f);
static const Vec BTN_UP(CX, 48.6f), BTN_DOWN(CX, 59.4f);
static const float COLS[4] = {10.5f, 27.2f, 43.92f, 60.62f};
static const float ROW_TR = 77.0f, ROW_CV = 93.0f, ROW_OUT = 111.0f;
static const float ENC_R_MM = 6.2f;

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
		float dx = std::sin(angle) * r * 0.52f, dy = -std::cos(angle) * r * 0.52f;
		fms::circle(vg, cx + dx, cy + dy, r * 0.1f, c.pointer, c.pointer, 0.f);
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
		fms::softShadow(vg, cx, cy, r, r * 0.12f, 0.26f);
		fms::circle(vg, cx, cy, r, on ? c.btnOn : c.btnOff, c.outline, std::max(1.f, r * 0.12f));
	}
};

/** Panel background and labels. */
struct PanelArt : Widget {
	Felony* module = nullptr;

	void text(NVGcontext* vg, Vec mm, const char* s) {
		nvgText(vg, mm2px(mm.x), mm2px(mm.y), s, NULL);
	}

	void draw(const DrawArgs& args) override {
		FTheme k = fTheme(module ? module->theme : 0);
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, k.panel);
		nvgFill(vg);

		// outputs on a dark plate
		nvgBeginPath(vg);
		nvgRoundedRect(vg, mm2px(3.0f), mm2px(ROW_OUT - 5.6f), mm2px(W - 6.0f), mm2px(13.6f), mm2px(2.6f));
		nvgFillColor(vg, k.plate);
		nvgFill(vg);

		std::shared_ptr<window::Font> bold = pluginFont("SpaceMono-Bold.ttf");
		std::shared_ptr<window::Font> reg = pluginFont("SpaceMono-Regular.ttf");
		if (!bold || !reg)
			return;
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFontFaceId(vg, bold->handle);
		nvgFontSize(vg, mm2px(4.0f));
		nvgFillColor(vg, k.ink);
		text(vg, Vec(CX, 5.6f), "felony");

		nvgFontFaceId(vg, reg->handle);
		nvgFontSize(vg, mm2px(2.6f));
		text(vg, Vec(BTN_UP.x, BTN_UP.y - 4.4f), "up");
		text(vg, Vec(BTN_DOWN.x, BTN_DOWN.y + 4.4f), "down");
		nvgFontSize(vg, mm2px(2.8f));
		for (int i = 0; i < 4; i++) {
			nvgFillColor(vg, k.ink);
			text(vg, Vec(COLS[i], ROW_TR + 5.4f), string::f("TR%d", i + 1).c_str());
			text(vg, Vec(COLS[i], ROW_CV + 5.4f), string::f("CV%d", i + 1).c_str());
			nvgFillColor(vg, k.plateInk);
			text(vg, Vec(COLS[i], ROW_OUT + 5.4f), std::string(1, (char) ('A' + i)).c_str());
		}
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
			b->box.size = mm2px(Vec(6.4f, 6.4f));
			b->box.pos = mm2px(i == 0 ? BTN_UP : BTN_DOWN).minus(b->box.size.div(2));
			addChild(b);
		}

		for (int i = 0; i < 4; i++) {
			addInput(createInputCentered<fms::Jack>(mm2px(Vec(COLS[i], ROW_TR)), module, Felony::TR_INPUT + i));
			addInput(createInputCentered<fms::Jack>(mm2px(Vec(COLS[i], ROW_CV)), module, Felony::CV_INPUT + i));
			addOutput(createOutputCentered<fms::Jack>(mm2px(Vec(COLS[i], ROW_OUT)), module, Felony::OUT_OUTPUT + i));
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
