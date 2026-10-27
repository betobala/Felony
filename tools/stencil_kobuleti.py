import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stencil_kit import Src, colors_block

f = Src(r'C:\Users\beto-\www\Batumi\src\Kobuleti.cpp')
f.rep('#include "Controls4ms.hpp"', '#include "Controls4ms.hpp"\n#include "Stencil.hpp"')
f.block('// ---------------------------------------------------------------- themes', 'static std::shared_ptr<window::Font> pluginFont',
        '''// ---------------------------------------------------------------- themes
// betobullet stencil (docs/art/STYLE.md): theme 0 = black panel, 1 = white panel
using bb::THEME_NAMES;
static const int NUM_THEMES = 2;
// LED colours: positive half white, negative half grey (the panel stays black and white)
static const NVGcolor LED_POSITIVE = nvgRGB(0xF4, 0xF4, 0xF2), LED_NEGATIVE = nvgRGB(0x8A, 0x8A, 0x88);

''' + colors_block('Kobuleti'))
f.block('/** The logotype, one SVG per theme (bone / slate). */', 'struct KobuletiWidget', r'''/** Panel background, stencil frames and every label. Drawn in code so the theme can change. */
struct PanelArt : Widget {
	Kobuleti* module = nullptr;

	void draw(const DrawArgs& args) override {
		bb::Ink k = bb::ink(module ? module->theme : 0);
		bb::Ink inv = {k.fg, k.bg};
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, k.bg);
		nvgFill(vg);

		// frames: select + modes, the two input rows; the outputs on a solid plate; the switches
		bb::bullets(vg, 4.0f, 46.8f, 16.4f, 2.4f, 10, k);
		bb::notchedFrame(vg, Rect(Vec(2.4f, 18.2f), Vec(46.0f, 12.4f)), k, 2.0f, 1.2f);
		bb::frame(vg, Rect(Vec(2.4f, 52.2f), Vec(46.0f, 26.8f)), k);
		const float X[4] = {8.2f, 19.6f, 31.0f, 42.4f};
		for (int i = 0; i < 3; i++) {
			bb::link(vg, X[i], X[i + 1], 58.2f, k, 3.6f);
			bb::link(vg, X[i], X[i + 1], 70.4f, k, 3.6f);
		}
		bb::chamferPath(vg, Rect(Vec(2.4f, 87.0f), Vec(46.0f, 26.4f)), 2.0f);
		nvgFillColor(vg, k.fg);
		nvgFill(vg);
		bb::hazard(vg, Rect(Vec(2.4f, 79.6f), Vec(46.0f, 1.4f)), k, 1.8f);

		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		std::shared_ptr<window::Font> bold = pluginFont("SpaceMono-Bold.ttf");
		if (!stencil || !bold)
			return;
		bb::title(vg, Rect(Vec(11.4f, 1.6f), Vec(36.4f, 6.4f)), "KOBULETI", stencil->handle, k, 4.0f);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

		// mode labels under the four LEDs
		nvgFontFaceId(vg, bold->handle);
		nvgFontSize(vg, mm2px(2.4f));
		nvgFillColor(vg, k.fg);
		static const char* const modes[4] = {"FREE", "QUAD", "PHASE", "DIV"};
		for (int i = 0; i < 4; i++)
			nvgText(vg, mm2px(19.0f + i * 7.8f), mm2px(27.3f), modes[i], NULL);

		nvgFontFaceId(vg, stencil->handle);
		nvgTextLetterSpacing(vg, mm2px(0.1f));
		// channel numbers on solid plates, under the knobs
		nvgFontSize(vg, mm2px(3.2f));
		for (int i = 0; i < 4; i++)
			bb::plate(vg, Vec(X[i], 48.5f), string::f("%d", i + 1).c_str(), k, 6.4f, 4.0f);

		nvgFontSize(vg, mm2px(2.8f));
		bb::plate(vg, Vec(25.4f, 64.0f), "CV", k, 9.0f);
		bb::plate(vg, Vec(25.4f, 75.6f), "RESET / SYNC", k, 9.0f);
		bb::plate(vg, Vec(25.4f, 97.8f), "SINE", inv, 9.0f);
		bb::plate(vg, Vec(25.4f, 109.4f), "ASSIGN", inv, 9.0f);

		// switches: title above, the two states on the right
		static const struct {
			float x;
			const char* title;
			const char* top;
			const char* bottom;
		} sw[3] = {{7.2f, "SYNC", "SYNC", "RESET"}, {22.4f, "WAVE 1", "ON", "OFF"}, {37.6f, "WAVE 2", "ON", "OFF"}};
		for (const auto& w : sw) {
			nvgFontFaceId(vg, stencil->handle);
			nvgFontSize(vg, mm2px(2.4f));
			bb::plate(vg, Vec(w.x + 2.6f, 115.6f), w.title, k, 10.0f, 2.8f);
			nvgFontFaceId(vg, bold->handle);
			nvgFontSize(vg, mm2px(2.3f));
			nvgFillColor(vg, k.fg);
			nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
			nvgText(vg, mm2px(w.x + 4.2f), mm2px(119.0f), w.top, NULL);
			nvgText(vg, mm2px(w.x + 4.2f), mm2px(122.2f), w.bottom, NULL);
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		}
		nvgTextLetterSpacing(vg, 0.f);
	}
};

''')
f.rep('''		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(27.0f);
		logo->box.pos = Vec((box.size.x - logo->box.size.x) / 2, mm2px(4.0f));
		addChild(logo);

''', '')
f.rep('''		KTheme k = kTheme(0);
''', '')
f.rep('''			led->addBaseColor(k.accent);
			addChild(led);
		}

		for''', '''			led->addBaseColor(LED_POSITIVE);
			addChild(led);
		}

		for''')
f.rep('''			// positive half in the accent colour, negative half in amber''', '''			// positive half white, negative half grey''')
f.rep('''			led->addBaseColor(k.accent);
			led->addBaseColor(k.negative);''', '''			led->addBaseColor(LED_POSITIVE);
			led->addBaseColor(LED_NEGATIVE);''')
f.save()
