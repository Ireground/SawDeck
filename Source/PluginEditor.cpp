#include "PluginEditor.h"

using namespace juce;

namespace col
{
    const Colour body       { 0xff3d5573 };
    const Colour bodyDark   { 0xff324862 };
    const Colour panel      { 0xff2f445e };
    const Colour panelEdge  { 0xff58719a };
    const Colour titleBand  { 0xff283b53 };
    const Colour orange     { 0xfff2923f };
    const Colour label      { 0xffd9e0ea };
    const Colour dim        { 0xff98a9bf };
    const Colour cyan       { 0xff63d2dc };
    const Colour track      { 0xff1f2e42 };
    const Colour ledOn      { 0xfff6c445 };
    const Colour ledOff     { 0xff4a5a70 };
    const Colour lcd        { 0xff080b0e };
    const Colour lcdText    { 0xff4fd6e6 };
}

static Font uiFont (float size, bool bold = true)
{
    return Font (FontOptions (size, bold ? Font::bold : Font::plain));
}
static Font monoFont (float size)
{
   #if JUCE_WINDOWS
    return Font (FontOptions ("Consolas", size, Font::bold));
   #else
    return Font (FontOptions (Font::getDefaultMonospacedFontName(), size, Font::bold));
   #endif
}

// =====================================================================
SawLookAndFeel::SawLookAndFeel()
{
    setColour (ComboBox::backgroundColourId, col::lcd);
    setColour (ComboBox::textColourId, col::lcdText);
    setColour (ComboBox::outlineColourId, col::panelEdge);
    setColour (ComboBox::arrowColourId, col::lcdText);
    setColour (PopupMenu::backgroundColourId, Colour (0xff1e2c3e));
    setColour (PopupMenu::textColourId, col::label);
    setColour (PopupMenu::headerTextColourId, col::orange);
    setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xff3a6f86));
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (TextButton::textColourOffId, col::label);
    setColour (TextButton::textColourOnId, col::label);
    setColour (Label::textColourId, col::label);
}

void SawLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                       float startAngle, float endAngle, Slider& s)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.0f);
    const float size = jmin (b.getWidth(), b.getHeight());
    const auto c = b.getCentre();
    const float r = size * 0.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcR = r - 2.0f;

    Path trackArc;
    trackArc.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (col::track);
    g.strokePath (trackArc, PathStrokeType (3.0f, PathStrokeType::curved, PathStrokeType::rounded));

    const bool bipolar = (bool) s.getProperties().getWithDefault ("bipolar", false);
    const float from = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;
    if (std::abs (angle - from) > 0.001f)
    {
        Path v;
        v.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, jmin (from, angle), jmax (from, angle), true);
        g.setColour (s.isEnabled() ? col::cyan : col::dim);
        g.strokePath (v, PathStrokeType (3.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // knob body
    const float kr = r - 7.0f;
    g.setColour (Colours::black.withAlpha (0.35f));
    g.fillEllipse (c.x - kr, c.y - kr + 2.0f, kr * 2.0f, kr * 2.0f);
    ColourGradient grad (Colour (0xff4b525b), c.x, c.y - kr, Colour (0xff1a1d21), c.x, c.y + kr, false);
    g.setGradientFill (grad);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);
    const float ir = kr * 0.72f;
    ColourGradient grad2 (Colour (0xff2e3238), c.x, c.y - ir, Colour (0xff3a3f46), c.x, c.y + ir, false);
    g.setGradientFill (grad2);
    g.fillEllipse (c.x - ir, c.y - ir, ir * 2.0f, ir * 2.0f);
    g.setColour (Colours::black.withAlpha (0.6f));
    g.drawEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f, 1.0f);

    // pointer
    Path ptr;
    ptr.addRoundedRectangle (-1.4f, -kr + 2.5f, 2.8f, kr * 0.55f, 1.4f);
    g.setColour (Colours::white.withAlpha (s.isEnabled() ? 0.95f : 0.4f));
    g.fillPath (ptr, AffineTransform::rotation (angle).translated (c.x, c.y));
}

void SawLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int w, int h, float sliderPos,
                                       float minPos, float maxPos, Slider::SliderStyle style, Slider& s)
{
    if (style != Slider::LinearVertical)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos, minPos, maxPos, style, s);
        return;
    }

    const float cx = (float) x + (float) w * 0.5f;
    const float top = (float) y, bottom = (float) (y + h);

    // tick marks
    g.setColour (col::dim.withAlpha (0.5f));
    for (int i = 0; i <= 10; ++i)
    {
        const float ty = top + (bottom - top) * (float) i / 10.0f;
        const float len = (i % 5 == 0) ? 7.0f : 4.0f;
        g.drawLine (cx - 9.0f - len, ty, cx - 9.0f, ty, 1.0f);
    }

    // slot
    auto slot = Rectangle<float> (cx - 3.0f, top - 2.0f, 6.0f, bottom - top + 4.0f);
    g.setColour (Colour (0xff111a26));
    g.fillRoundedRectangle (slot, 3.0f);
    auto fill = Rectangle<float> (cx - 1.5f, sliderPos, 3.0f, bottom - sliderPos);
    g.setColour (col::cyan);
    g.fillRoundedRectangle (fill, 1.5f);

    // cap
    auto cap = Rectangle<float> (cx - 13.0f, sliderPos - 7.0f, 26.0f, 14.0f);
    g.setColour (Colours::black.withAlpha (0.4f));
    g.fillRoundedRectangle (cap.translated (0.0f, 2.0f), 2.5f);
    ColourGradient grad (Colour (0xffd9dde2), cap.getX(), cap.getY(), Colour (0xff8d949c), cap.getX(), cap.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (cap, 2.5f);
    g.setColour (Colour (0xff22262b));
    g.fillRect (cap.getX() + 3.0f, sliderPos - 0.75f, cap.getWidth() - 6.0f, 1.5f);
}

void SawLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& b, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const float d = 9.0f;
    auto led = Rectangle<float> (r.getX() + 3.0f, r.getCentreY() - d * 0.5f, d, d);
    const bool on = b.getToggleState();
    if (on)
    {
        g.setColour (col::ledOn.withAlpha (0.25f));
        g.fillEllipse (led.expanded (3.0f));
        g.setColour (col::ledOn);
    }
    else g.setColour (highlighted ? col::ledOff.brighter (0.3f) : col::ledOff);
    g.fillEllipse (led);
    g.setColour (Colours::black.withAlpha (0.5f));
    g.drawEllipse (led, 0.8f);

    g.setColour (on ? col::label : col::dim);
    g.setFont (uiFont (11.0f));
    g.drawText (b.getButtonText(), r.withLeft (led.getRight() + 6.0f), Justification::centredLeft, false);
}

void SawLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    Colour fill (0xff24354b);
    if (down) fill = fill.darker (0.3f);
    else if (highlighted) fill = fill.brighter (0.15f);
    g.setColour (fill);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (col::panelEdge);
    g.drawRoundedRectangle (r, 4.0f, 1.0f);
}

Font SawLookAndFeel::getTextButtonFont (TextButton&, int) { return uiFont (11.0f); }
Font SawLookAndFeel::getComboBoxFont (ComboBox&)          { return monoFont (13.0f); }
Font SawLookAndFeel::getPopupMenuFont()                   { return uiFont (13.0f, false); }

Label* SawLookAndFeel::createSliderTextBox (Slider& s)
{
    auto* l = LookAndFeel_V4::createSliderTextBox (s);
    l->setFont (monoFont (11.0f));
    l->setColour (Label::textColourId, col::label);
    l->setColour (Label::backgroundColourId, Colours::transparentBlack);
    l->setColour (Label::outlineColourId, Colours::transparentBlack);
    l->setColour (Label::textWhenEditingColourId, col::label);
    l->setColour (TextEditor::backgroundColourId, col::lcd);
    l->setColour (TextEditor::textColourId, col::lcdText);
    l->setColour (TextEditor::highlightColourId, col::cyan.withAlpha (0.3f));
    return l;
}

// =====================================================================
// Knob / fader with caption on top and value box underneath
class KnobBox : public Component
{
public:
    KnobBox (AudioProcessorValueTreeState& state, const String& paramId, const String& title,
             bool fader = false, bool bipolar = false)
        : caption (title)
    {
        slider.setSliderStyle (fader ? Slider::LinearVertical : Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (Slider::TextBoxBelow, false, 70, 16);
        slider.getProperties().set ("bipolar", bipolar);
        slider.setColour (Slider::textBoxTextColourId, col::label);
        slider.setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
        slider.setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
        slider.setMouseDragSensitivity (220);
        addAndMakeVisible (slider);
        attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (state, paramId, slider);
        if (auto* p = state.getParameter (paramId))
            slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    }

    void paint (Graphics& g) override
    {
        g.setColour (col::label);
        g.setFont (uiFont (11.0f));
        g.drawText (caption, getLocalBounds().removeFromTop (14), Justification::centred, false);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        b.removeFromTop (15);
        slider.setBounds (b);
    }

    Slider slider;
private:
    String caption;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// =====================================================================
// LCD: preset name, the actual saw stack (detune + level) and the stereo field
class SawDisplay : public Component
{
public:
    explicit SawDisplay (SawDeckProcessor& p) : proc (p) { preview.prepare (48000.0); }

    void refresh()
    {
        auto q = proc.getParamSnapshot();
        const String key = String (q.saws) + "|" + String (q.detune, 3) + "|" + String (q.mix, 3) + "|"
                         + String (q.spread, 3) + "|" + proc.currentPresetName + "|" + String (proc.activeVoices.load());
        if (key != lastKey)
        {
            lastKey = key;
            preview.setParams (q);
            repaint();
        }
    }

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (col::lcd);
        g.fillRoundedRectangle (b, 5.0f);
        g.setColour (Colour (0xff1b2735));
        g.drawRoundedRectangle (b.reduced (0.5f), 5.0f, 1.5f);

        auto in = b.reduced (10.0f, 7.0f);
        g.setFont (monoFont (12.0f));
        g.setColour (col::lcdText);
        auto head = in.removeFromTop (16.0f);
        const int n = preview.layoutCount();
        g.drawText (n == 7 ? "7 SAWS  JP" : String (n) + " SAWS", head, Justification::centredRight, false);
        g.drawText (proc.currentPresetName.toUpperCase(), head.withTrimmedRight (90.0f), Justification::centredLeft, true);

        auto stereo = in.removeFromBottom (18.0f);
        in.removeFromBottom (4.0f);
        auto wave = in;

        // grid
        g.setColour (col::lcdText.withAlpha (0.08f));
        for (int i = 1; i < 4; ++i)
            g.drawHorizontalLine ((int) (wave.getY() + wave.getHeight() * i / 4.0f), wave.getX(), wave.getRight());

        float maxGain = 0.0001f;
        for (int i = 0; i < n; ++i) maxGain = jmax (maxGain, preview.layoutGain (i));
        const double det = preview.detuneAmount();
        const float midY = wave.getCentreY(), amp = wave.getHeight() * 0.45f;

        for (int i = 0; i < n; ++i)
        {
            const float gn = preview.layoutGain (i) / maxGain;
            const double ratio = 1.0 + preview.layoutOffset (i) * det * 18.0;
            const double cycles = 2.5 * ratio;
            const double ph = std::fmod (i * 0.137, 1.0);
            Path p;
            const float w = wave.getWidth();
            for (float x = 0.0f; x <= w; x += 1.5f)
            {
                const double t = std::fmod (x / w * cycles + ph, 1.0);
                const float yy = midY - (float) (2.0 * t - 1.0) * amp * (0.35f + 0.65f * gn);
                if (x == 0.0f) p.startNewSubPath (wave.getX() + x, yy);
                else p.lineTo (wave.getX() + x, yy);
            }
            g.setColour (col::lcdText.withAlpha (0.18f + 0.6f * gn));
            g.strokePath (p, PathStrokeType (1.1f));
        }

        // stereo field
        g.setFont (monoFont (10.0f));
        g.setColour (col::lcdText.withAlpha (0.6f));
        g.drawText ("L", stereo.removeFromLeft (12.0f), Justification::centredLeft, false);
        g.drawText ("R", stereo.removeFromRight (12.0f), Justification::centredRight, false);
        g.setColour (col::lcdText.withAlpha (0.25f));
        g.drawHorizontalLine ((int) stereo.getCentreY(), stereo.getX(), stereo.getRight());
        g.drawVerticalLine ((int) stereo.getCentreX(), stereo.getY() + 3.0f, stereo.getBottom() - 3.0f);
        for (int i = 0; i < n; ++i)
        {
            const float gn = preview.layoutGain (i) / maxGain;
            const float px = stereo.getCentreX() + preview.layoutPan (i) * stereo.getWidth() * 0.5f;
            const float rad = 1.5f + 3.0f * gn;
            g.setColour (col::lcdText.withAlpha (0.35f + 0.65f * gn));
            g.fillEllipse (px - rad, stereo.getCentreY() - rad, rad * 2.0f, rad * 2.0f);
        }
    }

private:
    SawDeckProcessor& proc;
    sawdeck::Engine preview;   // only used to compute the saw layout for drawing
    String lastKey;
};

// =====================================================================
class OutputMeter : public Component
{
public:
    void setLevels (float l, float r, int voices)
    {
        lvl[0] = jmax (l, lvl[0] * 0.85f);
        lvl[1] = jmax (r, lvl[1] * 0.85f);
        activeVoices = voices;
        repaint();
    }
    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setFont (uiFont (11.0f));
        g.setColour (col::label);
        g.drawText ("OUTPUT", b.removeFromTop (14.0f), Justification::centred, false);
        auto info = b.removeFromBottom (16.0f);
        g.setFont (monoFont (11.0f));
        g.drawText (String (activeVoices) + " playing", info, Justification::centred, false);
        b.removeFromBottom (4.0f);
        b = b.withSizeKeepingCentre (34.0f, b.getHeight());
        for (int ch = 0; ch < 2; ++ch)
        {
            auto bar = Rectangle<float> (b.getX() + ch * 20.0f, b.getY(), 14.0f, b.getHeight());
            g.setColour (Colour (0xff111a26));
            g.fillRoundedRectangle (bar, 3.0f);
            const float db = Decibels::gainToDecibels (lvl[ch], -60.0f);
            const float frac = jlimit (0.0f, 1.0f, (db + 60.0f) / 66.0f);
            auto f = bar.reduced (2.0f);
            f = f.removeFromBottom (f.getHeight() * frac);
            g.setColour (lvl[ch] >= 1.0f ? Colour (0xffe8503a) : col::cyan);
            g.fillRoundedRectangle (f, 2.0f);
            // 0 dB tick
            const float zeroY = bar.getBottom() - 2.0f - (bar.getHeight() - 4.0f) * (60.0f / 66.0f);
            g.setColour (col::orange.withAlpha (0.8f));
            g.drawHorizontalLine ((int) zeroY, bar.getX() - 2.0f, bar.getRight() + 2.0f);
        }
    }
private:
    float lvl[2] { 0.0f, 0.0f };
    int activeVoices = 0;
};

// =====================================================================
class SawDeckContent : public Component
{
public:
    explicit SawDeckContent (SawDeckProcessor& p)
        : proc (p), st (p.apvts), display (p),
          saws (st, "saws", "SAWS"), detune (st, "detune", "DETUNE"), mix (st, "mix", "MIX"), spread (st, "spread", "SPREAD"),
          octave (st, "octave", "OCTAVE", false, true), fine (st, "fine", "FINE", false, true),
          sub (st, "sub", "SUB"), glide (st, "glide", "GLIDE"),
          cutoff (st, "cutoff", "CUTOFF"), reso (st, "reso", "RESO"), fenv (st, "fEnv", "ENV AMT", false, true), key (st, "keyTrk", "KEY TRK"),
          fA (st, "fA", "A", true), fD (st, "fD", "D", true), fS (st, "fS", "S", true), fR (st, "fR", "R", true),
          aA (st, "aA", "A", true), aD (st, "aD", "D", true), aS (st, "aS", "S", true), aR (st, "aR", "R", true),
          vel (st, "vel", "VELOCITY"), voices (st, "voices", "VOICES"), bend (st, "bend", "BEND"), volume (st, "volume", "VOLUME"),
          keyboard (p.keyboardState, MidiKeyboardComponent::horizontalKeyboard)
    {
        for (auto* k : { &saws, &detune, &mix, &spread, &octave, &fine, &sub, &glide, &cutoff, &reso, &fenv, &key,
                         &fA, &fD, &fS, &fR, &aA, &aD, &aS, &aR, &vel, &voices, &bend, &volume })
            addAndMakeVisible (k);

        addAndMakeVisible (display);
        addAndMakeVisible (meter);

        setupLed (randPhase, "RANDOM PHASE"); randAtt = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (st, "randPhase", randPhase);
        setupLed (jpHpf, "JP HIGH-PASS");     hpfAtt  = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (st, "jpHpf", jpHpf);
        setupLed (slope24, "24 dB");          slopeAtt = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (st, "f24", slope24);
        randPhase.setTooltip ("On: every note starts the saws at random phases (like the JP-8000's free-running oscillators). Off: same phases every note, punchier and repeatable.");
        jpHpf.setTooltip ("The JP-8000's pitch-tracking high-pass filter on the saw stack. Cleans up the low end like the original.");

        const char* typeNames[] = { "LPF", "BPF", "HPF" };
        for (int i = 0; i < 3; ++i)
        {
            setupLed (typeBtn[i], typeNames[i]);
            typeBtn[i].setRadioGroupId (4242);
            typeBtn[i].onClick = [this, i]
            {
                if (! typeBtn[i].getToggleState()) return;
                if (auto* prm = st.getParameter ("fType"))
                {
                    prm->beginChangeGesture();
                    prm->setValueNotifyingHost (prm->convertTo0to1 ((float) i));
                    prm->endChangeGesture();
                }
            };
        }

        // presets
        for (auto* b : { &prevBtn, &nextBtn, &saveBtn, &loadBtn }) addAndMakeVisible (b);
        prevBtn.setButtonText ("<"); nextBtn.setButtonText (">");
        saveBtn.setButtonText ("SAVE"); loadBtn.setButtonText ("LOAD");
        addAndMakeVisible (presetBox);
        presetBox.setJustificationType (Justification::centred);
        presetBox.onChange = [this] { onPresetChosen(); };
        prevBtn.onClick = [this] { stepPreset (-1); };
        nextBtn.onClick = [this] { stepPreset (1); };
        saveBtn.onClick = [this] { savePreset(); };
        loadBtn.onClick = [this] { loadPreset(); };
        refreshPresetList();

        // keyboard
        keyboard.setAvailableRange (36, 96);
        keyboard.setScrollButtonsVisible (false);
        keyboard.setWantsKeyboardFocus (false);
        keyboard.setColour (MidiKeyboardComponent::whiteNoteColourId, Colour (0xffeceff2));
        keyboard.setColour (MidiKeyboardComponent::blackNoteColourId, Colour (0xff15181c));
        keyboard.setColour (MidiKeyboardComponent::keySeparatorLineColourId, Colour (0xff6b7480));
        keyboard.setColour (MidiKeyboardComponent::mouseOverKeyOverlayColourId, col::cyan.withAlpha (0.25f));
        keyboard.setColour (MidiKeyboardComponent::keyDownOverlayColourId, col::cyan.withAlpha (0.7f));
        keyboard.setColour (MidiKeyboardComponent::shadowColourId, Colours::black.withAlpha (0.35f));
        addAndMakeVisible (keyboard);

        setSize (SawDeckEditor::baseW, SawDeckEditor::baseH);
    }

    void tick()
    {
        display.refresh();
        const int t = (int) st.getRawParameterValue ("fType")->load();
        for (int i = 0; i < 3; ++i)
            if (typeBtn[i].getToggleState() != (i == t))
                typeBtn[i].setToggleState (i == t, dontSendNotification);

        meter.setLevels (proc.peakL.exchange (0.0f), proc.peakR.exchange (0.0f), proc.activeVoices.load());

        if (presetBox.getText() != proc.currentPresetName)
            presetBox.setText (proc.currentPresetName, dontSendNotification);

        const bool mono = (int) st.getRawParameterValue ("voices")->load() <= 1;
        if (mono != lastMono) { lastMono = mono; repaint (voiceSec); }
    }

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setGradientFill (ColourGradient (col::body, 0.0f, 0.0f, col::bodyDark, 0.0f, b.getBottom(), false));
        g.fillAll();

        // header
        g.setColour (Colour (0xffc9d3df));
        g.setFont (uiFont (28.0f).withExtraKerningFactor (0.12f));
        g.drawText ("SAWDECK", Rectangle<int> (22, 6, 220, 32), Justification::centredLeft, false);
        g.setColour (col::orange);
        g.setFont (uiFont (10.5f).withExtraKerningFactor (0.2f));
        g.drawText ("SUPERSAW SYNTHESIZER", Rectangle<int> (24, 36, 240, 14), Justification::centredLeft, false);
        g.setColour (Colour (0xff6a83a6));
        g.setFont (uiFont (17.0f).withExtraKerningFactor (0.12f));
        g.drawText ("JP-8000 ALGORITHM", Rectangle<int> (752, 12, 234, 30), Justification::centredRight, false);

        for (auto& s : sections) drawSection (g, s.first, s.second);

        // small hints
        g.setFont (uiFont (10.0f, false));
        g.setColour (col::dim);
        g.drawText ("7 saws = the original JP-8000 layout", Rectangle<int> (238, 226, 240, 14), Justification::centred, false);
        g.drawText ("TYPE", Rectangle<int> (660, 86, 60, 12), Justification::centredLeft, false);
        g.drawText ("SLOPE", Rectangle<int> (660, 176, 60, 12), Justification::centredLeft, false);
        g.drawText (lastMono ? "MONO / LEGATO" : "POLY", Rectangle<int> (630, 392, 244, 14), Justification::centred, false);
        g.drawText ("Voices 1 = mono legato with glide", Rectangle<int> (630, 410, 244, 14), Justification::centred, false);
        g.drawText ("Double-click a control to reset it", Rectangle<int> (630, 428, 244, 14), Justification::centred, false);
    }

    void resized() override
    {
        // header
        prevBtn.setBounds (318, 14, 26, 26);
        presetBox.setBounds (348, 14, 250, 26);
        nextBtn.setBounds (602, 14, 26, 26);
        saveBtn.setBounds (636, 14, 50, 26);
        loadBtn.setBounds (690, 14, 50, 26);

        // supersaw
        display.setBounds (24, 90, 204, 158);
        SawKnobRow ({ &saws, &detune, &mix, &spread }, 236, 88, 60, 92);
        randPhase.setBounds (242, 190, 118, 20);
        jpHpf.setBounds (362, 190, 118, 20);

        // pitch
        octave.setBounds (497, 88, 68, 82);  fine.setBounds (567, 88, 68, 82);
        sub.setBounds    (497, 172, 68, 82); glide.setBounds (567, 172, 68, 82);

        // filter
        for (int i = 0; i < 3; ++i) typeBtn[i].setBounds (660, 100 + i * 22, 64, 20);
        slope24.setBounds (660, 190, 64, 20);
        cutoff.setBounds (734, 88, 124, 82); reso.setBounds (860, 88, 124, 82);
        fenv.setBounds   (734, 172, 124, 82); key.setBounds (860, 172, 124, 82);

        // filter env / amp env
        int x = 24;
        for (auto* f : { &fA, &fD, &fS, &fR }) { f->setBounds (x, 292, 58, 156); x += 60; }
        x = 292;
        for (auto* f : { &aA, &aD, &aS, &aR }) { f->setBounds (x, 292, 58, 156); x += 60; }
        vel.setBounds (530, 300, 76, 92);

        // voice / output
        voices.setBounds (632, 292, 80, 94);
        bend.setBounds   (714, 292, 80, 94);
        volume.setBounds (796, 292, 80, 94);
        meter.setBounds  (886, 290, 94, 158);

        keyboard.setBounds (12, 460, 976, 92);
        keyboard.setKeyWidth (976.0f / 36.0f);
    }

private:
    void SawKnobRow (std::initializer_list<KnobBox*> knobs, int x, int y, int w, int h)
    {
        for (auto* k : knobs) { k->setBounds (x, y, w, h); x += w; }
    }

    void setupLed (ToggleButton& b, const String& text)
    {
        b.setButtonText (text);
        addAndMakeVisible (b);
    }

    static void drawSection (Graphics& g, Rectangle<int> r, const String& title)
    {
        auto rf = r.toFloat();
        g.setColour (Colours::black.withAlpha (0.25f));
        g.fillRoundedRectangle (rf.translated (0.0f, 2.0f), 5.0f);
        g.setColour (col::panel);
        g.fillRoundedRectangle (rf, 5.0f);
        auto band = rf.removeFromTop (20.0f);
        g.setColour (col::titleBand);
        g.fillRoundedRectangle (band, 5.0f);
        g.fillRect (band.withTrimmedTop (10.0f));
        g.setColour (col::panelEdge);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 5.0f, 1.0f);

        g.setFont (uiFont (11.5f).withExtraKerningFactor (0.1f));
        GlyphArrangement ga;
        ga.addLineOfText (g.getCurrentFont(), title, 0.0f, 0.0f);
        const float tw = ga.getBoundingBox (0, -1, true).getWidth() + 16.0f;
        g.setColour (col::orange.withAlpha (0.55f));
        const float cy = band.getCentreY();
        g.drawLine (band.getX() + 12.0f, cy, band.getCentreX() - tw * 0.5f, cy, 1.0f);
        g.drawLine (band.getCentreX() + tw * 0.5f, cy, band.getRight() - 12.0f, cy, 1.0f);
        g.setColour (col::orange);
        g.drawText (title, band, Justification::centred, false);
    }

    // ---- presets ----
    void refreshPresetList()
    {
        presetBox.clear (dontSendNotification);
        auto names = proc.getFactoryPresetNames();
        presetBox.addSectionHeading ("FACTORY");
        for (int i = 0; i < names.size(); ++i) presetBox.addItem (names[i], i + 1);
        userFiles = proc.getUserPresets();
        if (! userFiles.isEmpty())
        {
            presetBox.addSeparator();
            presetBox.addSectionHeading ("USER");
            for (int i = 0; i < userFiles.size(); ++i)
                presetBox.addItem (userFiles[i].getFileNameWithoutExtension(), 1001 + i);
        }
        presetBox.setText (proc.currentPresetName, dontSendNotification);
    }

    void loadById (int id)
    {
        if (id >= 1001) { auto idx = id - 1001; if (isPositiveAndBelow (idx, userFiles.size())) proc.loadUserPreset (userFiles[idx]); }
        else if (id >= 1) proc.loadFactoryPreset (id - 1);
        presetBox.setText (proc.currentPresetName, dontSendNotification);
    }

    void onPresetChosen()
    {
        const int id = presetBox.getSelectedId();
        if (id != 0) loadById (id);
    }

    void stepPreset (int dir)
    {
        Array<int> ids;
        for (int i = 0; i < presetBox.getNumItems(); ++i) ids.add (presetBox.getItemId (i));
        if (ids.isEmpty()) return;
        int cur = -1;
        for (int i = 0; i < presetBox.getNumItems(); ++i)
            if (presetBox.getItemText (i) == proc.currentPresetName) { cur = i; break; }
        int next = cur < 0 ? 0 : (cur + dir + ids.size()) % ids.size();
        loadById (ids[next]);
    }

    void savePreset()
    {
        auto folder = proc.getUserPresetFolder();
        folder.createDirectory();
        chooser = std::make_unique<FileChooser> ("Save SawDeck preset",
                                                 folder.getChildFile (File::createLegalFileName (proc.currentPresetName) + ".sawdeck"),
                                                 "*.sawdeck");
        chooser->launchAsync (FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles
                              | FileBrowserComponent::warnAboutOverwriting,
                              [this] (const FileChooser& fc)
                              {
                                  auto f = fc.getResult();
                                  if (f == File()) return;
                                  proc.saveUserPreset (f.withFileExtension ("sawdeck"));
                                  refreshPresetList();
                              });
    }

    void loadPreset()
    {
        chooser = std::make_unique<FileChooser> ("Load SawDeck preset", proc.getUserPresetFolder(), "*.sawdeck");
        chooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                              [this] (const FileChooser& fc)
                              {
                                  auto f = fc.getResult();
                                  if (f.existsAsFile()) proc.loadUserPreset (f);
                                  refreshPresetList();
                              });
    }

    SawDeckProcessor& proc;
    AudioProcessorValueTreeState& st;

    SawDisplay display;
    OutputMeter meter;
    KnobBox saws, detune, mix, spread, octave, fine, sub, glide, cutoff, reso, fenv, key,
            fA, fD, fS, fR, aA, aD, aS, aR, vel, voices, bend, volume;
    ToggleButton randPhase, jpHpf, slope24, typeBtn[3];
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> randAtt, hpfAtt, slopeAtt;

    TextButton prevBtn, nextBtn, saveBtn, loadBtn;
    ComboBox presetBox;
    Array<File> userFiles;
    std::unique_ptr<FileChooser> chooser;

    MidiKeyboardComponent keyboard;
    TooltipWindow tooltipWindow { this, 700 };
    bool lastMono = false;

    const Rectangle<int> voiceSec { 618, 266, 370, 186 };
    const std::vector<std::pair<Rectangle<int>, String>> sections {
        { { 12, 62, 470, 196 },  "SUPERSAW" },
        { { 490, 62, 150, 196 }, "PITCH" },
        { { 648, 62, 340, 196 }, "FILTER" },
        { { 12, 266, 260, 186 }, "FILTER ENV" },
        { { 280, 266, 330, 186 }, "AMP ENV" },
        { voiceSec,              "VOICE / OUTPUT" },
    };
};

// =====================================================================
SawDeckEditor::SawDeckEditor (SawDeckProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);
    content = std::make_unique<SawDeckContent> (p);
    addAndMakeVisible (*content);
    content->sendLookAndFeelChange();

    setResizable (true, true);
    setResizeLimits (600, 336, 2000, 1120);
    getConstrainer()->setFixedAspectRatio ((double) baseW / (double) baseH);

    const int w = jlimit (600, 2000, proc.uiWidth);
    sizeReady = true;
    setSize (w, roundToInt (w * (double) baseH / (double) baseW));

    startTimerHz (30);
}

SawDeckEditor::~SawDeckEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void SawDeckEditor::paint (Graphics& g) { g.fillAll (col::bodyDark); }

void SawDeckEditor::resized()
{
    if (content == nullptr) return;
    const float scale = (float) getWidth() / (float) baseW;
    content->setTransform (AffineTransform::scale (scale));
    content->setBounds (0, 0, baseW, baseH);
    if (sizeReady)
    {
        proc.uiWidth  = getWidth();
        proc.uiHeight = getHeight();
    }
}

void SawDeckEditor::timerCallback()
{
    if (content != nullptr) content->tick();
}
