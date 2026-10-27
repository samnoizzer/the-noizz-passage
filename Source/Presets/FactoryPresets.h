#pragma once
#include <vector>
#include <juce_core/juce_core.h>

namespace NoizzStudio
{

struct PresetData
{
    juce::String name;
    juce::String albumOrCategory;
    float inputGainDb;
    float outputGainDb;
    float nr;          // 0..1 ("NR" Noise Reduction knob)
    bool tight;        // ("Tight" intelligent gunshot gate button)
    float boost;       // 0..1 ("Boost" sub-bass kick + bass note knob)
    float ampGain;     // 0..1
    float ampBass;     // 0..1
    float ampMid;      // 0..1
    float ampTreble;   // 0..1
    float ampPresence; // 0..1
    float ampDepth;    // 0..1
    bool cabLOn;
    int cabLIRIndex;   // 0..9
    float cabLVolDb;   // -24..+6
    float cabLPan;     // -1..+1
    bool cabROn;
    int cabRIRIndex;   // 0..9
    float cabRVolDb;   // -24..+6
    float cabRPan;     // -1..+1
    float cabMix;      // 0..1 (0 = Cab L, 0.5 = 50/50 Blend, 1.0 = Cab R)
    float spread;      // 0..1 ("Spread" Haas mono->stereo / stereo->200% widener)
};

class FactoryPresets
{
public:
    static const std::vector<PresetData>& getPresets()
    {
        static const std::vector<PresetData> presets = {
            // --- DATALYSIUM (2023) - Full Playlist Coverage ---
            {
                "01 - The Axiom of Error (Staccato Attack)", "Datalysium (2023)",
                1.5f, 0.0f, 0.62f, true, 0.48f,
                0.56f, 0.52f, 0.46f, 0.68f, 0.66f, 0.54f,
                true, 0, 0.0f, -0.85f,
                true, 1, -0.5f, 0.85f,
                0.50f, 0.55f
            },
            {
                "02 - Algorithmic Salvation (Dave Otero Hybrid)", "Datalysium (2023)",
                2.0f, 0.0f, 0.65f, true, 0.52f,
                0.54f, 0.50f, 0.48f, 0.70f, 0.68f, 0.52f,
                true, 0, 0.0f, -0.90f,
                true, 1, 0.0f, 0.90f,
                0.48f, 0.62f
            },
            {
                "03 - Lexicontagion (8-String Sub Chug)", "Datalysium (2023)",
                2.5f, -0.5f, 0.70f, true, 0.68f,
                0.52f, 0.46f, 0.52f, 0.72f, 0.65f, 0.58f,
                true, 8, 0.0f, -0.95f,
                true, 6, -0.5f, 0.95f,
                0.52f, 0.58f
            },
            {
                "04 - Synaptic Depravation (Polyrhythmic Razor)", "Datalysium (2023)",
                1.8f, 0.0f, 0.64f, true, 0.45f,
                0.58f, 0.48f, 0.50f, 0.71f, 0.70f, 0.50f,
                true, 2, 0.0f, -0.88f,
                true, 1, 0.0f, 0.88f,
                0.50f, 0.60f
            },
            {
                "05 - Deletion Cult (Machine-Gun Palm Mute)", "Datalysium (2023)",
                2.2f, 0.0f, 0.72f, true, 0.64f,
                0.55f, 0.50f, 0.44f, 0.69f, 0.67f, 0.56f,
                true, 0, 0.0f, -1.00f,
                true, 6, 0.0f, 1.00f,
                0.50f, 0.70f
            },
            {
                "06 - Divinertia I (Progressive Fusion Crunch)", "Datalysium (2023)",
                0.5f, 0.5f, 0.45f, true, 0.30f,
                0.44f, 0.54f, 0.58f, 0.64f, 0.60f, 0.48f,
                true, 7, 0.0f, -0.75f,
                true, 1, -1.0f, 0.75f,
                0.45f, 0.52f
            },
            {
                "07 - Divinertia II (Dissonant Blackened Bite)", "Datalysium (2023)",
                1.6f, 0.0f, 0.58f, true, 0.42f,
                0.62f, 0.47f, 0.53f, 0.74f, 0.72f, 0.49f,
                true, 1, 0.0f, -0.92f,
                true, 4, 0.0f, 0.92f,
                0.50f, 0.66f
            },
            {
                "08 - Automated Twilight (Sci-Fi Shimmer & Swell)", "Datalysium (2023)",
                -1.5f, 1.2f, 0.38f, false, 0.22f,
                0.32f, 0.56f, 0.52f, 0.66f, 0.64f, 0.52f,
                true, 9, 0.0f, -0.95f,
                true, 7, -1.5f, 0.95f,
                0.40f, 0.82f
            },
            {
                "09 - Datalysium (Epic Wide Master Wall)", "Datalysium (2023)",
                2.0f, 0.0f, 0.60f, true, 0.55f,
                0.57f, 0.52f, 0.49f, 0.70f, 0.68f, 0.55f,
                true, 0, 0.0f, -1.00f,
                true, 1, 0.0f, 1.00f,
                0.50f, 0.78f
            },
            // --- SOLIPSIST (2016) ---
            {
                "10 - Deus Deceptor (Zack Ohren Recto Punch)", "Solipsist (2016)",
                1.5f, 0.0f, 0.60f, true, 0.58f,
                0.60f, 0.56f, 0.45f, 0.67f, 0.64f, 0.58f,
                true, 3, 0.0f, -0.90f,
                true, 0, -0.5f, 0.90f,
                0.46f, 0.54f
            },
            {
                "11 - Holographic Principle II: Convergence", "Solipsist (2016)",
                1.8f, 0.0f, 0.62f, true, 0.50f,
                0.58f, 0.53f, 0.47f, 0.68f, 0.65f, 0.55f,
                true, 3, 0.0f, -0.85f,
                true, 4, 0.0f, 0.85f,
                0.50f, 0.56f
            },
            {
                "12 - Simulated Reality (Surgical Tech Rhythm)", "Solipsist (2016)",
                1.4f, 0.0f, 0.64f, true, 0.46f,
                0.56f, 0.50f, 0.48f, 0.70f, 0.66f, 0.52f,
                true, 3, 0.0f, -0.88f,
                true, 2, 0.0f, 0.88f,
                0.50f, 0.52f
            },
            {
                "13 - Luminary Singularity (Searing Solo Lead)", "Solipsist (2016)",
                2.4f, 0.8f, 0.52f, true, 0.25f,
                0.66f, 0.52f, 0.62f, 0.68f, 0.64f, 0.50f,
                true, 7, 0.0f, -0.65f,
                true, 1, 0.0f, 0.65f,
                0.48f, 0.68f
            },
            // --- COSMIC DISSONANCE (2013 / 2018) & SINGLES ---
            {
                "14 - Cosmic Dissonance (Cali Mod Raw Aggro)", "Cosmic Dissonance",
                2.0f, 0.0f, 0.66f, true, 0.60f,
                0.62f, 0.54f, 0.46f, 0.69f, 0.68f, 0.57f,
                true, 5, 0.0f, -0.90f,
                true, 3, 0.0f, 0.90f,
                0.50f, 0.50f
            },
            {
                "15 - Kaleidoscopic Tendencies (Sweep & Tap Clarity)", "Cosmic Dissonance",
                1.2f, 0.4f, 0.55f, true, 0.35f,
                0.53f, 0.49f, 0.55f, 0.71f, 0.66f, 0.48f,
                true, 2, 0.0f, -0.80f,
                true, 6, 0.0f, 0.80f,
                0.50f, 0.60f
            },
            {
                "16 - Fleshbound Reliquary (2025 Brutal Modern)", "Single (2025)",
                2.5f, 0.0f, 0.70f, true, 0.65f,
                0.59f, 0.51f, 0.48f, 0.73f, 0.70f, 0.58f,
                true, 1, 0.0f, -1.00f,
                true, 8, 0.0f, 1.00f,
                0.50f, 0.74f
            },
            // --- ARTIST SIGNATURE RIGS ---
            {
                "17 - Justin McKinney Signature Cali + 33 Rig", "Artist Rig",
                2.2f, 0.0f, 0.65f, true, 0.50f,
                0.51f, 0.50f, 0.49f, 0.71f, 0.68f, 0.53f,
                true, 0, 0.0f, -0.90f,
                true, 2, 0.0f, 0.90f,
                0.50f, 0.60f
            },
            {
                "18 - Chris Beattie Low-G 8-String Precision", "Artist Rig",
                2.0f, 0.0f, 0.68f, true, 0.62f,
                0.50f, 0.45f, 0.52f, 0.72f, 0.69f, 0.50f,
                true, 6, 0.0f, -0.92f,
                true, 8, 0.0f, 0.92f,
                0.50f, 0.64f
            }
        };
        return presets;
    }
};

} // namespace NoizzStudio
