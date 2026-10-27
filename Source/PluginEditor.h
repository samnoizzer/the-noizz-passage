#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "GUI/NoizzLookAndFeel.h"

class TheNoizzPassageAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit TheNoizzPassageAudioProcessorEditor(TheNoizzPassageAudioProcessor&);
    ~TheNoizzPassageAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawFuturisticPCBBackground(juce::Graphics& g, juce::Rectangle<float> area);
    void draw3DModulePanel(juce::Graphics& g, juce::Rectangle<float> rect, const juce::String& title);
    void drawStereoMeter(juce::Graphics& g, juce::Rectangle<float> rect,
                         float levelL, float levelR, int numChannels, const juce::String& title);
    void updateCabComboSelection();

    TheNoizzPassageAudioProcessor& audioProcessor;
    NoizzStudio::NoizzLookAndFeel noizzLAF;

    // Top Header Toolbar: Preset Loading Bar + Save Icon, Undo/Redo, A/B Comparison, Bypass
    juce::ComboBox presetCombo;
    juce::TextButton savePresetBtn { "SAVE" };
    juce::TextButton loadPresetBtn { "OPEN" };
    juce::TextButton undoBtn       { "UNDO" };
    juce::TextButton redoBtn       { "REDO" };
    juce::TextButton abButtonA     { "A" };
    juce::TextButton abButtonB     { "B" };
    juce::TextButton abCopyBtn     { "COPY" };
    juce::TextButton bypassBtn     { "BYPASS" };

    // Input & Output Gain Sliders
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;

    // Front-End Signature Section: NR, Tight, Boost
    juce::Slider nrKnob;
    juce::TextButton tightButton { "TIGHT" };
    juce::Slider boostKnob;
    juce::Label nrLabel, boostLabel;

    // High-Gain Amp Simulator Knobs
    juce::Slider gainKnob, bassKnob, midKnob, trebleKnob, presenceKnob, depthKnob;
    juce::Label gainLabel, bassLabel, midLabel, trebleLabel, presenceLabel, depthLabel;

    // Dual Cabinet IR Loading Bars (Left & Right) + Mix Slider + Vol & Pan Knobs + On/Off Switches
    juce::TextButton cabLOnBtn { "CAB L ON" };
    juce::ComboBox cabLIRCombo;
    juce::TextButton cabLLoadBtn { "LOAD IR..." };
    juce::Slider cabLVolKnob, cabLPanKnob;
    juce::Label cabLVolLabel, cabLPanLabel;

    juce::TextButton cabROnBtn { "CAB R ON" };
    juce::ComboBox cabRIRCombo;
    juce::TextButton cabRLoadBtn { "LOAD IR..." };
    juce::Slider cabRVolKnob, cabRPanKnob;
    juce::Label cabRVolLabel, cabRPanLabel;

    juce::Slider cabMixSlider;
    juce::Label cabMixLabel;

    // Dedicated "Spread" Knob (Mono Haas & Stereo 100%->200% Widener)
    juce::Slider spreadKnob;
    juce::Label spreadLabel;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<ButtonAttachment> bypassAttach;
    std::unique_ptr<SliderAttachment> inGainAttach, outGainAttach;
    std::unique_ptr<SliderAttachment> nrAttach, boostAttach;
    std::unique_ptr<ButtonAttachment> tightAttach;
    std::unique_ptr<SliderAttachment> gainAttach, bassAttach, midAttach, trebleAttach, presenceAttach, depthAttach;
    std::unique_ptr<ButtonAttachment> cabLOnAttach, cabROnAttach;
    std::unique_ptr<SliderAttachment> cabLVolAttach, cabLPanAttach, cabRVolAttach, cabRPanAttach, cabMixAttach;
    std::unique_ptr<SliderAttachment> spreadAttach;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheNoizzPassageAudioProcessorEditor)
};
