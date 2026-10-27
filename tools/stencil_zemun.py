import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stencil_kit import Src, colors_block

f = Src(r'C:\Users\beto-\www\Belgrad\src\Zemun.cpp')
f.rep('#include "Controls4ms.hpp"', '#include "Controls4ms.hpp"\n#include "Stencil.hpp"')
f.block('// ---------------------------------------------------------------- themes', 'static std::shared_ptr<window::Font> pluginFont',
        '// ---------------------------------------------------------------- themes\n'
        '// betobullet stencil (docs/art/STYLE.md): theme 0 = black panel, 1 = white panel\n'
        'using bb::THEME_NAMES;\nstatic const int NUM_THEMES = 2;\n\n' + colors_block('Zemun'))
f.block('/** The logotype, one SVG per theme (bone / slate). */', 'struct ZemunWidget', r'''/** Panel background, frames, scales and every label. Drawn in code so the theme can change. */
struct PanelArt : Widget {
	Zemun* module = nullptr;

	void text(NVGcontext* vg, Vec mm, const char* s) {
		nvgText(vg, mm2px(mm.x), mm2px(mm.y), s, NULL);
	}

	void draw(const DrawArgs& args) override {
		bb::Ink k = bb::ink(module ? module->theme : 0);
		bb::Ink inv = {k.fg, k.bg};
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, k.bg);
		nvgFill(vg);

		// the frequency knob in brackets, the jack field framed, the output on a solid plate
		bb::bracketFrame(vg, Rect(FREQ_POS.minus(Vec(FREQ_R + 7.6f, FREQ_R + 7.6f)), Vec(2 * FREQ_R + 15.2f, 2 * FREQ_R + 19.4f)), k, 3.0f);
		bb::notchedFrame(vg, Rect(Vec(2.4f, 95.2f), Vec(W - 4.8f, 26.6f)), k);
		bb::chamferPath(vg, Rect(Vec(42.2f, 107.6f), Vec(14.0f, 13.4f)), 1.8f);
		nvgFillColor(vg, k.fg);
		nvgFill(vg);
		bb::bullets(vg, 19.0f, 52.1f, 93.6f, 2.0f, 9, k);

		// position dots of the mode switch
		for (int i = 0; i < zemun::NUM_MODES; i++) {
			Vec d = polar(MODE_POS, MIN_ANGLE + i * (MAX_ANGLE - MIN_ANGLE) / 9.f, MODE_R + 2.0f);
			nvgBeginPath(vg);
			nvgRect(vg, mm2px(d.x - 0.45f), mm2px(d.y - 0.45f), mm2px(0.9f), mm2px(0.9f));
			nvgFillColor(vg, k.fg);
			nvgFill(vg);
		}

		// slider scales: ticks and 0..10
		for (int s = 0; s < 2; s++) {
			float x = s == 0 ? FM_SLIDER_X : SPAN_SLIDER_X;
			float side = s == 0 ? -1.f : 1.f;
			for (int i = 0; i <= 10; i++) {
				float y = SLIDER_BOTTOM - 2.4f - (SLIDER_BOTTOM - SLIDER_TOP - 4.8f) * i / 10.f;
				nvgBeginPath(vg);
				nvgMoveTo(vg, mm2px(x + side * 3.4f), mm2px(y));
				nvgLineTo(vg, mm2px(x + side * (i % 2 == 0 ? 4.8f : 4.0f)), mm2px(y));
				nvgStrokeColor(vg, k.fg);
				nvgStrokeWidth(vg, mm2px(i % 5 == 0 ? 0.45f : 0.28f));
				nvgStroke(vg);
			}
		}

		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		std::shared_ptr<window::Font> reg = pluginFont("SpaceMono-Regular.ttf");
		std::shared_ptr<window::Font> bold = pluginFont("SpaceMono-Bold.ttf");
		if (!stencil || !reg || !bold)
			return;
		bb::title(vg, Rect(Vec(14.0f, 1.4f), Vec(W - 28.0f, 6.4f)), "ZEMUN", stencil->handle, k, 4.8f);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFillColor(vg, k.fg);

		// slider numbers and the numbers around the frequency knob
		nvgFontFaceId(vg, reg->handle);
		nvgFontSize(vg, mm2px(2.2f));
		for (int s = 0; s < 2; s++) {
			float x = s == 0 ? FM_SLIDER_X - 6.4f : SPAN_SLIDER_X + 6.4f;
			for (int i = 0; i <= 10; i += 2) {
				float y = SLIDER_BOTTOM - 2.4f - (SLIDER_BOTTOM - SLIDER_TOP - 4.8f) * i / 10.f;
				text(vg, Vec(x, y), string::f("%d", i).c_str());
			}
		}
		nvgFontSize(vg, mm2px(2.4f));
		for (int i = 0; i <= 10; i++) {
			Vec p = polar(FREQ_POS, MIN_ANGLE + i * (MAX_ANGLE - MIN_ANGLE) / 10.f, FREQ_R + 5.2f);
			text(vg, p, string::f("%d", i).c_str());
		}

		// mode names
		nvgFontFaceId(vg, bold->handle);
		nvgFontSize(vg, mm2px(2.6f));
		for (int i = 0; i < zemun::NUM_MODES; i++) {
			Vec p = polar(MODE_POS, MIN_ANGLE + i * (MAX_ANGLE - MIN_ANGLE) / 9.f, MODE_R + 5.6f);
			text(vg, p, zemun::MODE_NAMES[i]);
		}

		// knob and jack labels on stencil plates
		nvgFontFaceId(vg, stencil->handle);
		nvgFontSize(vg, mm2px(2.4f));
		nvgTextLetterSpacing(vg, mm2px(0.08f));
		bb::plate(vg, Vec(LEVEL_POS.x, 31.0f), "LEVEL", k, 11.0f);
		bb::plate(vg, Vec(SPAN_POS.x, 31.0f), "SPAN", k, 11.0f);
		bb::plate(vg, Vec(RESO_POS.x, 68.0f), "RESO", k, 11.0f);
		bb::plate(vg, Vec(BAL_POS.x, 68.0f), "BALANCE", k, 11.0f);
		bb::plate(vg, Vec(RESO_IN.x, RESO_IN.y + 5.2f), "RESO", k, 9.0f, 2.8f);
		bb::plate(vg, Vec(VOCT_IN.x, VOCT_IN.y + 5.2f), "V/OCT", k, 9.0f, 2.8f);
		bb::plate(vg, Vec(BAL_IN.x, BAL_IN.y + 5.2f), "BAL", k, 9.0f, 2.8f);
		bb::plate(vg, Vec(FM_IN.x, FM_IN.y + 5.2f), "FM", k, 9.0f, 2.8f);
		bb::plate(vg, Vec(AUDIO_IN.x, AUDIO_IN.y + 5.2f), "INPUT", k, 9.0f, 2.8f);
		bb::plate(vg, Vec(SPAN_IN.x, SPAN_IN.y + 5.2f), "SPAN", k, 9.0f, 2.8f);
		bb::plate(vg, Vec(OUT_POS.x, OUT_POS.y + 5.2f), "OUTPUT", inv, 11.0f, 2.8f);
		bb::plate(vg, Vec(TITO_POS.x, 38.0f), "TITO", k, 8.0f, 2.8f);
		bb::plate(vg, Vec(FREQ_POS.x, 56.6f), "FREQ", k, 12.0f, 3.4f);
		bb::plate(vg, Vec(MODE_POS.x, 87.4f), "MODE", k, 12.0f, 3.4f);
		nvgTextLetterSpacing(vg, 0.f);

		nvgFontFaceId(vg, bold->handle);
		nvgFontSize(vg, mm2px(2.4f));
		nvgFillColor(vg, k.fg);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		text(vg, Vec(TITO_POS.x + 4.4f, TITO_POS.y - 2.6f), "SM");
		text(vg, Vec(TITO_POS.x + 4.4f, TITO_POS.y + 2.6f), "XM");
	}
};

''')
f.rep('''		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(26.0f);
		logo->box.pos = Vec((box.size.x - logo->box.size.x) / 2, mm2px(3.0f));
		addChild(logo);
''', '')
f.save()
