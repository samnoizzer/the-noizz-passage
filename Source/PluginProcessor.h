#pragma once
#include <atomic>
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/NoizzDSPChain.h"
#include "Presets/FactoryPresets.h"

class TheNoizzPassageAudioProcessor : public juce::AudioProcessor
{
public:
    TheNoizzPassageAudioProcessor();
    ~TheNoizzPassageAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "THE NOIZZ PASSAGE"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentPresetIndex; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Preset & User Preset Management
    void loadFactoryPreset(int index);
    bool saveUserPresetToFile(const juce::File& file);
    bool loadUserPresetFromFile(const juce::File& file);

    // Dual Cabinet IR Management (Left = 0, Right = 1)
    void selectFactoryIR(int cabSide, int irIndex);
    bool loadExternalIR(int cabSide, const juce::File& wavFile);
    juce::String getActiveIRName(int cabSide) const;
    int getActiveFactoryIRIndex(int cabSide) const;

    // A/B State Comparison Management
    void switchABState(int targetSlot); // 0 = A, 1 = B
    void copyCurrentToOtherABSlot();
    int getCurrentABSlot() const noexcept { return activeABSlot; }

    // Real-time Metering Accessors (Input L/R, Output L/R, Channel Mode)
    float getInputLevelL() const noexcept  { return inPeakL.load(std::memory_order_relaxed); }
    float getInputLevelR() const noexcept  { return inPeakR.load(std::memory_order_relaxed); }
    float getOutputLevelL() const noexcept { return outPeakL.load(std::memory_order_relaxed); }
    float getOutputLevelR() const noexcept { return outPeakR.load(std::memory_order_relaxed); }
    int getActiveInputChannels() const noexcept  { return activeInChannels.load(std::memory_order_relaxed); }
    int getActiveOutputChannels() const noexcept { return activeOutChannels.load(std::memory_order_relaxed); }

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    juce::UndoManager& getUndoManager() noexcept { return undoManager; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;

    // DSP Modules
    NoizzStudio::NoiseReductionDSP noiseReduction;
    NoizzStudio::IntelligentTightGateDSP tightGate;
    NoizzStudio::SubKickBoostDSP subKickBoost;
    NoizzStudio::ZenithAmpSimDSP ampSim;
    NoizzStudio::CabinetIRConvolver cabConvolverL[2]; // stereo-capable L cab
    NoizzStudio::CabinetIRConvolver cabConvolverR[2]; // stereo-capable R cab
    NoizzStudio::PhaseSafeSpreadDSP spreadProcessor;
    juce::SpinLock irLock;

    // A/B Comparison Snapshots
    juce::ValueTree abStateSnapshots[2];
    int activeABSlot = 0; // 0 = A, 1 = B
    int currentPresetIndex = 0;

    // Lock-free atomic metering for Mono & Stereo I/O
    std::atomic<float> inPeakL  { 0.0f };
    std::atomic<float> inPeakR  { 0.0f };
    std::atomic<float> outPeakL { 0.0f };
    std::atomic<float> outPeakR { 0.0f };
    std::atomic<int> activeInChannels  { 2 };
    std::atomic<int> activeOutChannels { 2 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheNoizzPassageAudioProcessor)
};
