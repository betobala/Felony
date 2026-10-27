import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stencil_kit import Src, colors_block

f = Src(r'C:\Users\beto-\www\Skiff\src\Skiff.cpp')
f.rep('#include "Controls4ms.hpp"', '#include "Controls4ms.hpp"\n#include "Stencil.hpp"')
f.block('// ---------------------------------------------------------------- themes', 'static std::shared_ptr<window::Font> pluginFont',
        '// ---------------------------------------------------------------- themes\n'
        '// betobullet stencil (docs/art/STYLE.md): theme 0 = black panel, 1 = white panel\n'
        'using bb::THEME_NAMES;\n\n' + colors_block('Skiff'))
f.block('struct PanelArt : Widget {', 'struct SkiffWidget', r'''struct PanelArt : Widget {
	Skiff* module = nullptr;

	void draw(const DrawArgs& args) override {
		bb::Ink k = bb::ink(module ? module->theme : 0);
		bb::Ink inv = {k.fg, k.bg};
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, k.bg);
		nvgFill(vg);

		for (int i = 0; i < 2; i++) {
			float top = CH_TOP[i];
			bb::notchedFrame(vg, Rect(Vec(1.2f, top), Vec(W - 2.4f, GROUP_H)), k, 2.2f, 1.4f);
			bb::chamferPath(vg, Rect(Vec(2.4f, top + PLATE_DY), Vec(W - 4.8f, PLATE_H)), 1.6f);
			nvgFillColor(vg, k.fg);
			nvgFill(vg);
		}
		// three cartridges between the channels
		bb::bullets(vg, CX - 4.0f, CX + 4.0f, CH_TOP[1] - 0.5f, 2.2f, 3, k);

		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		if (!stencil)
			return;
		bb::title(vg, Rect(Vec(1.6f, 5.6f), Vec(W - 3.2f, 6.0f)), "SKIFF", stencil->handle, k, 3.4f);
		nvgFontFaceId(vg, stencil->handle);
		nvgFontSize(vg, mm2px(2.2f));
		nvgTextLetterSpacing(vg, mm2px(0.08f));
		for (int i = 0; i < 2; i++) {
			float top = CH_TOP[i];
			bb::plate(vg, Vec(CX, top + KNOB_DY + KNOB_R + 2.4f), string::f("PROB %d", i + 1).c_str(), k, 12.0f);
			bb::plate(vg, Vec(JACK_X, top + CV_DY + 4.5f), "CV", k, 6.4f, 2.8f);
			bb::plate(vg, Vec(JACK_X, top + IN_DY + 4.5f), i == 0 ? "IN" : "IN 2", k, 6.4f, 2.8f);
			bb::plate(vg, Vec(JACK_X, top + OUT_DY + 4.1f), "OUT", inv, 6.4f, 2.8f);
		}
		nvgTextLetterSpacing(vg, 0.f);
	}
};

''')
f.rep('''		// the logo sits under the top screw
		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(12.4f);
		logo->box.pos = Vec((box.size.x - logo->box.size.x) / 2, mm2px(5.6f));
		addChild(logo);

''', '')
f.save()
