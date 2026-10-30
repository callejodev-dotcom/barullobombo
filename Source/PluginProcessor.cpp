#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using escoita::U;

BBProcessor::BBProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undo, "BarulloBombo", createLayout())
{
    auto p = [this] (const char* id) { return apvts.getRawParameterValue (id); };
    motorP = p ("motor"); afinacion = p ("afinacion"); caida = p ("caida"); golpe = p ("golpe"); caracter = p ("caracter");
    barrido = p ("barrido"); tempoBarrido = p ("tempoBarrido"); tonGolpe = p ("tonGolpe"); sub = p ("sub"); velocidade = p ("velocidade");
    maza = p ("maza"); abafado = p ("abafado"); casco = p ("casco"); sala = p ("sala"); seguir = p ("seguir");
    volume = p ("volume"); bypass = p ("bypass");
    nome = U (bombo::PRESETS[0].nome);
}

juce::AudioProcessorValueTreeState::ParameterLayout BBProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    const auto& p0 = bombo::PRESETS[0];   // os valores de partida son os do primeiro preset
    const StringArray onOff { "Off", "On" };
    // o texto cun número fixo de decimais (texto → valor → texto dá o mesmo texto)
    auto real = [] (const char* id, const String& nomeP, NormalisableRange<float> r, float def, const char* unidade, int decimais = 1)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nomeP, r, def, AudioParameterFloatAttributes().withLabel (unidade)
                   .withStringFromValueFunction ([decimais] (float v, int) { return String (v, decimais); }));
    };
    auto centro = [] (float mn, float mx, float c) { NormalisableRange<float> r (mn, mx); r.setSkewForCentre (c); return r; };
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "motor", 1 }, "Motor", StringArray { "808", "909", U ("Sintético"), U ("Acústico") }, p0.motor));
    l.add (real ("afinacion", U ("Afinación"), centro (25.0f, 160.0f, 55.0f), (float) p0.afinacion, "Hz"));
    l.add (real ("caida", U ("Caída"), centro (40.0f, 5000.0f, 500.0f), (float) p0.caida, "ms", 0));
    l.add (real ("golpe", "Golpe", { 0.0f, 100.0f }, (float) p0.golpe * 100.0f, "%"));
    l.add (real ("caracter", U ("Carácter"), { 0.0f, 100.0f }, (float) p0.caracter * 100.0f, "%"));
    l.add (real ("barrido", "Barrido", { 0.0f, 48.0f }, (float) p0.barrido, "st"));
    l.add (real ("tempoBarrido", "Tempo do barrido", centro (2.0f, 400.0f, 40.0f), (float) p0.tempoBarrido, "ms"));
    l.add (real ("tonGolpe", "Ton do golpe", centro (500.0f, 15000.0f, 3500.0f), (float) p0.tonGolpe, "Hz", 0));
    l.add (real ("sub", "Sub", { 0.0f, 100.0f }, (float) p0.sub * 100.0f, "%"));
    l.add (real ("velocidade", "Velocidade", { 0.0f, 100.0f }, (float) p0.velocidade * 100.0f, "%"));
    l.add (real ("maza", "Maza", { 0.0f, 100.0f }, (float) p0.maza * 100.0f, "%"));
    l.add (real ("abafado", "Abafado", { 0.0f, 100.0f }, (float) p0.abafado * 100.0f, "%"));
    l.add (real ("casco", "Casco", { 0.0f, 100.0f }, (float) p0.casco * 100.0f, "%"));
    l.add (real ("sala", "Sala", { 0.0f, 100.0f }, (float) p0.sala * 100.0f, "%"));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "seguir", 1 }, "Seguir a nota", onOff, (int) p0.seguir));
    l.add (real ("volume", "Volume", { -24.0f, 6.0f, 0.1f }, 0.0f, "dB"));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "bypass", 1 }, "Bypass", onOff, 0));
    return l;
}

// ================================================================ presets
int BBProcessor::getNumPrograms() { return bombo::NUM_PRESETS; }
const juce::String BBProcessor::getProgramName (int i) { return U (bombo::PRESETS[juce::jlimit (0, bombo::NUM_PRESETS - 1, i)].nome); }

static std::vector<std::pair<const char*, float>> valoresDe (const bombo::Preset& p)
{
    return { { "motor", (float) p.motor }, { "afinacion", (float) p.afinacion }, { "caida", (float) p.caida }, { "golpe", (float) p.golpe * 100.0f },
             { "caracter", (float) p.caracter * 100.0f }, { "barrido", (float) p.barrido }, { "tempoBarrido", (float) p.tempoBarrido },
             { "tonGolpe", (float) p.tonGolpe }, { "sub", (float) p.sub * 100.0f }, { "velocidade", (float) p.velocidade * 100.0f },
             { "maza", (float) p.maza * 100.0f }, { "abafado", (float) p.abafado * 100.0f }, { "casco", (float) p.casco * 100.0f },
             { "sala", (float) p.sala * 100.0f }, { "seguir", p.seguir ? 1.0f : 0.0f } };
}

void BBProcessor::cargarPreset (int i)
{
    i = juce::jlimit (0, bombo::NUM_PRESETS - 1, i);
    const auto antes = std::make_pair (preset, nome);
    for (auto [id, v] : valoresDe (bombo::PRESETS[i]))
        if (auto* prm = apvts.getParameter (id))
        {
            const float n = prm->convertTo0to1 (v);
            if (std::abs (prm->getValue() - n) < 1.0e-6f) continue;
            prm->beginChangeGesture(); prm->setValueNotifyingHost (n); prm->endChangeGesture();
        }
    const auto despois = std::make_pair (i, U (bombo::PRESETS[i].nome));
    // o número e o nome do preset tamén se desfán (os parámetros xa os leva o desfacer do APVTS)
    estado.cambio (U ("preset"), [this, despois] { preset = despois.first; nome = despois.second; },
                                 [this, antes] { preset = antes.first; nome = antes.second; });
    updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

bool BBProcessor::presetEditado() const
{
    const int i = juce::jlimit (0, bombo::NUM_PRESETS - 1, preset);
    if (nome != U (bombo::PRESETS[i].nome)) return false;   // un preset propio
    for (auto [id, v] : valoresDe (bombo::PRESETS[i]))
        if (auto* prm = apvts.getParameter (id))
            if (std::abs (prm->getValue() - prm->convertTo0to1 (v)) > 0.002f) return true;
    return false;
}

escoita::Estado::Opcions BBProcessor::opcionsEstado()
{
    escoita::Estado::Opcions o;
    o.extension = "bbpreset";
    o.excluir = { "bypass" };
    // o nome do que se está a escoitar vai co estado (un preset propio leva o seu)
    o.extra = [this] { juce::ValueTree t ("Preset"); t.setProperty ("numero", preset, nullptr); t.setProperty ("nome", nome, nullptr); return t; };
    o.aplicarExtra = [this] (const juce::ValueTree& t) { preset = juce::jlimit (0, bombo::NUM_PRESETS - 1, (int) t.getProperty ("numero", 0)); nome = t.getProperty ("nome", nome).toString(); };
    return o;
}

bombo::Axustes BBProcessor::axustes() const
{
    bombo::Axustes a;
    a.motor = juce::jlimit (0, bombo::NUM_MOTORES - 1, (int) std::lround (motorP->load()));
    a.afinacion = afinacion->load(); a.caida = caida->load(); a.golpe = golpe->load() / 100.0; a.caracter = caracter->load() / 100.0;
    a.barrido = barrido->load(); a.tempoBarrido = tempoBarrido->load(); a.tonGolpe = tonGolpe->load(); a.sub = sub->load() / 100.0;
    a.velocidade = velocidade->load() / 100.0; a.maza = maza->load() / 100.0; a.abafado = abafado->load() / 100.0;
    a.casco = casco->load() / 100.0; a.sala = sala->load() / 100.0; a.seguir = seguir->load() > 0.5f;
    return a;
}

double BBProcessor::getTailLengthSeconds() const { return juce::jmin (12.0, (double) caida->load() / 1000.0 * 1.7 + 0.5); }

// ================================================================ audio
// o volume en pasos de 0,1 dB: a 0 dB, a ganancia é 1 xusto
static double ganancia (float db) { return juce::Decibels::decibelsToGain (std::round ((double) db * 10.0) / 10.0); }

bool BBProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return l.getMainInputChannelSet().isDisabled() && (out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono());
}

void BBProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    setLatencySamples (0);
    motor.axustar (axustes());
    motor.preparar (sampleRate);
    teito.preparar (sampleRate, 0.080, 0.891);
    volS = ganancia (volume->load());
    bypassS = bypass->load() > 0.5f ? 1.0 : 0.0;
}

void BBProcessor::disparar (int nota, float forza)
{
    motor.disparar (forza, nota);
    ultimaNota.store (nota); ultimaForza.store (forza);
    golpes.fetch_add (1);
}

void BBProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int outs = getTotalNumOutputChannels();
    buffer.clear();
    float* L = buffer.getWritePointer (0);
    float* R = outs > 1 ? buffer.getWritePointer (1) : nullptr;

    motor.axustar (axustes());
    if (tocarPendente.exchange (false)) disparar (36, 0.8f);
    const double volT = ganancia (volume->load()), bypassT = bypass->load() > 0.5f ? 1.0 : 0.0;
    const double sm = 1.0 - std::exp (-1.0 / (0.02 * sr));
    float ps = 0;
    auto evento = midi.cbegin();
    for (int i = 0; i < n; ++i)
    {
        // as notas no seu sitio dentro do bloque
        for (; evento != midi.cend() && (*evento).samplePosition <= i; ++evento)
        {
            const auto m = (*evento).getMessage();
            if (m.isNoteOn()) disparar (m.getNoteNumber(), m.getFloatVelocity());
            else if (m.isAllNotesOff() || m.isAllSoundOff()) motor.silenciar();
        }
        volS += (volT - volS) * sm; bypassS += (bypassT - bypassS) * sm;
        double l, r;
        motor.procesar (l, r);
        l *= volS; r *= volS;
        if (R != nullptr) teito.procesar (l, r);
        else l = r = teito.procesar (0.5 * (l + r));
        // o bypass cala o instrumento (o motor segue por dentro)
        l *= 1.0 - bypassS; r *= 1.0 - bypassS;
        L[i] = (float) l;
        if (R != nullptr) R[i] = (float) r;
        ps = std::max ({ ps, (float) std::abs (l), (float) std::abs (r) });
    }
    for (int ch = 2; ch < outs; ++ch) buffer.clear (ch, 0, n);
    nivelSaida.store (ps);
}

// ================================================================ estado
void BBProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto st = apvts.copyState();
    st.setProperty ("preset", preset, nullptr);
    st.setProperty ("nomePreset", nome, nullptr);
    st.setProperty ("uiScale", uiScale, nullptr);
    if (auto xml = st.createXml()) copyXmlToBinary (*xml, destData);
}

void BBProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (! xml || ! xml->hasTagName (apvts.state.getType())) return;
    auto tree = juce::ValueTree::fromXml (*xml);
    uiScale = juce::jlimit (0.5f, 2.0f, (float) tree.getProperty ("uiScale", 1.0f));
    preset = juce::jlimit (0, bombo::NUM_PRESETS - 1, (int) tree.getProperty ("preset", 0));
    nome = tree.getProperty ("nomePreset", U (bombo::PRESETS[preset].nome)).toString();
    apvts.replaceState (tree);
    updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

juce::AudioProcessorEditor* BBProcessor::createEditor() { return new BBEditor (*this); }

#ifndef BARULLO_NO_PLUGIN_ENTRY
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BBProcessor(); }
#endif
