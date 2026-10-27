# FELONY — manual

**betobullet** · 14 HP · Ornament & Crime no VCV Rack

O Felony roda o firmware original do Ornament & Crime (mxmxmx/O_C, MIT) sobre um O&C virtual. Os
apps, os menus e a tela são os do hardware. Só os drivers foram trocados: tela, conversores,
entradas, encoders e memória.

---

## 1. Painel

| Seção | O que é |
|---|---|
| **Tela** | OLED de 128 × 64, igual à do módulo. |
| **LEFT / RIGHT** | Encoders com botão (clique e toque longo). |
| **UP / DOWN** | Botões de cima e de baixo. |
| **TR1–TR4** | Entradas de gate/trigger (disparam por volta de 1 V). |
| **CV1–CV4** | Entradas de CV, de −3,3 a +6,6 V. |
| **A–D** | Saídas de −3 a +6 V, com calibração exata. |

**Usando os encoders com o mouse:**
- **Clique**: aperta o encoder.
- **Arrastar ou rolar a rodinha**: gira. O giro acelera como no original; para passos precisos, gire devagar.
- **Segurar parado**: toque longo (1 segundo).

## 2. Escolhendo o app

1. **Segure o encoder RIGHT** por cerca de 1 s. Abre a lista de apps.
2. **Gire o RIGHT** até o app desejado.
3. **Clique** para entrar. Se **segurar**, ele entra e também salva como app padrão.

| App | Função |
|---|---|
| **CopierMaschine** | sample & hold e registrador de deslocamento quantizado (ASR) |
| **Harrington 1200** | transformações neo-riemannianas de tríades |
| **Automatonnetz** | sequenciador na grade Tonnetz |
| **Quantermain** | quantizador quádruplo, com modos de Turing Machine e sequências |
| **Meta-Q** | quantizador duplo com sequências de escalas |
| **Quadraturia** | LFO quádruplo (base Frames) |
| **Low-rents** | atrator de Lorenz/Rössler |
| **Piqued** | 4 geradores de envelope, com o monitor gráfico do envelope |
| **Sequins** | sequenciador duplo |
| **Dialectic Ping Pong** | bolas quicando (envelopes) |
| **Viznutcracker sweet** | bytebeats |
| **Acid Curds** | progressões de acordes |
| **References** | tensões de referência e afinador (TR4) |

O manual completo de cada app está em https://ornament-and-cri.me/user-manual-v1_3/ e vale
integralmente para o Felony.

## 3. Salvamento

- **Ao salvar o patch**, o Felony guarda o app atual e as configurações de **todos** os apps, como
  faz o toque longo na lista de apps. Ao reabrir o projeto, ele volta no mesmo app com os mesmos
  ajustes.
- O **estado momentâneo** não é salvo, como no hardware: posição de sequência, envelope em curso.

## 4. Menu de contexto

- **Screen colour**: White, Blue, Yellow ou Green.
- **Theme**: Black ou White.
- **Power cycle**: reinicia o firmware, como desligar e ligar o módulo.

## 5. Limitação

Só pode haver **um Felony por Rack**, porque o firmware guarda o estado em variáveis globais. Um
segundo módulo mostra um aviso na tela e fica inativo.
