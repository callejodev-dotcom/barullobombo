// Proba sen interface: a afinación de cada motor, a caída, o barrido, seguir a nota, a velocidade,
// o golpe e o parche, o MIDI no seu sitio, o abafado dun golpe polo seguinte, todos os presets, o
// estado, o A/B, os presets propios e o desfacer, o limitador, o bypass e a independencia do
// tamaño de bloque; tamén capturas da interface.
//   BBRenderTest [saida.wav]                    proba enteira (devolve o número de fallos)
//   BBRenderTest --snapshot saida.png [mais]    a interface (con «mais», o panel Máis aberto)
//   BBRenderTest --render saida.wav [preset]    un preset tocando un ritmo
//   BBRenderTest --todos cartafol               cada preset no seu WAV
#include <juce_audio_formats/juce_audio_formats.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"
#include <cstdio>
#include <cstring>

using escoita::U;

static void writeWav (const juce::File& f, const juce::AudioBuffer<float>& b, double sr)
{
    f.deleteFile();
    juce::WavAudioFormat wav;
    if (auto os = std::unique_ptr<juce::OutputStream> (f.createOutputStream()))
        if (auto w = std::unique_ptr<juce::AudioFormatWriter> (wav.createWriterFor (os.get(), sr, (unsigned) b.getNumChannels(), 24, {}, 0)))
        {
            os.release();
            w->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
            std::printf ("WAV: %s\n", f.getFullPathName().toRawUTF8());
        }
}

struct Nota { double t; int nota; float forza; };

// toca as notas (no seu tempo, en mostras) polo plugin en bloques
static juce::AudioBuffer<float> tocar (BBProcessor& p, const std::vector<Nota>& notas, double len, int blockSize, double sr, bool preparar = true)
{
    if (preparar) { p.setPlayConfigDetails (0, 2, sr, blockSize); p.prepareToPlay (sr, blockSize); }
    const int total = (int) (len * sr);
    juce::AudioBuffer<float> out (2, total), buf (2, blockSize);
    out.clear();
    for (int pos = 0; pos < total; pos += blockSize)
    {
        const int n = juce::jmin (blockSize, total - pos);
        buf.setSize (2, n, false, false, true);
        juce::MidiBuffer midi;
        for (auto& e : notas)
        {
            const int k = (int) std::lround (e.t * sr);
            if (k >= pos && k < pos + n) midi.addEvent (juce::MidiMessage::noteOn (1, e.nota, e.forza), k - pos);
        }
        p.processBlock (buf, midi);
        for (int c = 0; c < 2; ++c) out.copyFrom (c, pos, buf, c, 0, n);
    }
    return out;
}

static void setParam (BBProcessor& p, const juce::String& id, float v)
{
    if (auto* prm = p.apvts.getParameter (id)) prm->setValueNotifyingHost (prm->convertTo0to1 (v));
}

// un bombo limpo: sen golpe, sen sub, sen sala nin carácter, sen barrido
static void limpo (BBProcessor& p, int motor, float afin = 50.0f, float caida = 1500.0f)
{
    setParam (p, "motor", (float) motor); setParam (p, "afinacion", afin); setParam (p, "caida", caida);
    setParam (p, "golpe", 0.0f); setParam (p, "caracter", 0.0f); setParam (p, "sub", 0.0f); setParam (p, "sala", 0.0f);
    setParam (p, "barrido", 0.0f); setParam (p, "seguir", 0.0f); setParam (p, "velocidade", 0.0f);
    setParam (p, "abafado", 0.0f); setParam (p, "casco", 0.0f);
}

static float pico (const juce::AudioBuffer<float>& b, int desde = 0, int ata = -1)
{
    if (ata < 0) ata = b.getNumSamples();
    float m = 0;
    for (int c = 0; c < b.getNumChannels(); ++c) m = juce::jmax (m, b.getMagnitude (c, desde, ata - desde));
    return m;
}
static bool finito (const juce::AudioBuffer<float>& b)
{
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = 0; i < b.getNumSamples(); ++i) if (! std::isfinite (b.getSample (c, i))) return false;
    return true;
}
static float diferenza (const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b)
{
    float d = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < juce::jmin (a.getNumSamples(), b.getNumSamples()); ++i) d = juce::jmax (d, std::abs (a.getSample (c, i) - b.getSample (c, i)));
    return d;
}
static double enerxia (const juce::AudioBuffer<float>& b, double sr, double t0, double t1)
{
    double e = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = (int) (t0 * sr); i < juce::jmin (b.getNumSamples(), (int) (t1 * sr)); ++i) e += (double) b.getSample (c, i) * b.getSample (c, i);
    return e;
}
// a frecuencia entre t0 e t1: os pasos por cero subindo, con interpolación
static double frecuencia (const juce::AudioBuffer<float>& b, double sr, double t0, double t1)
{
    double primeiro = -1, ultimo = -1; int cruces = 0;
    for (int i = juce::jmax (1, (int) (t0 * sr)); i < juce::jmin (b.getNumSamples(), (int) (t1 * sr)); ++i)
    {
        const float x0 = b.getSample (0, i - 1), x1 = b.getSample (0, i);
        if (x0 < 0 && x1 >= 0)
        {
            const double t = ((double) (i - 1) + (double) x0 / ((double) x0 - (double) x1)) / sr;
            if (primeiro < 0) primeiro = t;
            ultimo = t; ++cruces;
        }
    }
    return cruces < 2 ? 0.0 : (cruces - 1) / (ultimo - primeiro);
}
// os agudos (paso alto dun polo a 2 kHz) entre t0 e t1
static double agudos (const juce::AudioBuffer<float>& b, double sr, double t0, double t1)
{
    const double k = 1.0 - std::exp (-2 * bombo::PI * 2000.0 / sr);
    double z = 0, e = 0;
    for (int i = 0; i < juce::jmin (b.getNumSamples(), (int) (t1 * sr)); ++i)
    {
        z += (b.getSample (0, i) - z) * k;
        const double h = b.getSample (0, i) - z;
        if (i >= (int) (t0 * sr)) e += h * h;
    }
    return e;
}
// canto tarda o son en baixar 40 dB desde o pico do corpo (en xanelas de 5 ms)
static double tempoA40 (const juce::AudioBuffer<float>& b, double sr)
{
    const int w = (int) (0.005 * sr);
    double ref = 0; int onde = -1;
    for (int i = 0; i + w <= b.getNumSamples(); i += w)
    {
        const double e = enerxia (b, sr, i / sr, (i + w) / sr);
        if (e > ref) { ref = e; onde = -1; }
        else if (onde < 0 && e < ref * 1.0e-4) onde = i;
    }
    return onde < 0 ? -1.0 : onde / sr;
}

// a pegada dun son: FNV-1a de 64 bits das mostras (un cambio de organización non pode movela nin un bit)
static uint64_t pegada (const juce::AudioBuffer<float>& b)
{
    uint64_t h = 1469598103934665603ull;
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            const float x = b.getSample (c, i);
            uint32_t u; std::memcpy (&u, &x, 4);
            for (int k = 0; k < 4; ++k) { h ^= (u >> (8 * k)) & 0xFF; h *= 1099511628211ull; }
        }
    return h;
}

// un ritmo: catro negras a 120, coa primeira de cada compás máis forte
static std::vector<Nota> ritmo (double len)
{
    std::vector<Nota> v;
    for (double t = 0; t < len - 0.3; t += 0.5) v.push_back ({ t, 36, std::fmod (t, 2.0) < 0.01 ? 1.0f : 0.8f });
    return v;
}

static void snapshot (const juce::File& png, bool mais)
{
    BBProcessor p;
    const double sr = 48000;
    p.setPlayConfigDetails (0, 2, sr, 512);
    p.prepareToPlay (sr, 512);
    p.cargarPreset (mais ? 10 : 0);
    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    auto* ce = dynamic_cast<BBEditor*> (ed.get());
    ed->setSize (ui::MainView::W, ui::MainView::H);
    if (mais) ce->view().mostrarMais (true);
    p.tocar();
    juce::AudioBuffer<float> buf (2, 512);
    for (int blk = 0; blk < 12; ++blk)
    {
        juce::MidiBuffer m; p.processBlock (buf, m);
        ce->view().timerCallback();
        juce::Thread::sleep (10);
    }
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.5f);
    png.deleteFile();
    juce::FileOutputStream os (png);
    juce::PNGImageFormat().writeImageToStream (img, os);
    std::printf ("PNG: %s\n", png.getFullPathName().toRawUTF8());
}

int main (int argc, char** argv)
{
    std::setvbuf (stdout, nullptr, _IONBF, 0);
    juce::ScopedJuceInitialiser_GUI init;
    const double sr = 48000;
    const juce::String mode = argc > 1 ? juce::String (argv[1]) : juce::String();

    if (mode == "--snapshot" && argc > 2) { snapshot (juce::File (juce::String (argv[2])), argc > 3 && juce::String (argv[3]) == "mais"); return 0; }

    if (mode == "--render" && argc > 2)
    {
        BBProcessor p;
        p.cargarPreset (argc > 3 ? atoi (argv[3]) : 0);
        const double len = 4.0 + juce::jmin (5.0, p.getTailLengthSeconds());
        writeWav (juce::File (juce::String (argv[2])), tocar (p, ritmo (4.0), len, 512, sr), sr);
        return 0;
    }

    if (mode == "--todos" && argc > 2)
    {
        const juce::File dir { juce::String (argv[2]) };
        dir.createDirectory();
        for (int i = 0; i < bombo::NUM_PRESETS; ++i)
        {
            BBProcessor p;
            p.cargarPreset (i);
            // os que seguen a nota tocan unha liña de baixo; os outros, o ritmo
            const std::vector<Nota> n = bombo::PRESETS[i].seguir
                ? std::vector<Nota> { { 0.0, 29, 1.0f }, { 0.75, 29, 0.8f }, { 1.5, 32, 0.9f }, { 2.0, 27, 0.9f }, { 3.0, 24, 1.0f } }
                : ritmo (4.0);
            const double len = 4.0 + juce::jmin (3.0, p.getTailLengthSeconds());
            writeWav (dir.getChildFile (juce::String (i + 1).paddedLeft ('0', 2) + " " + U (bombo::PRESETS[i].nome) + ".wav"), tocar (p, n, len, 512, sr), sr);
        }
        return 0;
    }

    int fails = 0;
    auto check = [&fails] (bool ok, const char* what, const juce::String& extra = {})
    {
        std::printf ("%-52s %s %s\n", what, ok ? "OK" : "FALLO", extra.toRawUTF8());
        fails += ok ? 0 : 1;
    };
    const std::vector<Nota> un { { 0.0, 36, 1.0f } };

    {   // ao abrir: o primeiro preset, cos parámetros nos seus valores de partida (sen editar)
        BBProcessor p;
        juce::String mal;
        for (auto* prm : p.getParameters())
            if (auto* r = dynamic_cast<juce::RangedAudioParameter*> (prm))
                if (std::abs (r->getValue() - r->getDefaultValue()) > 1e-6f) mal << r->getParameterID() << " ";
        check (mal.isEmpty() && p.getPreset() == 0 && ! p.presetEditado() && p.nomePreset() == U (bombo::PRESETS[0].nome),
               "ao abrir: o primeiro preset, nos valores de partida", mal);
    }
    {   // a afinación: cada motor soa no seu ton (a 45 e a 90 Hz)
        juce::String que; bool ok = true;
        for (int m = 0; m < bombo::NUM_MOTORES; ++m)
            for (float f : { 45.0f, 90.0f })
            {
                BBProcessor p;
                limpo (p, m, f, 3000.0f);
                const auto o = tocar (p, un, 0.8, 512, sr);
                const double medida = frecuencia (o, sr, 0.15, 0.7);
                ok = ok && finito (o) && std::abs (medida / f - 1.0) < 0.01;
                que << juce::String (medida, 1) << " ";
            }
        check (ok, "afinación: cada motor no seu ton (45 e 90 Hz)", "(" + que.trim() + ")");
    }
    {   // a caída: o corpo baixa 40 dB nos 2/3 da caída (a caída son 60 dB), en cada motor
        juce::String que; bool ok = true;
        for (int m = 0; m < bombo::NUM_MOTORES; ++m)
            for (float c : { 200.0f, 1500.0f })
            {
                BBProcessor p;
                limpo (p, m, 55.0f, c);
                const auto o = tocar (p, un, c / 1000.0 * 1.4 + 0.1, 512, sr);
                const double t = tempoA40 (o, sr) * 1000.0, esperado = c * 40.0 / 60.0;
                ok = ok && t > esperado * 0.75 && t < esperado * 1.25;
                que << juce::String (juce::roundToInt (t)) << " ";
            }
        check (ok, "caída: cada motor no seu tempo (200 e 1500 ms)", "(" + que.trim() + " ms a -40 dB)");
    }
    {   // o barrido: empeza máis alto e baixa á afinación
        BBProcessor p;
        limpo (p, bombo::SINTETICO, 50.0f, 1500.0f);
        setParam (p, "barrido", 24.0f); setParam (p, "tempoBarrido", 30.0f);
        const auto o = tocar (p, un, 0.8, 512, sr);
        const double fIni = frecuencia (o, sr, 0.0, 0.02), fFin = frecuencia (o, sr, 0.3, 0.7);
        check (fIni > 2.0 * fFin && std::abs (fFin / 50.0 - 1.0) < 0.01, "barrido: empeza arriba e baixa á afinación",
               juce::String::formatted ("(%.0f Hz -> %.1f Hz)", fIni, fFin));
    }
    {   // seguir a nota: a nota 48 soa unha oitava por riba da 36; sen seguir, as dúas na afinación
        auto f = [&] (bool seguir, int nota)
        {
            BBProcessor p;
            limpo (p, bombo::M808, 50.0f, 3000.0f);
            setParam (p, "seguir", seguir ? 1.0f : 0.0f);
            return frecuencia (tocar (p, { { 0.0, nota, 1.0f } }, 0.8, 512, sr), sr, 0.15, 0.7);
        };
        const double a = f (true, 36), b = f (true, 48), c = f (false, 36), d = f (false, 48);
        check (std::abs (a / 65.406 - 1.0) < 0.01 && std::abs (b / a - 2.0) < 0.02 && std::abs (c - d) < 0.1 && std::abs (c / 50.0 - 1.0) < 0.01,
               "seguir a nota: cada nota no seu ton", juce::String::formatted ("(%.1f / %.1f Hz; sen seguir %.1f / %.1f)", a, b, c, d));
    }
    {   // a velocidade: ao 100 %, unha nota a un cuarto de forza soa a un cuarto; a 0 %, igual
        auto pk = [&] (float sens, float forza)
        {
            BBProcessor p;
            limpo (p, bombo::M909, 50.0f, 500.0f);
            setParam (p, "velocidade", sens);
            return pico (tocar (p, { { 0.0, 36, forza } }, 0.3, 512, sr));
        };
        const float full = pk (100, 1.0f), cuarto = pk (100, 0.25f), igual = pk (0, 0.25f);
        check (std::abs (cuarto / full - 0.25f) < 0.03f && std::abs (igual / full - 1.0f) < 0.01f, "velocidade: a forza da nota",
               juce::String::formatted ("(%.2f e %.2f)", cuarto / full, igual / full));
    }
    {   // o golpe: máis golpe, máis agudos no ataque (en cada motor)
        bool ok = true; juce::String que;
        for (int m = 0; m < bombo::NUM_MOTORES; ++m)
        {
            auto ag = [&] (float golpe)
            {
                BBProcessor p;
                limpo (p, m, 50.0f, 500.0f);
                setParam (p, "golpe", golpe); setParam (p, "maza", 80.0f);
                return agudos (tocar (p, un, 0.1, 512, sr), sr, 0.0, 0.02);
            };
            const double db = 10.0 * std::log10 (ag (100) / (ag (0) + 1e-30));
            ok = ok && db > 10.0;
            que << juce::String (juce::roundToInt (db)) << " ";
        }
        check (ok, "golpe: os agudos do ataque", "(+" + que.trim() + " dB)");
    }
    {   // o parche: o abafado acurta; a maza de plástico ten máis agudos ca a de feltro; o casco alonga
        auto render = [&] (float abaf, float maza, float casco)
        {
            BBProcessor p;
            limpo (p, bombo::ACUSTICO, 55.0f, 1200.0f);
            setParam (p, "abafado", abaf); setParam (p, "maza", maza); setParam (p, "casco", casco); setParam (p, "golpe", 50.0f);
            return tocar (p, un, 2.0, 512, sr);
        };
        const double aberto = tempoA40 (render (0, 50, 0), sr), abafado = tempoA40 (render (90, 50, 0), sr);
        const double feltro = agudos (render (0, 0, 0), sr, 0.0, 0.03), plastico = agudos (render (0, 100, 0), sr, 0.0, 0.03);
        const double sen = enerxia (render (30, 50, 0), sr, 0.4, 1.0), con = enerxia (render (30, 50, 100), sr, 0.4, 1.0);
        check (abafado < aberto * 0.6 && plastico > feltro * 4.0 && con > sen * 1.5, "parche: abafado, maza e casco",
               juce::String::formatted ("(%.0f/%.0f ms; ", aberto * 1000, abafado * 1000) + U ("plástico +")
               + juce::String::formatted ("%.0f dB; casco +%.1f dB)", 10 * std::log10 (plastico / feltro), 10 * std::log10 (con / sen)));
    }
    {   // o MIDI no seu sitio: unha nota na mostra 300 do segundo bloque empeza alí
        BBProcessor p;
        limpo (p, bombo::M909, 50.0f, 300.0f);
        setParam (p, "golpe", 100.0f);
        const auto o = tocar (p, { { 812.0 / sr, 36, 1.0f } }, 0.1, 512, sr);
        int onde = -1;
        for (int i = 0; i < o.getNumSamples() && onde < 0; ++i) if (o.getSample (0, i) != 0.0f) onde = i;
        check (onde == 812, "MIDI: a nota no seu sitio dentro do bloque", "(mostra " + juce::String (onde) + ")");
    }
    {   // un golpe abafa o anterior sen clic (fundido de 4 ms), e non se suman
        BBProcessor p;
        limpo (p, bombo::SINTETICO, 50.0f, 3000.0f);
        const auto o = tocar (p, { { 0.0, 36, 1.0f }, { 0.1013, 36, 1.0f } }, 0.4, 512, sr);
        float salto = 0;
        for (int i = (int) (0.095 * sr); i < (int) (0.12 * sr); ++i) salto = juce::jmax (salto, std::abs (o.getSample (0, i) - o.getSample (0, i - 1)));
        const float pk = pico (o, (int) (0.1 * sr), (int) (0.4 * sr)), pk1 = pico (o, 0, (int) (0.1 * sr));
        check (salto < 0.02f && pk < pk1 * 1.05f, "o golpe seguinte abafa o anterior, sen clic", juce::String::formatted ("(salto %.4f)", salto));
    }
    {   // bloques: con bloques de 512 e de 37 mostras sae o mesmo, mostra a mostra (todos os motores, con sala e carácter)
        bool ok = true;
        for (int m = 0; m < bombo::NUM_MOTORES; ++m)
        {
            auto render = [&] (int bloque)
            {
                BBProcessor p;
                p.cargarPreset (10);
                setParam (p, "motor", (float) m); setParam (p, "sala", 40.0f); setParam (p, "caracter", 60.0f); setParam (p, "sub", 30.0f);
                return tocar (p, ritmo (2.5), 3.0, bloque, sr);
            };
            const auto x = render (512), y = render (37);
            ok = ok && diferenza (x, y) == 0.0f && finito (x) && pico (x) > 0.1f;
            if (m == bombo::ACUSTICO)
            {
                std::printf ("pegada da saída: %016llx\n", (unsigned long long) pegada (x));
                if (argc > 1 && ! mode.startsWith ("--")) writeWav (juce::File (mode), x, sr);
            }
        }
        check (ok, "mesmo son con bloques de 512 e de 37");
    }
    {   // os presets: todos soan, sen valores non numéricos, sen pasar do teito
        int mal = 0; float peor = 0, menos = 10; juce::String cal, calMenos;
        for (int i = 0; i < bombo::NUM_PRESETS; ++i)
        {
            BBProcessor p;
            p.cargarPreset (i);
            const auto o = tocar (p, ritmo (2.0), 2.5, 512, sr);
            const float pk = pico (o);
            if (! finito (o) || pk < 0.2f || p.presetEditado()) { ++mal; std::printf ("   mal: %s\n", bombo::PRESETS[i].nome); }
            if (pk > peor) { peor = pk; cal = U (bombo::PRESETS[i].nome); }
            if (pk < menos) { menos = pk; calMenos = U (bombo::PRESETS[i].nome); }
        }
        check (mal == 0 && peor <= 0.8913f, "os presets soan", "(" + juce::String (bombo::NUM_PRESETS) + U ("; o máis alto, ") + cal
               + juce::String::formatted (": %.1f dBFS; ", juce::Decibels::gainToDecibels (peor)) + U ("o máis baixo, ") + calMenos
               + juce::String::formatted (": %.1f dBFS)", juce::Decibels::gainToDecibels (menos)));
    }
    {   // TOCAR: un golpe sen MIDI; «todo o son fóra» cala
        BBProcessor p;
        p.setPlayConfigDetails (0, 2, sr, 512); p.prepareToPlay (sr, 512);
        setParam (p, "caida", 3000.0f); setParam (p, "sala", 0.0f);
        juce::AudioBuffer<float> buf (2, 512);
        juce::MidiBuffer m;
        p.tocar();
        p.processBlock (buf, m);
        const float soa = pico (buf);
        m.addEvent (juce::MidiMessage::allSoundOff (1), 0);
        p.processBlock (buf, m);
        m.clear();
        p.processBlock (buf, m);
        check (soa > 0.1f && pico (buf) < 1.0e-4f && p.golpes.load() == 1, "tocar e todo o son fóra");
    }
    {   // o bypass cala o instrumento (cun fundido)
        BBProcessor p;
        setParam (p, "bypass", 1.0f);
        const auto o = tocar (p, ritmo (1.0), 1.0, 512, sr);
        check (pico (o, (int) (0.2 * sr)) < 1.0e-4f, "bypass: cala");
    }
    {   // saída mono
        BBProcessor p;
        juce::AudioProcessor::BusesLayout l;
        l.outputBuses.add (juce::AudioChannelSet::mono());
        const bool soporta = p.setBusesLayout (l);
        p.prepareToPlay (sr, 512);
        juce::AudioBuffer<float> buf (1, 512);
        juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOn (1, 36, 1.0f), 0);
        p.processBlock (buf, m);
        check (soporta && pico (buf) > 0.05f && finito (buf), "saída mono");
    }
    {   // estado: os parámetros, o preset e a escala gárdanse e recupéranse; soa igual
        BBProcessor a;
        a.cargarPreset (12);
        setParam (a, "caida", 333.0f); setParam (a, "volume", -3.0f);
        a.uiScale = 0.8f;
        juce::MemoryBlock mb; a.getStateInformation (mb);
        BBProcessor b; b.setStateInformation (mb.getData(), (int) mb.getSize());
        bool ok = b.getPreset() == 12 && b.nomePreset() == a.nomePreset() && b.presetEditado() && std::abs (b.uiScale - 0.8f) < 1e-6f;
        const auto x = tocar (a, ritmo (1.5), 1.5, 512, sr), y = tocar (b, ritmo (1.5), 1.5, 512, sr);
        ok = ok && diferenza (x, y) == 0.0f;
        check (ok, "estado gardado/recuperado (soa igual)", "(" + juce::String ((int) mb.getSize()) + " bytes)");
    }
    {   // A/B e presets propios (do núcleo), co nome do preset
        BBProcessor p;
        p.cargarPreset (0);
        p.estado.cambiarLado();
        p.cargarPreset (10);
        p.estado.cambiarLado();
        bool ok = p.estado.lado() == 0 && p.getPreset() == 0 && ! p.presetEditado();
        p.estado.cambiarLado();
        ok = ok && p.getPreset() == 10 && p.nomePreset() == U (bombo::PRESETS[10].nome);
        check (ok, "A/B (os mandos e o preset)");

        const auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("barullobombo_presets_proba");
        dir.deleteRecursively();
        p.estado.setCarpeta (dir);
        setParam (p, "caida", 777.0f);
        const bool saved = p.estado.gardar ("Proba: rock longo");
        p.cargarPreset (3);
        const auto list = p.estado.presets();
        const bool loaded = list.size() == 1 && list[0].hasFileExtension ("bbpreset") && p.estado.cargar (list[0]);
        const float caida = p.apvts.getRawParameterValue ("caida")->load();
        check (saved && loaded && std::abs (caida - 777.0f) < 0.5f && p.getPreset() == 10, "presets propios: gardar e cargar");
        dir.deleteRecursively();
    }
    {   // desfacer e refacer: cargar un preset e mover un mando, cada un no seu paso
        BBProcessor p;
        p.undo.clearUndoHistory();
        p.estado.novoPaso(); p.cargarPreset (5); p.apvts.copyState(); p.estado.novoPaso();
        setParam (p, "golpe", 90.0f); p.apvts.copyState(); p.estado.novoPaso();
        p.undo.undo();
        const bool u1 = p.getPreset() == 5 && ! p.presetEditado();
        p.undo.undo();
        const bool u2 = p.getPreset() == 0 && ! p.presetEditado();
        p.undo.redo(); p.undo.redo();
        check (u1 && u2 && p.getPreset() == 5 && p.presetEditado(), "desfacer e refacer (preset e mandos)");
    }
    {   // os programas do DAW son os presets
        BBProcessor p;
        p.setCurrentProgram (12);
        check (p.getNumPrograms() == bombo::NUM_PRESETS && p.getCurrentProgram() == 12 && p.getProgramName (12) == U (bombo::PRESETS[12].nome),
               "programas do DAW = presets");
    }
    {   // o limitador do núcleo: a +6 dB, con todo a tope, non pasa de −1 dBFS; latencia 0
        BBProcessor p;
        p.cargarPreset (17);
        setParam (p, "volume", 6.0f); setParam (p, "golpe", 100.0f); setParam (p, "sub", 100.0f); setParam (p, "sala", 100.0f);
        const float pk = pico (tocar (p, ritmo (2.0), 2.0, 512, sr));
        check (pk <= 0.8913f && p.getLatencySamples() == 0, "teito a -1 dBFS e sen latencia", juce::String::formatted ("(pico %.4f)", pk));
    }
    return fails;
}
