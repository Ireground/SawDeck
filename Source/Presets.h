#pragma once
#include <vector>
#include <utility>

// Factory presets. Values are in real units (Hz, seconds, 0..1, etc).
// Anything not listed uses the parameter's default.
// fType: 0 = LP, 1 = BP, 2 = HP.   Booleans: 0 / 1.

struct FactoryPreset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;
};

inline const std::vector<FactoryPreset>& getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets =
    {
        { "Init Supersaw", {} },

        { "JP Classic Lead", { { "saws", 7 }, { "detune", 0.6f }, { "mix", 0.7f }, { "spread", 0.8f },
            { "cutoff", 9000 }, { "reso", 0.15f }, { "aA", 0.003f }, { "aD", 0.3f }, { "aS", 1.0f }, { "aR", 0.4f },
            { "vel", 0.2f }, { "volume", 0.0f } } },

        { "Trance Anthem Lead", { { "saws", 7 }, { "detune", 0.7f }, { "mix", 0.8f }, { "spread", 1.0f },
            { "sub", 0.2f }, { "cutoff", 11000 }, { "reso", 0.1f }, { "fEnv", 0.15f },
            { "fA", 0.001f }, { "fD", 0.6f }, { "fS", 0.3f }, { "aR", 0.5f } } },

        { "Euphoric Pad", { { "saws", 12 }, { "detune", 0.65f }, { "mix", 0.85f }, { "spread", 1.0f },
            { "cutoff", 3500 }, { "reso", 0.15f }, { "fEnv", 0.25f }, { "fA", 1.2f }, { "fD", 2.0f }, { "fS", 0.6f }, { "fR", 2.0f },
            { "aA", 0.8f }, { "aD", 1.0f }, { "aS", 1.0f }, { "aR", 2.5f }, { "vel", 0.1f } } },

        { "Trance Pluck", { { "saws", 7 }, { "detune", 0.55f }, { "mix", 0.6f }, { "spread", 0.8f },
            { "cutoff", 500 }, { "reso", 0.25f }, { "fEnv", 0.7f }, { "f24", 1 },
            { "fA", 0.001f }, { "fD", 0.25f }, { "fS", 0.0f }, { "fR", 0.25f },
            { "aA", 0.001f }, { "aD", 0.35f }, { "aS", 0.0f }, { "aR", 0.3f }, { "vel", 0.5f }, { "volume", 3.0f } } },

        { "Hardstyle Lead", { { "saws", 9 }, { "detune", 0.72f }, { "mix", 0.75f }, { "spread", 0.9f },
            { "sub", 0.3f }, { "cutoff", 7000 }, { "reso", 0.3f }, { "fEnv", 0.2f },
            { "fA", 0.001f }, { "fD", 0.15f }, { "fS", 0.2f },
            { "aA", 0.001f }, { "aD", 0.2f }, { "aS", 0.9f }, { "aR", 0.2f } } },

        { "Future Bass Chords", { { "saws", 9 }, { "detune", 0.6f }, { "mix", 0.7f }, { "spread", 1.0f },
            { "cutoff", 2500 }, { "reso", 0.2f }, { "fEnv", 0.35f }, { "f24", 1 },
            { "fA", 0.02f }, { "fD", 0.4f }, { "fS", 0.4f },
            { "aA", 0.01f }, { "aD", 0.5f }, { "aS", 0.8f }, { "aR", 0.3f } } },

        { "Mono Glide Lead", { { "voices", 1 }, { "glide", 0.12f }, { "saws", 7 }, { "detune", 0.58f }, { "mix", 0.65f },
            { "spread", 0.6f }, { "cutoff", 8000 }, { "reso", 0.2f },
            { "aA", 0.002f }, { "aS", 1.0f }, { "aR", 0.2f }, { "volume", 0.0f } } },

        { "Mega Wide 16", { { "saws", 16 }, { "detune", 0.7f }, { "mix", 0.9f }, { "spread", 1.0f },
            { "cutoff", 14000 }, { "aR", 0.6f } } },

        { "Soft Strings", { { "saws", 10 }, { "detune", 0.5f }, { "mix", 0.7f }, { "spread", 0.9f },
            { "cutoff", 4000 }, { "reso", 0.0f }, { "keyTrk", 0.5f },
            { "aA", 0.4f }, { "aD", 0.5f }, { "aS", 0.9f }, { "aR", 1.2f }, { "vel", 0.4f } } },

        { "Big Room Stab", { { "saws", 8 }, { "detune", 0.72f }, { "mix", 0.8f }, { "spread", 0.7f },
            { "sub", 0.4f }, { "cutoff", 1800 }, { "reso", 0.2f }, { "fEnv", 0.5f },
            { "fA", 0.001f }, { "fD", 0.18f }, { "fS", 0.15f },
            { "aA", 0.001f }, { "aD", 0.25f }, { "aS", 0.3f }, { "aR", 0.15f } } },

        { "Dark Sweep Pad", { { "saws", 11 }, { "detune", 0.62f }, { "mix", 0.8f }, { "spread", 1.0f },
            { "cutoff", 400 }, { "reso", 0.45f }, { "fEnv", 0.5f }, { "f24", 1 },
            { "fA", 4.0f }, { "fD", 4.0f }, { "fS", 0.5f }, { "fR", 3.0f },
            { "aA", 1.5f }, { "aS", 1.0f }, { "aR", 3.0f } } },

        { "Saw Bass", { { "voices", 1 }, { "glide", 0.04f }, { "saws", 5 }, { "detune", 0.35f }, { "mix", 0.4f },
            { "spread", 0.2f }, { "octave", -1 }, { "sub", 0.5f },
            { "cutoff", 1500 }, { "reso", 0.2f }, { "fEnv", 0.3f }, { "f24", 1 },
            { "fA", 0.001f }, { "fD", 0.3f }, { "fS", 0.2f },
            { "aA", 0.001f }, { "aD", 0.3f }, { "aS", 0.8f }, { "aR", 0.08f } } },

        { "Airy Top Layer (HP)", { { "saws", 7 }, { "detune", 0.65f }, { "mix", 0.8f }, { "spread", 1.0f },
            { "fType", 2 }, { "cutoff", 400 }, { "reso", 0.2f }, { "aR", 0.5f } } },

        { "Detune Chaos", { { "saws", 16 }, { "detune", 1.0f }, { "mix", 1.0f }, { "spread", 1.0f },
            { "cutoff", 10000 } } },

        { "Pure Saw (1 Osc)", { { "saws", 1 }, { "spread", 0.0f }, { "jpHpf", 0 } } },
    };
    return presets;
}
