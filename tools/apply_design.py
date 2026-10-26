"""One-off: apply the light v2 concept (docs/art) to src/Felony.cpp."""
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
p = os.path.join(ROOT, 'src', 'Felony.cpp')
s = open(p, encoding='utf-8').read()


def rep(a, b):
    global s
    assert a in s, a[:60]
    s = s.replace(a, b, 1)


def block(start, end, new):
    global s
    i, j = s.index(start), s.index(end)
    s = s[:i] + new + s[j:]


block('struct FTheme {', 'static const char* const SCREEN_COLOR_NAMES', r'''struct FTheme {
	NVGcolor panel, ink, grey, line, group, plate, plateInk, pill, pillInk, accent;
};
static const char* const THEME_NAMES[2] = {"Bone", "Slate"};

// Bone (light, main) and Slate (dark), after concepts light v2 / dark v2 (docs/art/STYLE.md)
static FTheme fTheme(int t) {
	FTheme k;
	if (t == 1) {
		k.panel = nvgRGB(0x17, 0x19, 0x1B);
		k.ink = nvgRGB(0xE6, 0xE8, 0xE8);
		k.grey = nvgRGB(0x8A, 0x8D, 0x93);
		k.line = nvgRGB(0x6B, 0x6F, 0x75);
		k.group = nvgRGB(0x1B, 0x1D, 0x20);
		k.plate = nvgRGB(0x0E, 0x10, 0x12);
		k.plateInk = nvgRGB(0xE6, 0xE8, 0xE8);
		k.pill = nvgRGB(0xD6, 0xD8, 0xD8);
		k.pillInk = nvgRGB(0x17, 0x19, 0x1B);
		k.accent = nvgRGB(0xE8, 0xB3, 0x3C);
	}
	else {
		k.panel = nvgRGB(0xE8, 0xE6, 0xE1);
		k.ink = nvgRGB(0x0F, 0x12, 0x16);
		k.grey = nvgRGB(0x6E, 0x71, 0x77);
		k.line = nvgRGB(0x0F, 0x12, 0x16);
		k.group = nvgRGB(0xEE, 0xEC, 0xE7);
		k.plate = nvgRGB(0x14, 0x16, 0x1A);
		k.plateInk = nvgRGB(0xF6, 0xF4, 0xEF);
		k.pill = nvgRGB(0x14, 0x16, 0x1A);
		k.pillInk = nvgRGB(0xF6, 0xF4, 0xEF);
		k.accent = nvgRGB(0xE8, 0xA3, 0x17);
	}
	return k;
}

''')

rep("nvgRGB(0xF4, 0xF2, 0xEA), nvgRGB(0xFF, 0xFF, 0xFF), nvgRGB(0xE8, 0xB3, 0x3C), nvgRGB(0xFF, 0xDD, 0x8A), nvgRGB(0x0F, 0x12, 0x16),",
    "nvgRGB(0x1B, 0x1E, 0x24), nvgRGB(0x3A, 0x3D, 0x44), nvgRGB(0xE8, 0xA3, 0x17), nvgRGB(0xFF, 0xD2, 0x6E), nvgRGB(0x0F, 0x12, 0x16),")
rep("nvgRGB(0x2A, 0x2C, 0x30), nvgRGB(0x40, 0x42, 0x46), nvgRGB(0xE8, 0xB3, 0x3C), nvgRGB(0xFF, 0xDD, 0x8A), nvgRGB(0x6B, 0x6F, 0x75),",
    "nvgRGB(0x0B, 0x0B, 0x0D), nvgRGB(0x2A, 0x2C, 0x30), nvgRGB(0xE8, 0xB3, 0x3C), nvgRGB(0xFF, 0xDD, 0x8A), nvgRGB(0x6B, 0x6F, 0x75),")

block('// ---------------------------------------------------------------- layout', '// ---------------------------------------------------------------- widgets', r'''// ---------------------------------------------------------------- layout (mm), 14 HP, after concept light v2
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

''')

# encoder: a small accent dot as the pointer, like the concept
rep("""		float dx = std::sin(angle) * r * 0.52f, dy = -std::cos(angle) * r * 0.52f;
		fms::circle(vg, cx + dx, cy + dy, r * 0.1f, c.pointer, c.pointer, 0.f);""",
    """		float dx = std::sin(angle) * r * 0.62f, dy = -std::cos(angle) * r * 0.62f;
		fms::circle(vg, cx + dx, cy + dy, r * 0.075f, c.btnOn, c.btnOn, 0.f);""")

# up/down: black caps like the encoders
rep("""		bool on = down || releaseAt >= 0.0;
		fms::softShadow(vg, cx, cy, r, r * 0.12f, 0.26f);
		fms::circle(vg, cx, cy, r, on ? c.btnOn : c.btnOff, c.outline, std::max(1.f, r * 0.12f));""",
    """		bool on = down || releaseAt >= 0.0;
		// black cap like the encoders; it sinks a little while pressed
		fms::softShadow(vg, cx, cy, r, r * (on ? 0.04f : 0.14f), 0.3f);
		fms::circle(vg, cx, cy, r, c.knob, c.knobEdge, std::max(1.f, r * 0.08f));
		NVGpaint hi = nvgRadialGradient(vg, cx - r * 0.25f, cy - r * 0.3f, r * 0.05f, r * 0.9f,
		                                nvgRGBAf(1.f, 1.f, 1.f, on ? 0.04f : 0.10f), nvgRGBAf(1.f, 1.f, 1.f, 0.f));
		nvgBeginPath(vg);
		nvgCircle(vg, cx, cy, r * 0.9f);
		nvgFillPaint(vg, hi);
		nvgFill(vg);""")

block('/** Panel background and labels. */', 'struct FelonyWidget', r'''/** The logotype, one SVG per theme. */
struct LogoWidget : TransparentWidget {
	Felony* module = nullptr;
	std::shared_ptr<window::Svg> svg[2];

	void load(float heightMm) {
		svg[0] = window::Svg::load(asset::plugin(pluginInstance, "res/logo_bone.svg"));
		svg[1] = window::Svg::load(asset::plugin(pluginInstance, "res/logo_slate.svg"));
		float w = svg[0]->handle->width, h = svg[0]->handle->height;
		box.size = Vec(mm2px(heightMm) * w / h, mm2px(heightMm));
	}

	void draw(const DrawArgs& args) override {
		int i = (module && module->theme == 1) ? 1 : 0;
		if (!svg[i])
			return;
		float sc = box.size.x / svg[i]->handle->width;
		nvgSave(args.vg);
		nvgScale(args.vg, sc, sc);
		window::svgDraw(args.vg, svg[i]->handle);
		nvgRestore(args.vg);
	}
};

/** Panel background, outlined groups, the output box and the pill labels. */
struct PanelArt : Widget {
	Felony* module = nullptr;

	static void rrect(NVGcontext* vg, Rect mm, float radius) {
		nvgBeginPath(vg);
		nvgRoundedRect(vg, mm2px(mm.pos.x), mm2px(mm.pos.y), mm2px(mm.size.x), mm2px(mm.size.y), mm2px(radius));
	}

	void group(NVGcontext* vg, Rect mm, const FTheme& k) {
		rrect(vg, mm, 2.4f);
		nvgFillColor(vg, k.group);
		nvgFill(vg);
		nvgStrokeColor(vg, k.line);
		nvgStrokeWidth(vg, mm2px(0.3f));
		nvgStroke(vg);
	}

	/** Rounded pill with its label, centred at mm. */
	void pill(NVGcontext* vg, Vec mm, const char* s, NVGcolor fill, NVGcolor ink) {
		float w = std::max(mm2px(7.6f), nvgTextBounds(vg, 0, 0, s, NULL, NULL) + mm2px(2.4f));
		float h = mm2px(3.1f);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, mm2px(mm.x) - w / 2, mm2px(mm.y) - h / 2, w, h, h / 2);
		nvgFillColor(vg, fill);
		nvgFill(vg);
		nvgFillColor(vg, ink);
		nvgText(vg, mm2px(mm.x), mm2px(mm.y) + mm2px(0.1f), s, NULL);
	}

	void draw(const DrawArgs& args) override {
		FTheme k = fTheme(module ? module->theme : 0);
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, k.panel);
		nvgFill(vg);

		group(vg, ENC_GROUP, k);
		group(vg, BTN_GROUP, k);
		group(vg, Rect(Vec(2.6f, GROUP_TOP[0]), Vec(W - 5.2f, GROUP_H)), k);
		group(vg, Rect(Vec(2.6f, GROUP_TOP[1]), Vec(W - 5.2f, GROUP_H)), k);
		// outputs in the black box
		rrect(vg, Rect(Vec(2.6f, GROUP_TOP[2]), Vec(W - 5.2f, GROUP_H)), 2.4f);
		nvgFillColor(vg, k.plate);
		nvgFill(vg);
		if (module && module->theme == 1) {
			nvgStrokeColor(vg, k.line);
			nvgStrokeWidth(vg, mm2px(0.3f));
			nvgStroke(vg);
		}

		std::shared_ptr<window::Font> bold = pluginFont("SpaceMono-Bold.ttf");
		if (!bold)
			return;
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFontFaceId(vg, bold->handle);
		nvgFontSize(vg, mm2px(2.5f));
		nvgTextLetterSpacing(vg, mm2px(0.08f));
		pill(vg, Vec(ENC_L.x, ENC_PILL_Y), "LEFT", k.pill, k.pillInk);
		pill(vg, Vec(ENC_R.x, ENC_PILL_Y), "RIGHT", k.pill, k.pillInk);
		pill(vg, Vec(CX, BTN_UP.y + 4.6f), "UP", k.pill, k.pillInk);
		pill(vg, Vec(CX, BTN_DOWN.y + 4.6f), "DOWN", k.pill, k.pillInk);
		for (int i = 0; i < 4; i++) {
			pill(vg, Vec(COLS[i], GROUP_TOP[0] + PILL_DY), string::f("TR%d", i + 1).c_str(), k.pill, k.pillInk);
			pill(vg, Vec(COLS[i], GROUP_TOP[1] + PILL_DY), string::f("CV%d", i + 1).c_str(), k.pill, k.pillInk);
			// on the black box the pills turn light
			pill(vg, Vec(COLS[i], GROUP_TOP[2] + PILL_DY), std::string(1, (char) ('A' + i)).c_str(), k.plateInk, k.plate);
		}
		nvgTextLetterSpacing(vg, 0.f);
	}
};

''')

rep("""		OledDisplay* oled = new OledDisplay;""", """		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(4.6f);
		logo->box.pos = Vec((box.size.x - logo->box.size.x) / 2, mm2px(2.4f));
		addChild(logo);

		OledDisplay* oled = new OledDisplay;""")
rep("""			b->box.size = mm2px(Vec(6.4f, 6.4f));""", """			b->box.size = mm2px(Vec(6.8f, 6.8f));""")
rep("""			addInput(createInputCentered<fms::Jack>(mm2px(Vec(COLS[i], ROW_TR)), module, Felony::TR_INPUT + i));
			addInput(createInputCentered<fms::Jack>(mm2px(Vec(COLS[i], ROW_CV)), module, Felony::CV_INPUT + i));
			addOutput(createOutputCentered<fms::Jack>(mm2px(Vec(COLS[i], ROW_OUT)), module, Felony::OUT_OUTPUT + i));""",
    """			addInput(createInputCentered<fms::Jack>(mm2px(Vec(COLS[i], GROUP_TOP[0] + JACK_DY)), module, Felony::TR_INPUT + i));
			addInput(createInputCentered<fms::Jack>(mm2px(Vec(COLS[i], GROUP_TOP[1] + JACK_DY)), module, Felony::CV_INPUT + i));
			addOutput(createOutputCentered<fms::Jack>(mm2px(Vec(COLS[i], GROUP_TOP[2] + JACK_DY)), module, Felony::OUT_OUTPUT + i));""")

open(p, 'w', encoding='utf-8', newline='\n').write(s)
print('applied')
