#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "SynthEngine.h"

class SawDeckProcessor : public juce::AudioProcessor
{
public:
    SawDeckProcessor();
    ~SawDeckProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SawDeck"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- presets ----
    juce::StringArray getFactoryPresetNames() const;
    void loadFactoryPreset (int index);
    juce::File getUserPresetFolder() const;
    juce::Array<juce::File> getUserPresets() const;
    bool loadUserPreset (const juce::File& file);
    bool saveUserPreset (const juce::File& file);

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    juce::String currentPresetName { "Init Supersaw" };

    // editor size (kept here so it survives the editor closing / FX chain switching)
    int uiWidth = 1000, uiHeight = 560;

    // GUI-readable state
    std::atomic<int> activeVoices { 0 };
    std::atomic<float> peakL { 0.0f }, peakR { 0.0f };
    sawdeck::Params getParamSnapshot() const;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void resetAllToDefault();

    sawdeck::Engine engine;
    juce::SmoothedValue<float> outGain;
    int currentProgram = 0;

    std::atomic<float>* p (const char* id) const { return apvts.getRawParameterValue (id); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SawDeckProcessor)
};
