**Barullo Bombo** é un sintetizador de bombos: do house ao rock, pasando polo 808 e o 909. Para o día a día abondan un preset, o motor (**808**, **909**, **Sintético** ou **Acústico**) e catro mandos (afinación, caída, golpe e carácter). Se queres ir a máis, detrás de **MÁIS** están o barrido, o ton do golpe, o sub, a velocidade, o parche (maza, abafado e casco), a sala e o volume.

Sen mostras: cada golpe calcúlase ao momento. Unha ferramenta de [Escoita](https://www.escoita.link).

## Descargas

| Sistema | Instalador | Á man |
|---|---|---|
| **Mac** (Apple Silicon e Intel, macOS 11+): AU, VST3, CLAP e app | `BarulloBombo-…-mac.pkg` | `BarulloBombo-…-mac.zip` |
| **Windows** (64 bits): VST3, CLAP e app | `BarulloBombo-…-windows-setup.exe` | `BarulloBombo-…-windows-x64.zip` |
| **Linux** (x86_64): VST3, LV2, CLAP e app | — | `BarulloBombo-…-linux-x86_64.tar.gz` → `./instalar.sh` |

Se o sistema avisa ao abrilo:
- **Mac:** clic dereito sobre o instalador → *Abrir* → *Abrir*.
- **Windows:** «Windows protexeu o teu PC» → *Máis información* → *Executar de todos os xeitos*.

Avisan porque os instaladores non levan sinatura de pago, non porque haxa nada raro.

## Que ten

- **20 presets de fábrica**, en grupos: *House e techno* (Catro no chan, Almacén, Rumor, Minimal, Profundo), *Hip hop e trap* (Boom longo, Rebumbio, Drill, Clásico, Boom bap), *Rock e acústicos* (Rock aberto, Rock abafado, Metal, Jazz, No cuarto, Funk) e *Electrónica* (Drum and bass, Hardstyle, Electro, Lo-fi). Son tamén os programas do DAW.
- **O motor**:
  - **808**: un filtro ressonante que vibra ao golpealo cun pulso, coma a caixa de ritmos de 1980. Redondo e suave; coa caída longa e seguindo a nota, o baixo do trap.
  - **909**: un oscilador triangular redondeado ata case unha senoide, cun barrido rápido e un golpe marcado. O bombo do house e do techno.
  - **Sintético**: unha senoide limpa, co barrido e a caída libres.
  - **Acústico**: un parche: os modos dunha membrana circular, a maza (feltro, madeira ou plástico), o abafado, o parche de diante e o casco.
- **Os catro mandos**: *Afinación* (25–160 Hz; na pantalla, a nota que lle corresponde), *Caída* (de 40 ms a 5 s), *Golpe* (o clic do ataque) e *Carácter* (saturación: redondea o pico e sobe a cola).
- **TOCAR**: un golpe para escoitalo sen teclado (o LED acéndese con cada golpe, tamén os do MIDI). Cambiar de preset ou de motor, ou soltar un mando, tamén toca.
- **SEGUIR NOTA**: cada nota MIDI soa no seu ton (La = 440 Hz), para tocar o bombo coma un baixo. Apagado, todas as notas soan na afinación.
- **MÁIS**:
  - *O corpo e o golpe*: barrido (canto máis alto empeza o ton, ata 48 semitonos), tempo do barrido, ton do golpe, sub (unha senoide limpa por debaixo) e velocidade (canto conta a forza da nota);
  - *O parche e a sala*: maza, abafado e casco (no Acústico), sala (un cuarto curto, en estéreo) e volume.
- **A pantalla**: o golpe co son de agora (a onda e o ton, que baixa co barrido), un cursor que corre cando soa, a afinación coa súa nota e o nivel de saída.
- **Cada golpe abafa o anterior** cun fundido de 4 ms, coma unha caixa de ritmos: sen clic e sen que se sumen.
- **Presets propios, A/B e desfacer** (do núcleo común): os presets van en `.bbpreset`, en Documentos/Barullo Bombo/Presets. Cada clic é un paso de desfacer.
- Un **limitador de seguridade** á saída: nunca pasa de −1 dBFS. Saída estéreo ou mono; sen latencia.

## Como se fixo

`Source/Bombo.h`, sen JUCE:

```
nota ─┬─ corpo (808, 909, Sintético ou Acústico, co barrido) ─┬─ × forza ─ carácter ─┬─ sala ─ saída
      ├─ sub (senoide na afinación) ──────────────────────────┘                      │
      └─ golpe (pulso e ruído polo paso banda) ─ × forza ─────────────────────────────┘
```

- **O barrido**: o ton empeza *barrido* semitonos por riba e baixa á afinación cunha curva exponencial (o *tempo do barrido* é a súa constante de tempo).
- **808**: un fasor que xira á frecuencia do corpo e cae no tempo da caída (un resonador complexo: se o ton se move co barrido, a amplitude non cambia), excitado cun pulso de 1 ms. Coma na máquina, o son nace do filtro, non dun oscilador.
- **909**: un oscilador triangular que pasa por un conformador senoidal (case unha senoide, cos harmónicos impares da triangular) e un chisco de pegada nos primeiros 15 ms.
- **Acústico**: doce modos dunha membrana circular (as proporcións dos ceros de Bessel, de 1 a 4,23) co peso de golpear preto do centro, excitados por un pulso que dura o que a maza está contra o parche (2,8 ms de feltro, 0,4 de plástico; menos canto máis forte). O abafado acurta todos e máis os altos; o casco engade o parche de diante (un 4,5 % máis alto, que bate co de atrás) e tres modos do casco. Un golpe máis forte estira máis a pel e sobe máis o ton.
- **O golpe**: un pulso curto e ruído que cae axiña, por un paso banda no *ton do golpe*. Cada motor ten o seu: o 808 suave, o 909 con ruído, o Sintético un clic seco e o Acústico o da maza.
- **O carácter**: unha tanh cada vez máis forte no corpo e no sub, normalizada para que o pico quede no seu sitio e suba a cola. O golpe súmase despois, limpo: saturado rachaba nun ruído que soaba a clip.
- **A sala**: catro liñas realimentadas cunha matriz de Hadamard (0,45 s), cos agudos que se apagan antes; o bombo queda no centro e a sala ábrese aos lados.

## Estrutura

| Ficheiro | Que é |
|---|---|
| `Source/Bombo.h` | O motor |
| `Source/Presets.h` | Os presets de fábrica |
| `Source/PluginProcessor.*` | O plugin: parámetros, presets, estado e o MIDI |
| `Source/PluginEditor.*` | A interface |
| `tools/RenderTest.cpp` | A proba |

Usa o núcleo común de Escoita ([escoita-nucleo](https://github.com/callejodev-dotcom/escoita-nucleo), v0.3.0): o texto en galego, o aviso das versións novas, os presets propios, o A/B e o desfacer (`escoita::Estado`), o limitador de seguridade (`escoita::Teito`), a CI e os instaladores.

## Probas

- **Ao abrir**: o primeiro preset, cos parámetros nos seus valores de partida.
- **Afinación**: cada motor soa no seu ton (45 e 90 Hz, ±1 %).
- **Caída**: cada motor baixa 40 dB nos dous terzos da caída (200 e 1500 ms).
- **Barrido**: empeza máis de unha oitava por riba e acaba na afinación.
- **Seguir a nota**: o Do1 a 65,4 Hz e o Do2 unha oitava por riba; sen seguir, as dúas na afinación.
- **Velocidade**, **golpe** (os agudos do ataque en cada motor) e **parche** (o abafado acurta, a maza de plástico ten máis agudos ca a de feltro e o casco alonga).
- **MIDI**: a nota empeza na súa mostra dentro do bloque.
- **Un golpe abafa o anterior** sen clic.
- **Bloques**: con bloques de 512 e de 37 mostras sae o mesmo, mostra a mostra, nos catro motores (e a súa pegada).
- **Os 20 presets** soan, sen valores non numéricos e sen chegar ao teito (o limitador non actúa con ningún).
- **Tocar** e **todo o son fóra**, **texto e valor de cada parámetro** (ida e volta, tamén o motor cos tiles mal lidos), **bypass**, **saída mono**, **estado**, **A/B**, **presets propios**, **desfacer**, **programas do DAW** e **limitador**.
- **Validación**: pluginval ao nivel máximo (10), auval no Mac e clap-validator.

Na CI van en Mac, Windows e Linux, e tamén con AddressSanitizer e UBSan (`tools/probas-ci.sh`).

## Compilar

Nun Mac: `./build_mac.sh` (compila e instala no teu usuario). Á man:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBARULLO_TESTS=ON
cmake --build build --config Release
build/BBRenderTest_artefacts/Release/BBRenderTest
```

Outras ordes da proba: `--snapshot saida.png [mais]` (a interface), `--render saida.wav [preset]` (un preset tocando un ritmo) e `--todos cartafol` (cada preset no seu WAV).

## Publicar unha versión

```
git tag v0.1.0 && git push origin v0.1.0
```

A versión sae da liña `project(BarulloBombo VERSION …)` de `CMakeLists.txt`; o texto da versión, de `packaging/NOTAS-VERSION.md`.

## Créditos e licenzas

- **Barullo Bombo é software libre**, baixo a licenza **GNU Affero General Public License v3.0** (ficheiro `LICENSE`): podes usalo, estudalo, modificalo e compartilo; se distribúes unha versión modificada, ten que ir co seu código e coa mesma licenza. Usa JUCE 8 coa súa licenza AGPLv3.
- Tipografías Montserrat e Inter (licenza OFL, `Resources/Montserrat-OFL.txt` e `Resources/Inter-OFL.txt`).
- «808» e «909» só din a que soa cada motor: non hai nada copiado das máquinas (nin mostras nin o clic gravado do 909); todo se calcula.
