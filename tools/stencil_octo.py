import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stencil_kit import Src, colors_block

t = Src(r'C:\Users\beto-\www\OctoWorkout\src\Theme.hpp')
t.block('// Visual themes of the panel', 'struct Theme {', '''// Visual themes of the panel (see docs/art/STYLE.md): betobullet stencil, black and white only.
// "Black" (default) and "White" (inverse); the display and the LEDs are white in both.
#include "plugin.hpp"

static const int NUM_THEMES = 2;
static const char* const THEME_NAMES[NUM_THEMES] = {"Black", "White"};

static const NVGcolor ACCENT = nvgRGB(0xF4, 0xF4, 0xF2);
static const NVGcolor RUN_LED = nvgRGB(0xF4, 0xF4, 0xF2);
// outputs 1..8: white like everything else
static const NVGcolor LED_COLORS[8] = {
	ACCENT, ACCENT, ACCENT, ACCENT, ACCENT, ACCENT, ACCENT, ACCENT};

''')
t.block('inline Theme themeColors(int t) {', '\n}\n', '''inline Theme themeColors(int t) {
	Theme th;
	if (t == 1) {
		th.panel = nvgRGB(0xF1, 0xF1, 0xEE);
		th.ink = nvgRGB(0x0B, 0x0B, 0x0B);
		th.grey = nvgRGB(0x50, 0x50, 0x50);
		th.frame = nvgRGB(0x0B, 0x0B, 0x0B);
		th.button = nvgRGB(0x0B, 0x0B, 0x0B);
	}
	else {
		th.panel = nvgRGB(0x0B, 0x0B, 0x0B);
		th.ink = nvgRGB(0xF1, 0xF1, 0xEE);
		th.grey = nvgRGB(0xB4, 0xB4, 0xB2);
		th.frame = nvgRGB(0xF1, 0xF1, 0xEE);
		th.button = nvgRGB(0xF1, 0xF1, 0xEE);
	}
	th.screen = nvgRGB(0x00, 0x00, 0x00);
	th.head = nvgRGB(0xA0, 0xA0, 0xA0);
	th.accent = ACCENT;
	th.text = nvgRGB(0xF4, 0xF4, 0xF2);
	th.sub = nvgRGB(0xA0, 0xA0, 0xA0);
	return th;''')
t.save()

f = Src(r'C:\Users\beto-\www\OctoWorkout\src\OctoWorkout.cpp')
f.rep('#include "Controls4ms.hpp"', '#include "Controls4ms.hpp"\n#include "Stencil.hpp"')
f.block('namespace fms {', '// ---------- widgets ----------', colors_block('OctoWorkout'))
f.rep('''		nvgFillColor(args.vg, pressed ? nvgRGB(0xE0, 0x4E, 0x32) : ACCENT);''', '''		nvgFillColor(args.vg, pressed ? nvgRGB(0x9A, 0x9A, 0x98) : ACCENT);''')
f.rep('''		nvgStrokeColor(args.vg, nvgRGB(0xF6, 0xF4, 0xEF));
		nvgStrokeWidth(args.vg, mm2px(0.4f));
		nvgLineCap(args.vg, NVG_ROUND);''', '''		nvgStrokeColor(args.vg, nvgRGB(0x0B, 0x0B, 0x0B));
		nvgStrokeWidth(args.vg, mm2px(0.6f));
		nvgLineCap(args.vg, NVG_BUTT);''')
f.block('/** The logotype, an SVG per theme (paper / slate). */', 'struct OctoWorkoutWidget', r'''/** Panel background, stencil frames and labels. Drawn in code so the theme can change. */
struct PanelArt : Widget {
	OctoWorkout* module = nullptr;

	void draw(const DrawArgs& args) override {
		int theme = module ? module->theme : 0;
		bb::Ink k = bb::ink(theme);
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, k.bg);
		nvgFill(vg);

		// inputs, the display and the controls, the hazard band, the outputs
		bb::frame(vg, Rect(Vec(2.0f, 14.6f), Vec(36.64f, 20.6f)), k);
		for (int i = 0; i < 3; i++)
			bb::link(vg, 6.2f + 9.4f * i, 6.2f + 9.4f * (i + 1), 24.0f, k, 3.6f);
		bb::bracketFrame(vg, Rect(Vec(4.0f, 35.0f), Vec(32.64f, 23.0f)), k, 2.6f);
		bb::notchedFrame(vg, Rect(Vec(2.0f, 60.0f), Vec(36.64f, 21.2f)), k, 2.0f, 1.2f);
		bb::hazard(vg, Rect(Vec(2.0f, 82.2f), Vec(36.64f, 1.6f)), k, 1.8f);
		bb::notchedFrame(vg, Rect(Vec(2.0f, 85.0f), Vec(36.64f, 36.4f)), k, 2.0f, 1.2f);

		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		if (!stencil)
			return;
		bb::title(vg, Rect(Vec(3.0f, 5.4f), Vec(34.64f, 6.6f)), "OCTO WORKOUT", stencil->handle, k, 2.9f);
		nvgFontFaceId(vg, stencil->handle);
		nvgTextLetterSpacing(vg, mm2px(0.08f));

		// input labels (clk and run double as cv 3 and cv 4)
		nvgFontSize(vg, mm2px(2.4f));
		const char* ins[] = {"CLK", "RUN", "CV 1", "CV 2"};
		for (int i = 0; i < 4; i++)
			bb::plate(vg, Vec(6.2f + 9.4f * i, 29.6f), ins[i], k, 8.0f, 2.8f);
		nvgFontSize(vg, mm2px(2.0f));
		nvgFillColor(vg, k.fg);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, mm2px(6.2f), mm2px(32.9f), "CV 3", NULL);
		nvgText(vg, mm2px(15.6f), mm2px(32.9f), "CV 4", NULL);

		nvgFontSize(vg, mm2px(2.4f));
		bb::plate(vg, Vec(10.5f, 77.6f), "START/STOP", k, 15.0f, 2.8f);

		// output numbers on plates under each jack
		nvgFontSize(vg, mm2px(2.6f));
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			float x = 6.2f + 9.4f * (i % 4);
			float y = (i < 4) ? 101.6f : 116.8f;
			bb::plate(vg, Vec(x, y), string::f("%d", i + 1).c_str(), k, 6.4f, 2.8f);
		}
		nvgTextLetterSpacing(vg, 0.f);
	}
};

''')
f.rep('''		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(30.0f);
		logo->box.pos = Vec((box.size.x - logo->box.size.x) / 2, mm2px(5.6f));
		addChild(logo);

''', '')
f.save()
