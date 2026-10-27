import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stencil_kit import Src, colors_block

f = Src(r'C:\Users\beto-\www\Chainsaw\src\Bucksaw.cpp')
f.rep('#include "Controls4ms.hpp"', '#include "Controls4ms.hpp"\n#include "Stencil.hpp"')
f.block('// ---------------------------------------------------------------- themes', 'static std::shared_ptr<window::Font> pluginFont',
        '// ---------------------------------------------------------------- themes\n'
        '// betobullet stencil (docs/art/STYLE.md): theme 0 = black panel, 1 = white panel\n'
        'using bb::THEME_NAMES;\nstatic const int NUM_THEMES = 2;\n\n' + colors_block('Bucksaw'))
f.block('/** Panel background and every label, drawn in code so the theme can change. */', 'struct BucksawWidget', r'''/** Panel background and every label, drawn in code so the theme can change. */
struct PanelArt : Widget {
	Bucksaw* module = nullptr;

	void draw(const DrawArgs& args) override {
		bb::Ink k = bb::ink(module ? module->theme : 0);
		bb::Ink inv = {k.fg, k.bg};
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, k.bg);
		nvgFill(vg);

		// the encoder and the two knobs in notched frames
		bb::notchedFrame(vg, Rect(Vec(1.2f, 11.8f), Vec(W - 2.4f, 19.8f)), k, 2.0f, 1.2f);
		bb::notchedFrame(vg, Rect(Vec(1.2f, 31.0f), Vec(W - 2.4f, 30.4f)), k, 2.0f, 1.2f);
		bb::hazard(vg, Rect(Vec(1.2f, 62.0f), Vec(W - 2.4f, 1.6f)), k, 1.8f);
		// jack field: inputs framed, the two outputs on a solid plate
		bb::frame(vg, Rect(Vec(1.2f, ROW[0] - 4.4f), Vec(W - 2.4f, ROW[3] - ROW[0] + 11.6f)), k, 2.0f, 0.5f);
		bb::chamferPath(vg, Rect(Vec(RX - 4.5f, ROW[2] - 4.4f), Vec(9.0f, ROW[3] - ROW[2] + 10.8f)), 1.4f);
		nvgFillColor(vg, k.fg);
		nvgFill(vg);

		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		if (!stencil)
			return;
		bb::title(vg, Rect(Vec(1.4f, 5.4f), Vec(W - 2.8f, 5.2f)), "BUCKSAW", stencil->handle, k, 2.7f);
		nvgFontFaceId(vg, stencil->handle);
		nvgTextLetterSpacing(vg, mm2px(0.06f));
		nvgFontSize(vg, mm2px(2.0f));
		bb::plate(vg, Vec(NOTE_LED.x, NOTE_LED.y + 2.9f), "NOTE", k, 6.4f, 2.4f);
		bb::plate(vg, Vec(ALT_LED.x, ALT_LED.y + 2.9f), "ALT", k, 6.4f, 2.4f);
		nvgFontSize(vg, mm2px(2.3f));
		bb::plate(vg, Vec(CX, DETUNE_POS.y + KNOB_R + 3.0f), "DETUNE", k, 13.0f, 2.8f);
		bb::plate(vg, Vec(CX, MORPH_POS.y + KNOB_R + 3.0f), "MORPH", k, 13.0f, 2.8f);
		const char* left[4] = {"DTN", "V/O 1", "V/O 2", "V/O 3"};
		const char* right[4] = {"MRPH", "FM", "OUT L", "OUT R"};
		nvgFontSize(vg, mm2px(1.9f));
		for (int i = 0; i < 4; i++) {
			bb::plate(vg, Vec(LX, ROW[i] + 5.2f), left[i], k, 7.6f, 2.4f);
			bb::plate(vg, Vec(RX, ROW[i] + 5.2f), right[i], i >= 2 ? inv : k, 7.6f, 2.4f);
		}
		nvgTextLetterSpacing(vg, 0.f);
	}
};

''')
f.save()
