#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <escoita_nucleo/escoita_nucleo.h>
#include "Bombo.h"

// Barullo Bombo: un sintetizador de bombos. O motor (808, 909, Sintético ou Acústico) e catro
// mandos (afinación, caída, golpe e carácter) para o día a día; detrás de «Máis», o barrido, o
// golpe, o sub, a velocidade, o parche e a sala. Os presets de fábrica son os programas.
class BBProcessor : public juce::AudioProcessor
{
public:
    BBProcessor();
    ~BBProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    juce::AudioProcessorParameter* getBypassParameter() const override { return apvts.getParameter ("bypass"); }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Barullo Bombo"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override { return preset; }
    void setCurrentProgram (int index) override { cargarPreset (index); }
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::UndoManager undo;   // desfacer e refacer (declárase antes ca o apvts)
    juce::AudioProcessorValueTreeState apvts;
    // presets propios (.bbpreset en Documentos/Barullo Bombo/Presets), A/B e desfacer, do núcleo
    escoita::Estado estado { apvts, &undo, opcionsEstado() };

    // ---- presets de fábrica (desde o fío da interface)
    void cargarPreset (int i);
    int getPreset() const { return preset; }
    juce::String nomePreset() const { return nome; }
    bool presetEditado() const;   // os mandos xa non son os do preset

    bombo::Axustes axustes() const;   // os parámetros de agora (para a pantalla)

    // o botón TOCAR: un golpe no seguinte bloque (desde a interface)
    void tocar() { tocarPendente.store (true); }

    // para a pantalla (escríbese desde o fío de audio)
    std::atomic<float> nivelSaida { 0 };
    std::atomic<int> golpes { 0 };          // cóntanse os golpes: a pantalla acende o LED e corre o cursor
    std::atomic<int> ultimaNota { 36 };
    std::atomic<float> ultimaForza { 1 };
    float uiScale = 1.0f;

private:
    escoita::Estado::Opcions opcionsEstado();
    void disparar (int nota, float forza);
    int preset = 0;
    juce::String nome;

    bombo::Bombo motor;
    escoita::Teito teito;
    std::atomic<bool> tocarPendente { false };
    double sr = 48000, volS = 1, bypassS = 0;
    std::atomic<float> *motorP = nullptr, *afinacion = nullptr, *caida = nullptr, *golpe = nullptr, *caracter = nullptr, *barrido = nullptr,
                       *tempoBarrido = nullptr, *tonGolpe = nullptr, *sub = nullptr, *velocidade = nullptr, *maza = nullptr, *abafado = nullptr,
                       *casco = nullptr, *sala = nullptr, *seguir = nullptr, *volume = nullptr, *bypass = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BBProcessor)
};
