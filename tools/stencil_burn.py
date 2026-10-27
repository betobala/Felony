import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stencil_kit import Src, colors_block

f = Src(r'C:\Users\beto-\www\DeltaV\src\Burn.cpp')
f.rep('#include "Controls4ms.hpp"', '#include "Controls4ms.hpp"\n#include "Stencil.hpp"')
f.block('// ---------------------------------------------------------------- themes', 'static std::shared_ptr<window::Font> pluginFont',
        '// ---------------------------------------------------------------- themes\n'
        '// betobullet stencil (docs/art/STYLE.md): theme 0 = black panel, 1 = white panel\n'
        'using bb::THEME_NAMES;\n\n' + colors_block('Burn'))
f.block('/** The logotype, one SVG per theme. */', 'struct BurnWidget', r'''struct PanelArt : Widget {
	Burn* module = nullptr;

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

		// attack and decay of each channel in a notched frame
		bb::notchedFrame(vg, Rect(Vec(1.4f, 9.8f), Vec(CX - 2.2f, 37.6f)), k);
		bb::notchedFrame(vg, Rect(Vec(CX + 0.8f, 9.8f), Vec(CX - 2.2f, 37.6f)), k);
		// a hazard strip splits the two channels in the shape row
		bb::hazard(vg, Rect(Vec(CX - 1.1f, 49.0f), Vec(2.2f, 15.4f)), k, 1.8f);
		bb::frame(vg, Rect(Vec(1.6f, 65.6f), Vec(W - 3.2f, 13.0f)), k);
		for (int r = 0; r < 2; r++) {
			float y = r == 0 ? ROW_A : ROW_B;
			bb::frame(vg, Rect(Vec(1.6f, y - 4.6f), Vec(W - 3.2f, 13.0f)), k);
			for (int j = 0; j < 4; j++)
				bb::link(vg, JX[j], JX[j + 1], y, k, 3.6f);
		}
		bb::chamferPath(vg, Rect(Vec(1.6f, ROW_C - 4.6f), Vec(W - 3.2f, 13.0f)), 2.0f);
		nvgFillColor(vg, k.fg);
		nvgFill(vg);

		// the shape scale: an arc with three dots (exp, lin, log) over each shape knob
		for (int i = 0; i < 2; i++) {
			float cx = mm2px(SX[i]), cy = mm2px(Y_SHAPE), r = mm2px(SHAPE_R + 2.2f);
			nvgBeginPath(vg);
			nvgArc(vg, cx, cy, r, -M_PI / 2 - 1.0f, -M_PI / 2 + 1.0f, NVG_CW);
			nvgStrokeColor(vg, k.fg);
			nvgStrokeWidth(vg, mm2px(0.35f));
			nvgStroke(vg);
			for (int d = -1; d <= 1; d++) {
				float a = -M_PI / 2 + d * 1.0f;
				nvgBeginPath(vg);
				nvgCircle(vg, cx + std::cos(a) * r, cy + std::sin(a) * r, mm2px(0.6f));
				nvgFillColor(vg, k.fg);
				nvgFill(vg);
			}
		}

		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		std::shared_ptr<window::Font> reg = pluginFont("SpaceMono-Regular.ttf");
		if (!stencil || !reg)
			return;
		bb::title(vg, Rect(Vec(12.4f, 1.4f), Vec(W - 24.8f, 6.0f)), "BURN", stencil->handle, k, 4.4f);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFontFaceId(vg, reg->handle);
		nvgFontSize(vg, mm2px(1.9f));
		nvgFillColor(vg, k.fg);
		for (int i = 0; i < 2; i++) {
			float r = SHAPE_R + 2.2f;
			text(vg, Vec(SX[i] - std::sin(1.0f) * r - 2.6f, Y_SHAPE - std::cos(1.0f) * r - 0.6f), "exp");
			text(vg, Vec(SX[i] + std::sin(1.0f) * r + 2.6f, Y_SHAPE - std::cos(1.0f) * r - 0.6f), "log");
		}

		nvgFontFaceId(vg, stencil->handle);
		nvgFontSize(vg, mm2px(2.2f));
		nvgTextLetterSpacing(vg, mm2px(0.08f));
		for (int i = 0; i < 2; i++) {
			bb::plate(vg, Vec(KX[i], Y_ATTACK + BIG_R + 2.4f), "ATTACK", k, 13.0f);
			bb::plate(vg, Vec(KX[i], Y_DECAY + BIG_R + 2.4f), "DECAY", k, 13.0f);
			bb::plate(vg, Vec(SX[i], Y_SHAPE + SHAPE_R + 2.4f), "SHAPE", k, 9.0f);
			bb::plate(vg, Vec(LFO_X[i], Y_SHAPE + 5.6f), "LFO", k, 6.6f);
			bb::plate(vg, Vec(KX[i], Y_TRIM + TRIM_R + 2.4f), string::f("LEVEL %d", i + 1).c_str(), k, 9.0f);
		}
		bb::plate(vg, Vec(CX, Y_TRIM + TRIM_R + 2.4f), "LINK", k, 7.0f);
		const char* rowA[5] = {"TRIG", "SLEW", "VCA 1", "TRIG", "SLEW"};
		const char* rowB[5] = {"ATK", "DEC", "VCA 2", "ATK", "DEC"};
		const char* rowC[5] = {"OUT 1", "EOC 1", "SUM", "OUT 2", "EOC 2"};
		for (int j = 0; j < 5; j++) {
			bb::plate(vg, Vec(JX[j], ROW_A + 5.0f), rowA[j], k, 7.6f, 2.8f);
			bb::plate(vg, Vec(JX[j], ROW_B + 5.0f), rowB[j], k, 7.6f, 2.8f);
			bb::plate(vg, Vec(JX[j], ROW_C + 5.0f), rowC[j], inv, 7.6f, 2.8f);
		}
		nvgTextLetterSpacing(vg, 0.f);
	}
};

''')
f.rep('''		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(17.0f);
		logo->box.pos = Vec((box.size.x - logo->box.size.x) / 2, mm2px(2.6f));
		addChild(logo);

''', '')
f.save()
