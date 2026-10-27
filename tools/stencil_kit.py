"""Helpers for applying the betobullet stencil identity to a plugin source file."""
import sys


class Src:
    def __init__(self, path):
        self.path = path
        self.s = open(path, encoding='utf-8').read()

    def rep(self, a, b):
        assert a in self.s, f'{self.path}: not found: {a[:70]!r}'
        self.s = self.s.replace(a, b, 1)

    def block(self, start, end, new):
        i = self.s.index(start)
        j = self.s.index(end, i)
        self.s = self.s[:i] + new + self.s[j:]

    def save(self):
        open(self.path, 'w', encoding='utf-8', newline='\n').write(self.s)
        print('ok', self.path)


def colors_block(module_type):
    return '''namespace fms {
const Colors& colors(const engine::Module* m) {
	static const Colors black = bb::controlColors(0), white = bb::controlColors(1);
	int t = m ? static_cast<const %s*>(m)->theme : 0;
	return t == 1 ? white : black;
}
}  // namespace fms

''' % module_type
