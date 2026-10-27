import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stencil_kit import Src

t = Src(r'C:\Users\beto-\www\Harmonion\src\Theme.hpp')
t.block('// Visual themes', '#include "plugin.hpp"', '''// Visual themes of the panel (see docs/art/STYLE.md): betobullet stencil, black and white only.
// "Black" (default) and "White" (inverse); the "accent" that marks everything that is on is white
// on the black panel and black on the white one.
''')
t.block('static const char* const THEME_NAMES', 'inline NVGcolor mixColor', '''static const char* const THEME_NAMES[NUM_THEMES] = {"Black", "White"};

inline const Theme& themeColors(int t) {
	static const Theme T[NUM_THEMES] = {
		// Black
		{nvgRGB(0x0B, 0x0B, 0x0B), nvgRGB(0x0B, 0x0B, 0x0B), nvgRGB(0xF1, 0xF1, 0xEE), nvgRGB(0xF1, 0xF1, 0xEE),
		 nvgRGB(0xB4, 0xB4, 0xB2), nvgRGB(0x00, 0x00, 0x00), nvgRGB(0x26, 0x26, 0x26), nvgRGB(0xF4, 0xF4, 0xF2),
		 nvgRGB(0x5E, 0x5E, 0x5C)},
		// White
		{nvgRGB(0xF1, 0xF1, 0xEE), nvgRGB(0xF1, 0xF1, 0xEE), nvgRGB(0x0B, 0x0B, 0x0B), nvgRGB(0x0B, 0x0B, 0x0B),
		 nvgRGB(0x50, 0x50, 0x50), nvgRGB(0x00, 0x00, 0x00), nvgRGB(0xDA, 0xDA, 0xD6), nvgRGB(0xF4, 0xF4, 0xF2),
		 nvgRGB(0x8A, 0x8A, 0x88)},
	};
	return T[t < 0 ? 0 : (t >= NUM_THEMES ? NUM_THEMES - 1 : t)];
}

''')
t.save()

w = Src(r'C:\Users\beto-\www\Harmonion\src\Widget.cpp')
w.rep('#include "Controls.hpp"', '#include "Controls.hpp"\n#include "Stencil.hpp"')
w.rep('''struct AmberLight : GrayModuleLightWidget {
	AmberLight() {
		addBaseColor(nvgRGB(0xff, 0xb3, 0x47));''', '''// (the name stays; in the betobullet identity the lights are white)
struct AmberLight : GrayModuleLightWidget {
	AmberLight() {
		addBaseColor(nvgRGB(0xf4, 0xf4, 0xf2));''')
w.block('/** The logotype, one SVG per theme (graphite / paper). */', '/** Panel background, section outlines and tabs.', '')
w.rep('''		nvgStrokeWidth(vg, mm2px(0.3f));
		for (const R& r : sections) {
			nvgBeginPath(vg);
			nvgRoundedRect(vg, mm2px(r.x), mm2px(r.y), mm2px(r.w), mm2px(r.h), mm2px(1.6f));
			nvgFillColor(vg, t.plate);
			nvgFill(vg);
			nvgStrokeColor(vg, t.outline);
			nvgStroke(vg);
		}''', '''		bb::Ink k = {t.panel, t.ink};
		for (const R& r : sections)
			bb::notchedFrame(vg, Rect(Vec(r.x, r.y), Vec(r.w, r.h)), k, 2.4f, 1.6f);''')
w.rep('''		nvgStrokeWidth(vg, mm2px(0.3f));
		for (const R& r : tabs) {
			nvgBeginPath(vg);
			nvgRoundedRect(vg, mm2px(r.x), mm2px(r.y), mm2px(r.w), mm2px(r.h), mm2px(1.0f));
			nvgFillColor(vg, t.plate);
			nvgFill(vg);
			nvgStrokeColor(vg, t.outline);
			nvgStroke(vg);
		}''', '''		for (const R& r : tabs) {
			bb::chamferPath(vg, Rect(Vec(r.x, r.y), Vec(r.w, r.h)), 1.0f);
			nvgFillColor(vg, t.panel);
			nvgFill(vg);
			nvgStrokeColor(vg, t.ink);
			nvgStrokeWidth(vg, mm2px(0.5f));
			nvgStroke(vg);
		}''')
w.rep('''		// display frame
		nvgBeginPath(vg);
		nvgRoundedRect(vg, mm2px(61.0f), mm2px(29.4f), mm2px(34.2f), mm2px(19.6f), mm2px(1.2f));
		nvgFillColor(vg, t.black);
		nvgFill(vg);
		nvgStrokeColor(vg, t.outline);
		nvgStroke(vg);''', '''		// display frame, with stencil brackets
		nvgBeginPath(vg);
		nvgRect(vg, mm2px(61.0f), mm2px(29.4f), mm2px(34.2f), mm2px(19.6f));
		nvgFillColor(vg, t.black);
		nvgFill(vg);
		bb::bracketFrame(vg, Rect(Vec(61.0f, 29.4f), Vec(34.2f, 19.6f)), k, 2.6f);

		// the name, in stencil type
		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		if (stencil) {
			nvgFontFaceId(vg, stencil->handle);
			nvgFontSize(vg, mm2px(4.2f));
			nvgTextLetterSpacing(vg, mm2px(0.3f));
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgFillColor(vg, t.ink);
			nvgText(vg, mm2px(78.7f), mm2px(2.9f), "HARMONION", NULL);
			nvgTextLetterSpacing(vg, 0.f);
		}''')
# section headers in stencil type
w.rep('''		std::shared_ptr<window::Font> font = pluginFont("MartianMono-Label.ttf");
		if (!font)
			return;
		nvgFontFaceId(args.vg, font->handle);''', '''		std::shared_ptr<window::Font> font = pluginFont("MartianMono-Label.ttf");
		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		if (!font || !stencil)
			return;
		nvgFontFaceId(args.vg, font->handle);''')
w.rep('''			nvgFillColor(args.vg, (l.header || l.size >= 2.4f) ? t.ink : t.grey);''', '''			nvgFillColor(args.vg, (l.header || l.size >= 2.4f) ? t.ink : t.grey);
			nvgFontFaceId(args.vg, l.header ? stencil->handle : font->handle);''')
w.rep('''		// Center column
		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(30.0f);
		logo->box.pos = Vec(mm2px(78.7f) - logo->box.size.x / 2, mm2px(3.2f) - logo->box.size.y / 2);
		addChild(logo);
''', '''		// Center column (the name is drawn by PanelArt)
''')
w.block('namespace ctl {\nconst Colors& colors', '}  // namespace ctl', '''namespace ctl {
const Colors& colors(const engine::Module* m) {
	static const Colors black = bb::controlColors(0), white = bb::controlColors(1);
	int t = m ? static_cast<const hm::Harmonion*>(m)->theme : 0;
	return t == 1 ? white : black;
}
''')
w.save()
