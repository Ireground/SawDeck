#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using namespace juce;

static String timeText (float s, int)
{
    if (s < 1.0f) return String (roundToInt (s * 1000.0f)) + " ms";
    return String (s, 2) + " s";
}
static String pctText (float v, int)    { return String (roundToInt (v * 100.0f)) + "%"; }
static String bipctText (float v, int)  { auto i = roundToInt (v * 100.0f); return (i > 0 ? "+" : "") + String (i) + "%"; }

static NormalisableRange<float> timeRange (float lo, float hi, float centre)
{
    NormalisableRange<float> r (lo, hi);
    r.setSkewForCentre (centre);
    return r;
}

AudioProcessorValueTreeState::ParameterLayout SawDeckProcessor::createLayout()
{
    using F = AudioParameterFloat;  using I = AudioParameterInt;
    using B = AudioParameterBool;   using C = AudioParameterChoice;
    using Attr = AudioParameterFloatAttributes;

    std::vector<std::unique_ptr<RangedAudioParameter>> ps;
    auto pct   = Attr().withStringFromValueFunction (pctText);
    auto bipct = Attr().withStringFromValueFunction (bipctText);
    auto tm    = Attr().withStringFromValueFunction (timeText);

    // supersaw
    ps.push_back (std::make_unique<I> (ParameterID { "saws", 1 }, "Saws", 1, 16, 7));
    ps.push_back (std::make_unique<F> (ParameterID { "detune", 1 }, "Detune", NormalisableRange<float> (0.0f, 1.0f), 0.5f, pct));
    ps.push_back (std::make_unique<F> (ParameterID { "mix", 1 }, "Mix", NormalisableRange<float> (0.0f, 1.0f), 0.65f, pct));
    ps.push_back (std::make_unique<F> (ParameterID { "spread", 1 }, "Stereo Spread", NormalisableRange<float> (0.0f, 1.0f), 0.75f, pct));
    ps.push_back (std::make_unique<B> (ParameterID { "randPhase", 1 }, "Random Phase", true));
    ps.push_back (std::make_unique<B> (ParameterID { "jpHpf", 1 }, "JP High-Pass", true));

    // pitch
    ps.push_back (std::make_unique<I> (ParameterID { "octave", 1 }, "Octave", -2, 2, 0));
    ps.push_back (std::make_unique<F> (ParameterID { "fine", 1 }, "Fine", NormalisableRange<float> (-100.0f, 100.0f), 0.0f,
                                       Attr().withStringFromValueFunction ([] (float v, int) { auto i = roundToInt (v); return (i > 0 ? "+" : "") + String (i) + " ct"; })));
    ps.push_back (std::make_unique<F> (ParameterID { "sub", 1 }, "Sub", NormalisableRange<float> (0.0f, 1.0f), 0.0f, pct));
    ps.push_back (std::make_unique<F> (ParameterID { "glide", 1 }, "Glide", timeRange (0.0f, 2.0f, 0.2f), 0.0f,
                                       Attr().withStringFromValueFunction ([] (float v, int) { return v < 0.0005f ? String ("Off") : timeText (v, 0); })));

    // filter
    ps.push_back (std::make_unique<C> (ParameterID { "fType", 1 }, "Filter Type", StringArray { "LP", "BP", "HP" }, 0));
    ps.push_back (std::make_unique<B> (ParameterID { "f24", 1 }, "Filter 24 dB", false));
    ps.push_back (std::make_unique<F> (ParameterID { "cutoff", 1 }, "Cutoff", timeRange (20.0f, 20000.0f, 1000.0f), 20000.0f,
                                       Attr().withStringFromValueFunction ([] (float v, int) { return v >= 1000.0f ? String (v / 1000.0f, 1) + " kHz" : String (roundToInt (v)) + " Hz"; })));
    ps.push_back (std::make_unique<F> (ParameterID { "reso", 1 }, "Resonance", NormalisableRange<float> (0.0f, 1.0f), 0.1f, pct));
    ps.push_back (std::make_unique<F> (ParameterID { "fEnv", 1 }, "Filter Env", NormalisableRange<float> (-1.0f, 1.0f), 0.0f, bipct));
    ps.push_back (std::make_unique<F> (ParameterID { "keyTrk", 1 }, "Key Track", NormalisableRange<float> (0.0f, 1.0f), 0.0f, pct));

    ps.push_back (std::make_unique<F> (ParameterID { "fA", 1 }, "Filter Attack",  timeRange (0.001f, 10.0f, 0.5f), 0.005f, tm));
    ps.push_back (std::make_unique<F> (ParameterID { "fD", 1 }, "Filter Decay",   timeRange (0.001f, 10.0f, 0.5f), 0.5f, tm));
    ps.push_back (std::make_unique<F> (ParameterID { "fS", 1 }, "Filter Sustain", NormalisableRange<float> (0.0f, 1.0f), 0.5f, pct));
    ps.push_back (std::make_unique<F> (ParameterID { "fR", 1 }, "Filter Release", timeRange (0.001f, 10.0f, 0.5f), 0.3f, tm));

    // amp
    ps.push_back (std::make_unique<F> (ParameterID { "aA", 1 }, "Amp Attack",  timeRange (0.001f, 10.0f, 0.5f), 0.005f, tm));
    ps.push_back (std::make_unique<F> (ParameterID { "aD", 1 }, "Amp Decay",   timeRange (0.001f, 10.0f, 0.5f), 0.4f, tm));
    ps.push_back (std::make_unique<F> (ParameterID { "aS", 1 }, "Amp Sustain", NormalisableRange<float> (0.0f, 1.0f), 1.0f, pct));
    ps.push_back (std::make_unique<F> (ParameterID { "aR", 1 }, "Amp Release", timeRange (0.001f, 10.0f, 0.5f), 0.35f, tm));
    ps.push_back (std::make_unique<F> (ParameterID { "vel", 1 }, "Velocity", NormalisableRange<float> (0.0f, 1.0f), 0.3f, pct));

    // voice / output
    ps.push_back (std::make_unique<I> (ParameterID { "voices", 1 }, "Voices", 1, 16, 8));
    ps.push_back (std::make_unique<I> (ParameterID { "bend", 1 }, "Bend Range", 0, 12, 2));
    ps.push_back (std::make_unique<F> (ParameterID { "volume", 1 }, "Volume", NormalisableRange<float> (-36.0f, 6.0f), -3.0f,
                                       Attr().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " dB"; })));

    return { ps.begin(), ps.end() };
}

SawDeckProcessor::SawDeckProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SawDeck", createLayout())
{
}

bool SawDeckProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == AudioChannelSet::stereo();
}

void SawDeckProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
    engine.setParams (getParamSnapshot());
    outGain.reset (sampleRate, 0.03);
    outGain.setCurrentAndTargetValue (Decibels::decibelsToGain (p ("volume")->load()));
    keyboardState.reset();
}

sawdeck::Params SawDeckProcessor::getParamSnapshot() const
{
    sawdeck::Params q;
    q.saws        = (int) p ("saws")->load();
    q.detune      = p ("detune")->load();
    q.mix         = p ("mix")->load();
    q.spread      = p ("spread")->load();
    q.randomPhase = p ("randPhase")->load() > 0.5f;
    q.jpHpf       = p ("jpHpf")->load() > 0.5f;
    q.octave      = (int) p ("octave")->load();
    q.fineCents   = p ("fine")->load();
    q.sub         = p ("sub")->load();
    q.glide       = p ("glide")->load();
    q.filterType  = (int) p ("fType")->load();
    q.slope24     = p ("f24")->load() > 0.5f;
    q.cutoff      = p ("cutoff")->load();
    q.reso        = p ("reso")->load();
    q.envAmt      = p ("fEnv")->load();
    q.keyTrack    = p ("keyTrk")->load();
    q.fA = p ("fA")->load(); q.fD = p ("fD")->load(); q.fS = p ("fS")->load(); q.fR = p ("fR")->load();
    q.aA = p ("aA")->load(); q.aD = p ("aD")->load(); q.aS = p ("aS")->load(); q.aR = p ("aR")->load();
    q.velSens     = p ("vel")->load();
    q.polyphony   = (int) p ("voices")->load();
    q.bendRange   = p ("bend")->load();
    return q;
}

void SawDeckProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);
    engine.setParams (getParamSnapshot());

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    HeapBlock<float> tmpR;
    if (R == nullptr) { tmpR.calloc ((size_t) numSamples); R = tmpR.get(); }

    int pos = 0;
    for (const auto meta : midi)
    {
        const int evPos = jlimit (0, numSamples, meta.samplePosition);
        if (evPos > pos) { engine.render (L, R, pos, evPos - pos); pos = evPos; }

        const auto m = meta.getMessage();
        if (m.isNoteOn())                     engine.noteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff())               engine.noteOff (m.getNoteNumber());
        else if (m.isPitchWheel())            engine.pitchBend (m.getPitchWheelValue());
        else if (m.isSustainPedalOn())        engine.sustainPedal (true);
        else if (m.isSustainPedalOff())       engine.sustainPedal (false);
        else if (m.isAllNotesOff() || m.isAllSoundOff()) engine.allNotesOff();
    }
    if (pos < numSamples) engine.render (L, R, pos, numSamples - pos);

    outGain.setTargetValue (Decibels::decibelsToGain (p ("volume")->load()));
    for (int i = 0; i < numSamples; ++i)
    {
        const float g = outGain.getNextValue();
        L[i] *= g; R[i] *= g;
    }
    activeVoices.store (engine.activeVoiceCount());
    peakL.store (jmax (peakL.load(), buffer.getMagnitude (0, 0, numSamples)));
    if (buffer.getNumChannels() > 1) peakR.store (jmax (peakR.load(), buffer.getMagnitude (1, 0, numSamples)));
    else peakR.store (peakL.load());
}

AudioProcessorEditor* SawDeckProcessor::createEditor() { return new SawDeckEditor (*this); }

// ---------------- presets ----------------
int SawDeckProcessor::getNumPrograms() { return (int) getFactoryPresets().size(); }

const String SawDeckProcessor::getProgramName (int index)
{
    auto& fp = getFactoryPresets();
    return isPositiveAndBelow (index, (int) fp.size()) ? String (fp[(size_t) index].name) : String();
}

void SawDeckProcessor::setCurrentProgram (int index)
{
    // some hosts call this right after restoring a session; don't wipe the sound then
    if (index == currentProgram) return;
    loadFactoryPreset (index);
}

StringArray SawDeckProcessor::getFactoryPresetNames() const
{
    StringArray s;
    for (auto& fp : getFactoryPresets()) s.add (fp.name);
    return s;
}

void SawDeckProcessor::resetAllToDefault()
{
    for (auto* param : getParameters())
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (param))
            rp->setValueNotifyingHost (rp->getDefaultValue());
}

void SawDeckProcessor::loadFactoryPreset (int index)
{
    auto& fps = getFactoryPresets();
    if (! isPositiveAndBelow (index, (int) fps.size())) return;
    resetAllToDefault();
    for (auto& kv : fps[(size_t) index].values)
        if (auto* rp = apvts.getParameter (kv.first))
            rp->setValueNotifyingHost (rp->convertTo0to1 (kv.second));
    currentProgram = index;
    currentPresetName = fps[(size_t) index].name;
}

File SawDeckProcessor::getUserPresetFolder() const
{
    return File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("SawDeck Presets");
}

Array<File> SawDeckProcessor::getUserPresets() const
{
    auto files = getUserPresetFolder().findChildFiles (File::findFiles, false, "*.sawdeck");
    files.sort();
    return files;
}

bool SawDeckProcessor::saveUserPreset (const File& file)
{
    auto state = apvts.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    if (xml == nullptr) return false;
    file.getParentDirectory().createDirectory();
    if (! xml->writeTo (file)) return false;
    currentPresetName = file.getFileNameWithoutExtension();
    return true;
}

bool SawDeckProcessor::loadUserPreset (const File& file)
{
    auto xml = XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return false;
    auto tree = ValueTree::fromXml (*xml);
    resetAllToDefault();
    for (auto* param : getParameters())
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (param))
        {
            auto child = tree.getChildWithProperty ("id", rp->paramID);
            if (child.isValid() && child.hasProperty ("value"))
                rp->setValueNotifyingHost (rp->convertTo0to1 ((float) child.getProperty ("value")));
        }
    currentPresetName = file.getFileNameWithoutExtension();
    currentProgram = -1;
    return true;
}

// ---------------- state ----------------
void SawDeckProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    xml->setAttribute ("uiW", uiWidth);
    xml->setAttribute ("uiH", uiHeight);
    xml->setAttribute ("presetName", currentPresetName);
    xml->setAttribute ("program", currentProgram);
    copyXmlToBinary (*xml, destData);
}

void SawDeckProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return;
    uiWidth  = xml->getIntAttribute ("uiW", uiWidth);
    uiHeight = xml->getIntAttribute ("uiH", uiHeight);
    currentPresetName = xml->getStringAttribute ("presetName", currentPresetName);
    currentProgram = xml->getIntAttribute ("program", currentProgram);
    apvts.replaceState (ValueTree::fromXml (*xml));
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SawDeckProcessor(); }
