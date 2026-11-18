#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

class SawDeckContent;

// JP-8000-style look: slate-blue body, dark navy panels, orange section titles,
// dark knobs with cyan value rings, grey fader caps, yellow LEDs.
class SawLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SawLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float sliderPos,
                           float minPos, float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Label* createSliderTextBox (juce::Slider&) override;
};

class SawDeckEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit SawDeckEditor (SawDeckProcessor&);
    ~SawDeckEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int baseW = 1000, baseH = 560;

private:
    void timerCallback() override;

    SawDeckProcessor& proc;
    SawLookAndFeel lnf;
    std::unique_ptr<SawDeckContent> content;
    bool sizeReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SawDeckEditor)
};
