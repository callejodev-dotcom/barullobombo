// Bombo.h — O motor de Barullo Bombo, sen JUCE. Catro xeitos de facer un bombo:
//   808        un filtro ressonante (coma a ponte en T da máquina) que se pon a vibrar cun pulso
//   909        un oscilador triangular redondeado ata case unha senoide, cun barrido rápido
//   Sintético  unha senoide limpa, co barrido e a caída libres
//   Acústico   un parche: os modos dunha membrana circular, a maza, o abafado, o parche de
//              diante e o casco
// Enriba, en todos: o golpe (o clic do ataque), o sub, o carácter (saturación) e unha sala curta.
// Cada golpe colle os axustes no momento de dar (coma nunha caixa de ritmos): mover un mando
// cambia o seguinte. O carácter, a sala e o volume, en cambio, móvense ao momento.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace bombo {

constexpr double PI = 3.14159265358979323846;
enum Motor { M808 = 0, M909, SINTETICO, ACUSTICO, NUM_MOTORES };

struct Axustes
{
    int motor = M909;
    double afinacion = 52;      // Hz: o ton do corpo cando acaba o barrido
    double caida = 420;         // ms: canto tarda o corpo en baixar 60 dB
    double golpe = 0.55;        // 0..1: o clic do ataque
    double caracter = 0.2;      // 0..1: saturación
    double barrido = 26;        // semitonos por riba da afinación ao empezar
    double tempoBarrido = 22;   // ms: canto tarda o barrido en baixar (constante de tempo)
    double tonGolpe = 3800;     // Hz: o brillo do golpe
    double sub = 0;             // 0..1: unha senoide limpa na afinación, por baixo
    double velocidade = 0.5;    // 0..1: canto conta a forza da nota
    double maza = 0.5;          // acústico: 0 feltro, 0,5 madeira, 1 plástico
    double abafado = 0.3;       // acústico: 0 parche aberto, 1 almofada contra o parche
    double casco = 0.3;         // acústico: o parche de diante e o casco
    double sala = 0;            // 0..1: un cuarto curto arredor
    bool seguir = false;        // cada nota MIDI soa no seu ton (e a afinación non conta)

    // igual, mando a mando (sen comparar reais con ==: os mesmos bits dan o mesmo)
    bool operator== (const Axustes& o) const
    {
        auto ig = [] (double x, double y) { return ! (x < y) && ! (y < x); };
        return motor == o.motor && ig (afinacion, o.afinacion) && ig (caida, o.caida) && ig (golpe, o.golpe) && ig (caracter, o.caracter)
            && ig (barrido, o.barrido) && ig (tempoBarrido, o.tempoBarrido) && ig (tonGolpe, o.tonGolpe) && ig (sub, o.sub)
            && ig (velocidade, o.velocidade) && ig (maza, o.maza) && ig (abafado, o.abafado) && ig (casco, o.casco) && ig (sala, o.sala)
            && seguir == o.seguir;
    }
    bool operator!= (const Axustes& o) const { return ! (*this == o); }
};

// o ton de base: a afinación ou, seguindo a nota, o da nota (La = 440 Hz)
inline double frecuenciaBase (const Axustes& a, int nota)
{
    return a.seguir ? 440.0 * std::pow (2.0, (nota - 69) / 12.0) : a.afinacion;
}
// no parche, un golpe máis forte estira máis a pel e sobe máis o ton
inline double barridoEfectivo (const Axustes& a, double forza)
{
    return a.motor == ACUSTICO ? a.barrido * (0.4 + 0.6 * forza) : a.barrido;
}
// o ton do corpo t segundos despois do golpe (para a pantalla)
inline double tonEn (const Axustes& a, double t, int nota = 36, double forza = 1.0)
{
    const double tp = std::max (0.0005, a.tempoBarrido / 1000.0);
    return frecuenciaBase (a, nota) * std::pow (2.0, barridoEfectivo (a, forza) * std::exp (-t / tp) / 12.0);
}

namespace detalle
{
    // ruído branco igual en todos os sistemas (xorshift)
    struct Aleatorio
    {
        uint32_t s = 0x9E3779B9u;
        double seguinte() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (double) s / 2147483648.0 - 1.0; }
    };

    // filtro de estado variable (TPT), paso banda coa ganancia 1 no centro
    struct PasoBanda
    {
        double k = 1, a1 = 0, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
        void axustar (double fc, double q, double sr)
        {
            const double g = std::tan (PI * std::min (fc, 0.45 * sr) / sr);
            k = 1.0 / q;
            a1 = 1.0 / (1.0 + g * (g + k)); a2 = g * a1; a3 = g * a2;
            ic1 = ic2 = 0;
        }
        double paso (double x)
        {
            const double v3 = x - ic2, v1 = a1 * ic1 + a2 * v3, v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2;
            return k * v1;
        }
    };

    // un modo: un fasor que xira á súa frecuencia e cae no seu tempo. Se a frecuencia se move
    // (o barrido), a amplitude non cambia. A entrada vai á parte real; sae a imaxinaria, que
    // empeza en cero (sen clic)
    struct Modo
    {
        double re = 0, im = 0, r = 1, w = 0, amp = 0, c = 1, s = 0;
        void iniciar (double w0, double caeCada, double a)
        {
            re = im = 0; w = w0; r = caeCada; amp = a;
            c = std::cos (w); s = std::sin (w);
        }
        double paso (double x, double mult)
        {
            double cc = c, ss = s;
            if (mult > 1.0 || mult < 1.0) { cc = std::cos (w * mult); ss = std::sin (w * mult); }
            const double nr = r * (re * cc - im * ss) + x;
            im = r * (re * ss + im * cc);
            re = nr;
            return im * amp;
        }
    };

    // un cuarto curto: catro liñas realimentadas cunha matriz de Hadamard e os agudos que se
    // apagan antes. Entra mono e sae estéreo
    class Sala
    {
    public:
        void preparar (double sr)
        {
            static const double ms[4] = { 12.7, 16.3, 19.9, 24.1 };
            constexpr double rt = 0.45;   // segundos ata −60 dB
            for (int i = 0; i < 4; ++i)
            {
                len[i] = std::max (1, (int) std::lround (ms[i] * 0.001 * sr));
                buf[i].assign ((size_t) len[i], 0.0);
                pos[i] = 0; lp[i] = 0;
                g[i] = std::pow (10.0, -3.0 * len[i] / (rt * sr));
            }
            kLp = 1.0 - std::exp (-2.0 * PI * 5000.0 / sr);
        }
        void limpar() { for (int i = 0; i < 4; ++i) { std::fill (buf[i].begin(), buf[i].end(), 0.0); lp[i] = 0; } }
        void procesar (double x, double& l, double& r)
        {
            double o[4];
            for (int i = 0; i < 4; ++i) { o[i] = buf[i][(size_t) pos[i]]; lp[i] += (o[i] - lp[i]) * kLp; }
            const double h[4] = { 0.5 * (lp[0] + lp[1] + lp[2] + lp[3]), 0.5 * (lp[0] - lp[1] + lp[2] - lp[3]),
                                  0.5 * (lp[0] + lp[1] - lp[2] - lp[3]), 0.5 * (lp[0] - lp[1] - lp[2] + lp[3]) };
            for (int i = 0; i < 4; ++i)
            {
                buf[i][(size_t) pos[i]] = x * 0.5 + h[i] * g[i];
                if (++pos[i] >= len[i]) pos[i] = 0;
            }
            l = 0.7 * (o[0] + o[2]); r = 0.7 * (o[1] + o[3]);
        }
    private:
        std::vector<double> buf[4];
        int len[4] {}, pos[4] {};
        double g[4] {}, lp[4] {}, kLp = 0.5;
    };
}

// un golpe: dura ata que se apaga (ou ata que o abafa o seguinte)
class Voz
{
public:
    void preparar (double frecuencia) { sr = frecuencia; activa_ = false; }
    bool activa() const { return activa_; }
    long idade() const { return n; }
    // o golpe seguinte corta este nun fundido de 4 ms (coma unha caixa de ritmos, sen clic)
    void abafar() { if (activa_ && fadePaso <= 0) fadePaso = 1.0 / (0.004 * sr); }

    void disparar (const Axustes& a, double forza, int nota, uint32_t semente)
    {
        motor = a.motor;
        n = 0; activa_ = true; fade = 1; fadePaso = 0;
        forza = std::clamp (forza, 0.0, 1.0);
        ganancia = 1.0 - a.velocidade * (1.0 - forza);
        f0 = std::clamp (frecuenciaBase (a, nota), 15.0, 0.1 * sr);
        // o barrido nunca pasa de 0,4 da frecuencia de mostraxe
        barr = std::min (barridoEfectivo (a, forza), 12.0 * std::log2 (0.4 * sr / f0));
        kBarr = std::exp (-1.0 / (std::max (0.0005, a.tempoBarrido / 1000.0) * sr)); eBarr = 1;
        const double tau = std::max (0.005, a.caida / 1000.0 / 6.907755);
        kAmp = std::exp (-1.0 / (tau * sr)); amp = 1;
        punch = 0; ePunch = 1; kPunch = std::exp (-1.0 / (0.015 * sr));
        fase = 0;
        double maisLonga = tau;
        numModos = 0;

        switch (motor)
        {
            case M808:
                nPulso = std::max (2, (int) std::lround (0.001 * sr));
                normPulso = PI / (2.0 * nPulso);
                modos[0].iniciar (2.0 * PI * f0 / sr, kAmp, 1.0);
                numModos = 1;
                break;
            case M909:
                punch = 0.5;
                break;
            case ACUSTICO:
            {
                // modos dunha membrana circular (ceros de Bessel) e canto soa cada un golpeando
                // preto do centro; a maza dura alí de 2,8 ms (feltro) a 0,4 ms (plástico)
                static const double R[] = { 1.0, 1.594, 2.136, 2.296, 2.653, 2.918, 3.156, 3.501, 3.600, 3.652, 4.060, 4.231 };
                static const double A[] = { 1.0, 0.35, 0.12, 0.45, 0.06, 0.20, 0.04, 0.08, 0.25, 0.03, 0.03, 0.05 };
                const double ab = a.abafado, mz = a.maza;
                nPulso = std::max (2, (int) std::lround (sr * (0.0028 - 0.0024 * mz) * (1.25 - 0.25 * forza)));
                normPulso = PI / (2.0 * nPulso);
                const double tau0 = tau * (1.0 - 0.75 * ab);
                auto engadir = [&] (double ratio, double t, double am)
                {
                    const double w = 2.0 * PI * f0 * ratio / sr;
                    if (w * std::pow (2.0, barr / 12.0) > 0.9 * PI || numModos >= (int) modos.size()) return;
                    modos[(size_t) numModos++].iniciar (w, std::exp (-1.0 / (std::max (0.002, t) * sr)), am);
                    maisLonga = std::max (maisLonga, t);
                };
                for (int k = 0; k < 12; ++k)
                    engadir (R[k], tau0 * std::pow (R[k], -(1.0 + 2.0 * ab)),
                             A[k] * (k == 0 ? 1.0 : 1.0 - 0.6 * ab) * std::pow (R[k], 0.8 * (mz - 0.5)));
                // o parche de diante (un pouco máis alto: bate co de atrás) e o casco
                const double cs = a.casco;
                if (cs > 0.001)
                {
                    engadir (1.045, tau * (0.6 + 0.8 * cs) * (1.0 - 0.5 * ab), 0.45 * cs);
                    const double tc = tau * (0.35 + 0.5 * cs) * (1.0 - 0.5 * ab);
                    engadir (3.4, tc, 0.10 * cs); engadir (5.1, tc * 0.8, 0.07 * cs); engadir (7.3, tc * 0.6, 0.05 * cs);
                }
                break;
            }
            default: break;
        }

        // o golpe: un pulso curto e ruído, polo paso banda no seu ton
        double durClic = 0.0004, tRuido = 0.003, centro = a.tonGolpe;
        ruido = 0.5; clicNivel = a.golpe;
        switch (motor)
        {
            case M808:      durClic = 0.0003; tRuido = 0.0015; ruido = 0.25; clicNivel *= 0.95; break;
            case M909:      break;
            case SINTETICO: durClic = 0.00025; tRuido = 0.0015; ruido = 0.15; break;
            case ACUSTICO:
                durClic = 0.0004 + 0.0012 * (1.0 - a.maza);
                tRuido = 0.001 + 0.003 * (1.0 - a.maza);
                ruido = 0.3 + 0.5 * a.maza;
                centro *= 0.35 + 0.8 * a.maza;
                clicNivel *= (0.42 + 0.78 * a.maza) * (0.6 + 0.4 * forza);
                break;
            default: break;
        }
        nClic = std::max (2, (int) std::lround (durClic * sr));
        kRuido = std::exp (-1.0 / (tRuido * sr)); eRuido = 1;
        bp.axustar (centro, 0.7, sr);
        rnd.s = semente | 1u;
        maisLonga = std::max (maisLonga, tRuido * 2.0);

        subNivel = a.sub * 0.6; subFase = 0; subAmp = 1; kSub = kAmp;
        vida = (long) (maisLonga * 11.5 * sr) + nClic + (long) (0.01 * sr);
    }

    // devolve o corpo e o sub; o golpe vai á parte (en «clic»), para que o carácter non o rache
    double procesar (double& clic)
    {
        clic = 0;
        if (! activa_) return 0.0;
        const double mult = barr > 0 && eBarr > 1.0e-9 ? std::exp (barr * eBarr * (0.69314718055994531 / 12.0)) : 1.0;
        eBarr *= kBarr;

        // o corpo
        double corpo = 0;
        switch (motor)
        {
            case M808:
            {
                const double x = n < nPulso ? std::sin (PI * ((double) n + 0.5) / nPulso) * normPulso : 0.0;
                corpo = modos[0].paso (x, mult);
                break;
            }
            case M909:
            {
                fase += f0 * mult / sr; fase -= std::floor (fase);
                const double tri = fase < 0.25 ? 4 * fase : fase < 0.75 ? 2 - 4 * fase : 4 * fase - 4;
                const double redondo = std::sin (0.5 * PI * tri);
                corpo = amp * (1.0 + punch * ePunch) / (1.0 + punch) * (0.8 * redondo + 0.2 * tri);
                break;
            }
            case SINTETICO:
                fase += f0 * mult / sr; fase -= std::floor (fase);
                corpo = amp * std::sin (2.0 * PI * fase);
                break;
            case ACUSTICO:
            {
                const double x = n < nPulso ? std::sin (PI * ((double) n + 0.5) / nPulso) * normPulso : 0.0;
                for (int k = 0; k < numModos; ++k) corpo += modos[(size_t) k].paso (x, mult);
                corpo *= 0.6;
                break;
            }
            default: break;
        }
        amp *= kAmp; ePunch *= kPunch;

        // o golpe
        double e = ruido * rnd.seguinte() * eRuido;
        if (n < nClic) e += (1.0 - ruido) * std::sin (PI * ((double) n + 0.5) / nClic);
        eRuido *= kRuido;
        clic = bp.paso (e) * clicNivel * 2.1 * ganancia;

        // o sub, cun fundido de 4 ms para que non tape o golpe
        double sb = 0;
        if (subNivel > 0)
        {
            subFase += f0 / sr; subFase -= std::floor (subFase);
            sb = subNivel * subAmp * std::sin (2.0 * PI * subFase) * std::min (1.0, (double) n / (0.004 * sr));
            subAmp *= kSub;
        }

        double y = (corpo + sb) * ganancia;
        if (fadePaso > 0)
        {
            fade -= fadePaso;
            if (fade <= 0) { activa_ = false; clic = 0; return 0.0; }
            y *= fade; clic *= fade;
        }
        if (++n >= vida) activa_ = false;
        return y;
    }

private:
    double sr = 48000;
    bool activa_ = false;
    int motor = M909;
    long n = 0, vida = 0;
    double f0 = 50, barr = 0, kBarr = 0, eBarr = 1;
    double amp = 1, kAmp = 1, punch = 0, ePunch = 1, kPunch = 1, fase = 0, ganancia = 1;
    int nPulso = 48; double normPulso = 1;
    std::array<detalle::Modo, 16> modos;
    int numModos = 0;
    int nClic = 2; double clicNivel = 0, ruido = 0, eRuido = 0, kRuido = 1;
    detalle::PasoBanda bp;
    detalle::Aleatorio rnd;
    double subNivel = 0, subFase = 0, subAmp = 1, kSub = 1;
    double fade = 1, fadePaso = 0;
};

// o instrumento: tres voces (cada golpe abafa o anterior), o carácter e a sala
class Bombo
{
public:
    static constexpr int VOCES = 3;

    void preparar (double frecuencia)
    {
        sr = frecuencia;
        for (auto& v : voces) v.preparar (sr);
        sala.preparar (sr);
        kSuave = 1.0 - std::exp (-1.0 / (0.01 * sr));
        driveS = ax.caracter; salaS = ax.sala;
        golpes = 0;
    }
    void axustar (const Axustes& a) { ax = a; }
    const Axustes& axustes() const { return ax; }

    void disparar (double forza, int nota)
    {
        Voz* libre = nullptr;
        for (auto& v : voces)
        {
            if (v.activa()) v.abafar();
            else if (libre == nullptr) libre = &v;
        }
        if (libre == nullptr)   // todas soando: a máis vella
        {
            libre = &voces[0];
            for (auto& v : voces) if (v.idade() > libre->idade()) libre = &v;
        }
        libre->disparar (ax, forza, nota, 0x2545F491u * ++golpes);
    }

    void silenciar()
    {
        for (auto& v : voces) v.preparar (sr);
        sala.limpar();
    }

    bool soando() const
    {
        for (auto& v : voces) if (v.activa()) return true;
        return false;
    }

    void procesar (double& l, double& r)
    {
        double s = 0, golpe = 0;
        for (auto& v : voces) if (v.activa()) { double c; s += v.procesar (c); golpe += c; }
        // o carácter: tanh cada vez máis forte no corpo (o pico queda no seu sitio e a cola sobe); o golpe
        // entra despois, limpo: saturado, rachaba nun ruído coma un clip
        driveS += (ax.caracter - driveS) * kSuave;
        if (driveS > 1.0e-5)
        {
            const double g = 1.0 + 8.0 * driveS * driveS;
            const double sat = std::tanh (g * s) / std::tanh (g);
            s += std::min (1.0, driveS * 4.0) * (sat - s);
        }
        s = (s + golpe) * 0.62;
        salaS += (ax.sala - salaS) * kSuave;
        double wl, wr;
        sala.procesar (s, wl, wr);
        // con sala, o seco baixa un pouco: a suma non pasa do pico de sen sala
        const double seco = s * (1.0 - 0.3 * salaS);
        l = seco + salaS * 0.5 * wl;
        r = seco + salaS * 0.5 * wr;
    }

private:
    Axustes ax;
    double sr = 48000, kSuave = 0.01, driveS = 0, salaS = 0;
    std::array<Voz, VOCES> voces;
    detalle::Sala sala;
    uint32_t golpes = 0;
};

} // namespace bombo
