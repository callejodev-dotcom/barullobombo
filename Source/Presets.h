// Presets.h — Os presets de fábrica de Barullo Bombo, por grupos. O primeiro é o de partida (os
// parámetros saen del). Cando un preset segue a nota, a afinación non conta (vai a nota).
#pragma once
#include "Bombo.h"

namespace bombo {

struct Preset
{
    const char* nome;
    const char* grupo;
    const char* descricion;
    int motor;
    double afinacion, caida, golpe, caracter, barrido, tempoBarrido, tonGolpe, sub, velocidade, maza, abafado, casco, sala;
    bool seguir;
};

inline const Preset PRESETS[] = {
    //                                                                           motor      afin  caída golpe car  barr  tBarr tonG  sub  vel  maza abaf casco sala  nota
    { "Catro no chan", "House e techno", "O bombo do house de sempre (estilo 909): golpe claro e corpo redondo e curto.",
      M909,      52,  420, 0.55, 0.20, 26,  22, 3800, 0.00, 0.5, 0.5, 0.3, 0.3, 0.00, false },
    { "Almacén", "House e techno", "Techno de nave industrial: máis saturado e cun chisco de sala, para que retumbe.",
      M909,      48,  700, 0.45, 0.55, 30,  28, 3000, 0.20, 0.5, 0.5, 0.3, 0.3, 0.25, false },
    { "Rumor", "House e techno", "A cola longa do techno de rumble: para mandala a unha reverb e apertala co sidechain.",
      SINTETICO, 45, 1200, 0.30, 0.45, 20,  35, 2500, 0.30, 0.4, 0.5, 0.3, 0.3, 0.45, false },
    { "Minimal", "House e techno", "Pequeno, seco e preciso: un punto no chan.",
      SINTETICO, 60,  220, 0.60, 0.10, 18,  12, 6000, 0.00, 0.5, 0.5, 0.3, 0.3, 0.00, false },
    { "Profundo", "House e techno", "Deep house: suave e redondo, cun sub por debaixo.",
      SINTETICO, 42,  650, 0.25, 0.15, 14,  40, 2000, 0.25, 0.5, 0.5, 0.3, 0.3, 0.05, false },

    { "Boom longo", "Hip hop e trap", "O 808 do trap: longo e tocando notas coma un baixo (cada nota soa no seu ton).",
      M808,      44, 2600, 0.25, 0.35,  5,  30, 3000, 0.00, 0.3, 0.5, 0.3, 0.3, 0.00, true },
    { "Rebumbio", "Hip hop e trap", "Un 808 saturado ata rachar: óese tamén nos altofalantes pequenos.",
      M808,      44, 2000, 0.35, 0.80,  7,  25, 3500, 0.00, 0.3, 0.5, 0.3, 0.3, 0.00, true },
    { "Drill", "Hip hop e trap", "Un 808 cun barrido máis marcado: entra deslizando dende arriba.",
      M808,      44, 1500, 0.50, 0.40, 12,  15, 4000, 0.00, 0.3, 0.5, 0.3, 0.3, 0.00, true },
    { "Clásico", "Hip hop e trap", "A caixa de ritmos de 1980 (estilo 808): suave, redondo e sen présa.",
      M808,      52,  900, 0.20, 0.05,  4,  20, 2500, 0.00, 0.5, 0.5, 0.3, 0.3, 0.00, false },
    { "Boom bap", "Hip hop e trap", "O bombo dun disco de vinilo dos 90: acústico, abafado e sucio.",
      ACUSTICO,  58,  350, 0.60, 0.45,  3,  50, 2500, 0.25, 0.5, 0.4, 0.6, 0.2, 0.15, false },

    { "Rock aberto", "Rock e acústicos", "Un bombo de rock sen almofada: resoa, co parche de diante e o casco, nun cuarto.",
      ACUSTICO,  56,  900, 0.50, 0.15,  3,  60, 3500, 0.00, 0.6, 0.55, 0.20, 0.60, 0.35, false },
    { "Rock abafado", "Rock e acústicos", "Cunha almofada dentro: curto, con pegada e o golpe da maza ben claro.",
      ACUSTICO,  60,  380, 0.65, 0.20, 2.5, 45, 4500, 0.15, 0.6, 0.80, 0.75, 0.25, 0.20, false },
    { "Metal", "Rock e acústicos", "Maza de plástico e parche abafado: un clic que atravesa o dobre pedal.",
      ACUSTICO,  64,  260, 0.90, 0.25,  2,  30, 6500, 0.20, 0.3, 1.00, 0.90, 0.10, 0.05, false },
    { "Jazz", "Rock e acústicos", "Un bombo pequeno afinado alto, con maza de feltro: soa a madeira e a cuarto.",
      ACUSTICO,  78,  700, 0.25, 0.00, 1.5, 70, 2500, 0.00, 0.7, 0.10, 0.10, 0.50, 0.40, false },
    { "No cuarto", "Rock e acústicos", "Un bombo gravado de lonxe, cos micrófonos do cuarto abertos.",
      ACUSTICO,  54,  550, 0.40, 0.10,  3,  50, 3000, 0.10, 0.6, 0.40, 0.45, 0.40, 0.60, false },
    { "Funk", "Rock e acústicos", "Seco e redondo: o bombo do funk e do soul.",
      ACUSTICO,  62,  300, 0.50, 0.20,  2,  35, 4000, 0.05, 0.6, 0.60, 0.70, 0.20, 0.10, false },

    { "Drum and bass", "Electrónica", "Rápido e con moita pegada: corta por riba dos breaks.",
      SINTETICO, 58,  300, 0.70, 0.35, 30,  10, 5000, 0.00, 0.5, 0.5, 0.3, 0.3, 0.05, false },
    { "Hardstyle", "Electrónica", "Saturación a tope: o bombo é a melodía (cada nota, no seu ton).",
      SINTETICO, 50,  600, 0.60, 0.95, 36,  18, 3500, 0.00, 0.4, 0.5, 0.3, 0.3, 0.10, true },
    { "Electro", "Electrónica", "Electro dos 80 (estilo 808): curto e cun pouco de barrido.",
      M808,      58,  450, 0.45, 0.25, 10,  15, 4000, 0.00, 0.5, 0.5, 0.3, 0.3, 0.00, false },
    { "Lo-fi", "Electrónica", "Escuro e morno, coma gravado nunha cinta vella.",
      M808,      55,  500, 0.10, 0.30,  3,  25, 1200, 0.00, 0.5, 0.5, 0.3, 0.3, 0.20, false },
};

constexpr int NUM_PRESETS = (int) (sizeof (PRESETS) / sizeof (PRESETS[0]));

} // namespace bombo
