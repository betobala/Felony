"""One-off: betobullet stencil identity on src/Felony.cpp."""
import os

p = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'src', 'Felony.cpp')
s = open(p, encoding='utf-8').read()


def rep(a, b):
    global s
    assert a in s, a[:70]
    s = s.replace(a, b, 1)


def block(start, end, new):
    global s
    i, j = s.index(start), s.index(end)
    s = s[:i] + new + s[j:]


rep('#include "Controls4ms.hpp"', '#include "Controls4ms.hpp"\n#include "Stencil.hpp"')

block('// ---------------------------------------------------------------- themes', 'static const char* const SCREEN_COLOR_NAMES', '''// ---------------------------------------------------------------- themes
// betobullet stencil (docs/art/STYLE.md): theme 0 = black panel, 1 = white panel
using bb::THEME_NAMES;

''')

block('namespace fms {', 'static std::shared_ptr<window::Font> pluginFont', '''namespace fms {
const Colors& colors(const engine::Module* m) {
	static const Colors black = bb::controlColors(0), white = bb::controlColors(1);
	int t = m ? static_cast<const Felony*>(m)->theme : 0;
	return t == 1 ? white : black;
}
}  // namespace fms

''')

# encoder pointer: a white bar like the concept
rep("""		float dx = std::sin(angle) * r * 0.62f, dy = -std::cos(angle) * r * 0.62f;
		fms::circle(vg, cx + dx, cy + dy, r * 0.075f, c.btnOn, c.btnOn, 0.f);""",
    """		nvgBeginPath(vg);
		nvgMoveTo(vg, cx + std::sin(angle) * r * 0.18f, cy - std::cos(angle) * r * 0.18f);
		nvgLineTo(vg, cx + std::sin(angle) * r * 0.7f, cy - std::cos(angle) * r * 0.7f);
		nvgStrokeColor(vg, c.pointer);
		nvgStrokeWidth(vg, std::max(1.5f, r * 0.11f));
		nvgLineCap(vg, NVG_BUTT);
		nvgStroke(vg);""")

block('/** The logotype, one SVG per theme. */', 'struct FelonyWidget', r'''/** Black panel, stencil frames, hazard band and the knocked-out label plates. */
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

''')

rep('''		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(4.6f);
		logo->box.pos = Vec((box.size.x - logo->box.size.x) / 2, mm2px(2.4f));
		addChild(logo);

''', '')

open(p, 'w', encoding='utf-8', newline='\n').write(s)
print('ok')
