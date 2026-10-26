# Felony — guia de estilo

Concept escolhido: **claro v2** (`docs/art/source/style_v1/felony_light__v2.png`), com o **escuro v2** como tema
Slate. Linguagem de controles do 4ms, como Octo, Harmonion, Kobuleti e Zemun.

## Paleta (`fTheme()` e `fms::colors()` em `src/Felony.cpp`)

| Token | Bone | Slate | Uso |
|---|---|---|---|
| `panel` | `#E8E6E1` | `#17191B` | fundo |
| `line` | `#0F1216` | `#6B6F75` | contorno dos grupos (0,3 mm) |
| `group` | `#EEECE7` | `#1B1D20` | fundo dos grupos arredondados |
| `pill` / `pillInk` | `#14161A` / `#F6F4EF` | `#D6D8D8` / `#17191B` | pílulas dos rótulos |
| `plate` | `#14161A` | `#0E1012` | caixa preta das saídas (pílulas claras) |
| `accent` | `#E8A317` | `#E8B33C` | **único acento**: ponto dos encoders, curva do logotipo |

## Logo

"felony" em **Syne Bold** seguido de uma curva de envelope (ataque e decaimento) em âmbar: a tela do Piqued num
traço. Gerado de contornos de fonte (`logotypes.py`), em `docs/art/logo/` e `res/logo_*.svg`; 4,6 mm de altura no topo.

## Layout (mm, 14 HP)

- Tela OLED 60 × 30 com moldura preta, y 9,6–39,6.
- Grupo dos encoders y 43,4–69,8: encoders de 8,2 mm de raio (serrilhados, ponto âmbar), pílulas LEFT/RIGHT;
  grupo central com UP e DOWN (capas pretas).
- Três faixas de 15,6 mm: TR1–4 (grupo), CV1–4 (grupo), saídas A–D (caixa preta). Colunas 11,4 / 27,5 / 43,6 / 59,7.
- Rótulos em pílulas, Space Mono Bold 2,5 mm, maiúsculas.
