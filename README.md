# SawDeck

A simple go-to supersaw synth (VST3 + standalone), built on the Roland JP-8000 supersaw algorithm.

## How to get the VST3

1. Create a new GitHub repo called **SawDeck**.
2. Upload everything in this folder, including the hidden `.github` folder (it holds the build script).
3. Open the repo's **Actions** tab. The "Build SawDeck" run starts automatically and takes about 10 minutes.
4. When it's green, open the run and download **SawDeck-VST3-Windows**.
5. Unzip it and copy `SawDeck.vst3` into `C:\Program Files\Common Files\VST3`, then rescan plugins in REAPER.

There's also **SawDeck-Standalone-Windows**: a standalone `SawDeck.exe` you can play without a DAW.

## Controls

**SUPERSAW**
- **Saws** (1–16): how many stacked saws per note. 7 is the original JP-8000 layout.
- **Detune**: uses the JP-8000's own detune curve. It's gentle for most of the range and gets extreme in the last ~10%.
- **Mix**: uses the JP's mix curve. It turns the centre saw down while the detuned saws come up.
- **Spread**: stereo width of the saw stack.
- **Random Phase**: the saws start at random phases on every note (JP-style). Turn it off for a punchier transient that's the same on every note.
- **JP High-Pass**: the JP's high-pass filter that follows the note pitch. It cleans up the low end.

**PITCH**: Octave, Fine, Sub (a sine one octave down), Glide.

**FILTER**: LP / BP / HP, 12 or 24 dB, Cutoff, Reso, Env Amt (±), Key Track. It has its own ADSR (FILTER ENV).

**AMP ENV**: ADSR plus Velocity sensitivity.

**VOICE / OUTPUT**: Voices (polyphony 1–16; 1 = mono legato), Bend range, Volume, output meter.

Double-click any control to reset it. Mouse wheel nudges controls. The window is resizable and remembers its size.

## Presets

There are 16 factory presets in the dropdown. **SAVE** writes your own presets to `Documents\SawDeck Presets`, and they then show up in the dropdown under USER.
