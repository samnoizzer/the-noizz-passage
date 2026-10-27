#include "PluginProcessor.h"
#include "PluginEditor.h"

TheNoizzPassageAudioProcessorEditor::TheNoizzPassageAudioProcessorEditor(TheNoizzPassageAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&noizzLAF);

    // Resizable aspect-ratio-locked window (16:10 widescreen futuristic chassis)
    setResizable(true, true);
    setResizeLimits(880, 550, 1920, 1200);
    getConstrainer()->setFixedAspectRatio(16.0 / 10.0);
    setSize(1120, 700);

    auto setupKnob = [this](juce::Slider& s, juce::Label& l, const juce::String& text, const juce::String& suffix = "")
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);
        if (suffix.isNotEmpty())
            s.setTextValueSuffix(suffix);
        addAndMakeVisible(s);

        l.setText(text, juce::dontSendNotification);
        l.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(13.0f));
        l.setColour(juce::Label::textColourId, NoizzStudio::NoizzLookAndFeel::kBrightText);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };

    // --- Top Toolbar: Presets Bar + Save/Open + Undo/Redo + A/B + Bypass ---
    const auto& presets = NoizzStudio::FactoryPresets::getPresets();
    for (int i = 0; i < int(presets.size()); ++i)
        presetCombo.addItem(presets[size_t(i)].name, i + 1);
    presetCombo.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetCombo.onChange = [this]()
    {
        const int idx = presetCombo.getSelectedId() - 1;
        if (idx >= 0)
        {
            audioProcessor.loadFactoryPreset(idx);
            updateCabComboSelection();
        }
    };
    addAndMakeVisible(presetCombo);

    savePresetBtn.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Save THE NOIZZ PASSAGE Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.noizz");
        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                                 [this](const juce::FileChooser& fc)
                                 {
                                     auto f = fc.getResult();
                                     if (f != juce::File{})
                                         audioProcessor.saveUserPresetToFile(f.withFileExtension(".noizz"));
                                 });
    };
    addAndMakeVisible(savePresetBtn);

    loadPresetBtn.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Load THE NOIZZ PASSAGE Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.noizz");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                 [this](const juce::FileChooser& fc)
                                 {
                                     auto f = fc.getResult();
                                     if (f.existsAsFile())
                                     {
                                         audioProcessor.loadUserPresetFromFile(f);
                                         updateCabComboSelection();
                                     }
                                 });
    };
    addAndMakeVisible(loadPresetBtn);

    undoBtn.onClick = [this]() { audioProcessor.getUndoManager().undo(); };
    redoBtn.onClick = [this]() { audioProcessor.getUndoManager().redo(); };
    addAndMakeVisible(undoBtn);
    addAndMakeVisible(redoBtn);

    abButtonA.setClickingTogglesState(false);
    abButtonB.setClickingTogglesState(false);
    abButtonA.setToggleState(true, juce::dontSendNotification);
    abButtonA.onClick = [this]()
    {
        audioProcessor.switchABState(0);
        abButtonA.setToggleState(true, juce::dontSendNotification);
        abButtonB.setToggleState(false, juce::dontSendNotification);
    };
    abButtonB.onClick = [this]()
    {
        audioProcessor.switchABState(1);
        abButtonA.setToggleState(false, juce::dontSendNotification);
        abButtonB.setToggleState(true, juce::dontSendNotification);
    };
    abCopyBtn.onClick = [this]() { audioProcessor.copyCurrentToOtherABSlot(); };
    addAndMakeVisible(abButtonA);
    addAndMakeVisible(abButtonB);
    addAndMakeVisible(abCopyBtn);

    bypassBtn.setClickingTogglesState(true);
    addAndMakeVisible(bypassBtn);
    bypassAttach = std::make_unique<ButtonAttachment>(audioProcessor.getAPVTS(), "bypass", bypassBtn);

    // --- Input & Output Sliders ---
    inputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 18);
    inputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(inputGainSlider);
    inputGainLabel.setText("INPUT", juce::dontSendNotification);
    inputGainLabel.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(12.5f));
    inputGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(inputGainLabel);
    inGainAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "input_gain", inputGainSlider);

    outputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 18);
    outputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(outputGainSlider);
    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    outputGainLabel.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(12.5f));
    outputGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputGainLabel);
    outGainAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "output_gain", outputGainSlider);

    // --- Front-End Signature Section: NR, Tight, Boost ---
    setupKnob(nrKnob, nrLabel, "NR");
    nrAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "nr", nrKnob);

    tightButton.setClickingTogglesState(true);
    addAndMakeVisible(tightButton);
    tightAttach = std::make_unique<ButtonAttachment>(audioProcessor.getAPVTS(), "tight", tightButton);

    setupKnob(boostKnob, boostLabel, "BOOST");
    boostAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "boost", boostKnob);

    // --- High-Gain Amp Section ---
    setupKnob(gainKnob, gainLabel, "GAIN");
    setupKnob(bassKnob, bassLabel, "BASS");
    setupKnob(midKnob, midLabel, "MID");
    setupKnob(trebleKnob, trebleLabel, "TREBLE");
    setupKnob(presenceKnob, presenceLabel, "PRESENCE");
    setupKnob(depthKnob, depthLabel, "DEPTH");

    gainAttach     = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "amp_gain", gainKnob);
    bassAttach     = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "amp_bass", bassKnob);
    midAttach      = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "amp_mid", midKnob);
    trebleAttach   = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "amp_treble", trebleKnob);
    presenceAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "amp_presence", presenceKnob);
    depthAttach    = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "amp_depth", depthKnob);

    // --- Dual Cabinet IR Loader (CAB L & CAB R) ---
    const auto& irBank = NoizzStudio::EmbeddedIRBank::getFactoryIRs();
    for (int i = 0; i < NoizzStudio::EmbeddedIRBank::kNumFactoryIRs; ++i)
    {
        cabLIRCombo.addItem(irBank[size_t(i)].displayName, i + 1);
        cabRIRCombo.addItem(irBank[size_t(i)].displayName, i + 1);
    }
    updateCabComboSelection();

    cabLIRCombo.onChange = [this]()
    {
        const int idx = cabLIRCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < NoizzStudio::EmbeddedIRBank::kNumFactoryIRs)
            audioProcessor.selectFactoryIR(0, idx);
    };
    cabRIRCombo.onChange = [this]()
    {
        const int idx = cabRIRCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < NoizzStudio::EmbeddedIRBank::kNumFactoryIRs)
            audioProcessor.selectFactoryIR(1, idx);
    };
    addAndMakeVisible(cabLIRCombo);
    addAndMakeVisible(cabRIRCombo);

    cabLOnBtn.setClickingTogglesState(true);
    cabROnBtn.setClickingTogglesState(true);
    addAndMakeVisible(cabLOnBtn);
    addAndMakeVisible(cabROnBtn);
    cabLOnAttach = std::make_unique<ButtonAttachment>(audioProcessor.getAPVTS(), "cabL_on", cabLOnBtn);
    cabROnAttach = std::make_unique<ButtonAttachment>(audioProcessor.getAPVTS(), "cabR_on", cabROnBtn);

    cabLLoadBtn.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Load External Left Cabinet IR (.wav / .aiff)",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.wav;*.aiff;*.aif");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                 [this](const juce::FileChooser& fc)
                                 {
                                     auto f = fc.getResult();
                                     if (f.existsAsFile() && audioProcessor.loadExternalIR(0, f))
                                         cabLIRCombo.setText("EXT: " + f.getFileNameWithoutExtension(), juce::dontSendNotification);
                                 });
    };
    cabRLoadBtn.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Load External Right Cabinet IR (.wav / .aiff)",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.wav;*.aiff;*.aif");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                 [this](const juce::FileChooser& fc)
                                 {
                                     auto f = fc.getResult();
                                     if (f.existsAsFile() && audioProcessor.loadExternalIR(1, f))
                                         cabRIRCombo.setText("EXT: " + f.getFileNameWithoutExtension(), juce::dontSendNotification);
                                 });
    };
    addAndMakeVisible(cabLLoadBtn);
    addAndMakeVisible(cabRLoadBtn);

    setupKnob(cabLVolKnob, cabLVolLabel, "CAB L VOL", " dB");
    setupKnob(cabLPanKnob, cabLPanLabel, "CAB L PAN");
    setupKnob(cabRVolKnob, cabRVolLabel, "CAB R VOL", " dB");
    setupKnob(cabRPanKnob, cabRPanLabel, "CAB R PAN");

    cabLVolAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "cabL_vol", cabLVolKnob);
    cabLPanAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "cabL_pan", cabLPanKnob);
    cabRVolAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "cabR_vol", cabRVolKnob);
    cabRPanAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "cabR_pan", cabRPanKnob);

    cabMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    cabMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    addAndMakeVisible(cabMixSlider);
    cabMixLabel.setText("CAB L / R MIX SLIDER", juce::dontSendNotification);
    cabMixLabel.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(12.5f));
    cabMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cabMixLabel);
    cabMixAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "cab_mix", cabMixSlider);

    // --- Dedicated "Spread" Knob ---
    setupKnob(spreadKnob, spreadLabel, "SPREAD (100-200%)");
    spreadAttach = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), "spread", spreadKnob);

    startTimerHz(30);
}

TheNoizzPassageAudioProcessorEditor::~TheNoizzPassageAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void TheNoizzPassageAudioProcessorEditor::updateCabComboSelection()
{
    const int idxL = audioProcessor.getActiveFactoryIRIndex(0);
    const int idxR = audioProcessor.getActiveFactoryIRIndex(1);
    if (idxL >= 0)
        cabLIRCombo.setSelectedId(idxL + 1, juce::dontSendNotification);
    else
        cabLIRCombo.setText("EXT: " + audioProcessor.getActiveIRName(0), juce::dontSendNotification);

    if (idxR >= 0)
        cabRIRCombo.setSelectedId(idxR + 1, juce::dontSendNotification);
    else
        cabRIRCombo.setText("EXT: " + audioProcessor.getActiveIRName(1), juce::dontSendNotification);
}

void TheNoizzPassageAudioProcessorEditor::timerCallback()
{
    repaint();
}

void TheNoizzPassageAudioProcessorEditor::drawFuturisticPCBBackground(juce::Graphics& g, juce::Rectangle<float> area)
{
    // 1. Deep 3D Chassis Base Gradient (Pitch Black to Dark Metallic Charcoal)
    juce::ColourGradient bgGrad(juce::Colour(0xff050608), area.getX(), area.getY(),
                                juce::Colour(0xff12151e), area.getRight(), area.getBottom(), false);
    bgGrad.addColour(0.5, juce::Colour(0xff0b0d13));
    g.setGradientFill(bgGrad);
    g.fillRect(area);

    // 2. Procedural Futuristic PCB Circuit Traces & Glowing Neon Red Vias
    const float w = area.getWidth();
    const float h = area.getHeight();

    // Subtle dark copper grid lines
    g.setColour(NoizzStudio::NoizzLookAndFeel::kTraceDarkRed.withAlpha(0.35f));
    const int numTraces = 28;
    for (int i = 0; i < numTraces; ++i)
    {
        const float y0 = h * (0.08f + 0.86f * (float(i) / float(numTraces)));
        const float xStart = (i % 2 == 0) ? 0.0f : w * 0.65f;
        const float xMid1  = xStart + w * 0.12f;
        const float xMid2  = xMid1 + h * 0.04f;
        const float yShift = y0 + ((i % 3 == 0) ? 18.0f : -18.0f);
        const float xEnd   = xMid2 + w * 0.16f;

        juce::Path trace;
        trace.startNewSubPath(xStart, y0);
        trace.lineTo(xMid1, y0);
        trace.lineTo(xMid2, yShift); // 45-degree PCB routing angle
        trace.lineTo(xEnd, yShift);

        g.setColour((i % 4 == 0) ? NoizzStudio::NoizzLookAndFeel::kNeonRed.withAlpha(0.28f)
                                 : NoizzStudio::NoizzLookAndFeel::kTraceDarkRed.withAlpha(0.45f));
        g.strokePath(trace, juce::PathStrokeType((i % 4 == 0) ? 1.8f : 1.1f));

        // Solder via pad at trace endpoint
        g.setColour(NoizzStudio::NoizzLookAndFeel::kNeonRed.withAlpha(0.55f));
        g.drawEllipse(xEnd - 3.5f, yShift - 3.5f, 7.0f, 7.0f, 1.3f);
        g.setColour(juce::Colour(0xff050608));
        g.fillEllipse(xEnd - 1.8f, yShift - 1.8f, 3.6f, 3.6f);
    }

    // Outer 3D Beveled Frame
    g.setColour(NoizzStudio::NoizzLookAndFeel::kNeonRed.withAlpha(0.65f));
    g.drawRoundedRectangle(area.reduced(3.0f), 8.0f, 2.0f);
}

void TheNoizzPassageAudioProcessorEditor::draw3DModulePanel(juce::Graphics& g, juce::Rectangle<float> rect, const juce::String& title)
{
    // 3D drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillRoundedRectangle(rect.translated(2.0f, 3.5f), 6.0f);

    // Metallic dark gray panel body with semi-transparent PCB visibility
    juce::ColourGradient panelGrad(juce::Colour(0xe8191c26), rect.getX(), rect.getY(),
                                   juce::Colour(0xe80c0e14), rect.getX(), rect.getBottom(), false);
    g.setGradientFill(panelGrad);
    g.fillRoundedRectangle(rect, 6.0f);

    // Glowing Neon Red corner brackets + dark metallic border
    g.setColour(NoizzStudio::NoizzLookAndFeel::kMetallicEdge);
    g.drawRoundedRectangle(rect, 6.0f, 1.2f);

    // Header bar with Neon Red underline
    auto headerRect = rect.removeFromTop(26.0f);
    g.setColour(NoizzStudio::NoizzLookAndFeel::kNeonRed.withAlpha(0.18f));
    g.fillRoundedRectangle(headerRect, 6.0f);
    g.setColour(NoizzStudio::NoizzLookAndFeel::kNeonRed);
    g.drawLine(headerRect.getX() + 8.0f, headerRect.getBottom(), headerRect.getRight() - 8.0f, headerRect.getBottom(), 1.5f);

    g.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(13.5f));
    g.setColour(NoizzStudio::NoizzLookAndFeel::kBrightText);
    g.drawText(title, headerRect.reduced(10.0f, 0.0f), juce::Justification::centredLeft, false);
}

void TheNoizzPassageAudioProcessorEditor::drawStereoMeter(juce::Graphics& g, juce::Rectangle<float> rect,
                                                          float levelL, float levelR, int numChannels,
                                                          const juce::String& title)
{
    g.setColour(juce::Colour(0xff08090d));
    g.fillRoundedRectangle(rect, 5.0f);
    g.setColour(NoizzStudio::NoizzLookAndFeel::kMetallicEdge);
    g.drawRoundedRectangle(rect, 5.0f, 1.2f);

    auto topLabel = rect.removeFromTop(20.0f);
    g.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(11.0f));
    g.setColour(NoizzStudio::NoizzLookAndFeel::kBrightText);
    g.drawText(title + (numChannels == 1 ? " [M]" : " [S]"), topLabel, juce::Justification::centred, false);

    auto barsArea = rect.reduced(7.0f, 6.0f);
    const float barW = (barsArea.getWidth() - 4.0f) * 0.5f;
    juce::Rectangle<float> barL(barsArea.getX(), barsArea.getY(), barW, barsArea.getHeight() - 14.0f);
    juce::Rectangle<float> barR(barsArea.getX() + barW + 4.0f, barsArea.getY(), barW, barsArea.getHeight() - 14.0f);

    auto drawSingleBar = [&](juce::Rectangle<float> b, float linLevel, const char* chLabel)
    {
        g.setColour(juce::Colour(0xff040507));
        g.fillRoundedRectangle(b, 2.0f);

        const float db = juce::Decibels::gainToDecibels(std::max(linLevel, 1.0e-5f), -60.0f);
        const float norm = juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f);

        const int numSegments = 24;
        const float segGap = 1.5f;
        const float segH = (b.getHeight() - segGap * (numSegments - 1)) / float(numSegments);

        for (int s = 0; s < numSegments; ++s)
        {
            const float segNorm = float(s + 1) / float(numSegments);
            const float sy = b.getBottom() - (s + 1) * segH - s * segGap;
            juce::Rectangle<float> segRect(b.getX() + 1.0f, sy, b.getWidth() - 2.0f, segH);

            if (norm >= segNorm)
            {
                juce::Colour c = (segNorm > 0.92f) ? juce::Colour(0xffffeb3b)
                               : (segNorm > 0.72f) ? NoizzStudio::NoizzLookAndFeel::kNeonRedBright
                                                   : NoizzStudio::NoizzLookAndFeel::kNeonRed.darker(0.15f);
                g.setColour(c);
                g.fillRect(segRect);
            }
            else
            {
                g.setColour(juce::Colour(0xff161922));
                g.fillRect(segRect);
            }
        }

        g.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(10.5f));
        g.setColour(NoizzStudio::NoizzLookAndFeel::kBrightText);
        g.drawText(chLabel, juce::Rectangle<float>(b.getX(), b.getBottom() + 2.0f, b.getWidth(), 12.0f),
                   juce::Justification::centred, false);
    };

    drawSingleBar(barL, levelL, "L");
    drawSingleBar(barR, (numChannels == 1) ? levelL : levelR, "R");
}

void TheNoizzPassageAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto fullBounds = getLocalBounds().toFloat();
    drawFuturisticPCBBackground(g, fullBounds);

    const float pad = fullBounds.getWidth() * 0.012f;
    auto content = fullBounds.reduced(pad);

    // 1. Top Header Bar
    auto headerArea = content.removeFromTop(content.getHeight() * 0.11f);
    g.setColour(juce::Colour(0xee0c0e14));
    g.fillRoundedRectangle(headerArea, 6.0f);
    g.setColour(NoizzStudio::NoizzLookAndFeel::kNeonRed);
    g.drawRoundedRectangle(headerArea, 6.0f, 1.5f);

    // Plugin Title & Developer Branding on Left of Header
    auto brandArea = headerArea.reduced(14.0f, 4.0f).withWidth(headerArea.getWidth() * 0.28f);
    g.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(headerArea.getHeight() * 0.42f));
    g.setColour(NoizzStudio::NoizzLookAndFeel::kNeonRedBright);
    g.drawText("THE NOIZZ PASSAGE", brandArea.removeFromTop(brandArea.getHeight() * 0.62f),
               juce::Justification::centredLeft, false);
    g.setFont(NoizzStudio::NoizzLookAndFeel::getBoldBrightFont(headerArea.getHeight() * 0.24f));
    g.setColour(NoizzStudio::NoizzLookAndFeel::kBrightText.withAlpha(0.85f));
    g.drawText("BY NOIZZ STUDIO // ZENITH TECH-DEATH AMP SUITE", brandArea,
               juce::Justification::centredLeft, false);

    content.removeFromTop(pad);

    // 2. Left & Right Input/Output Meter + Slider Strips
    const float ioStripW = content.getWidth() * 0.105f;
    auto leftStrip  = content.removeFromLeft(ioStripW);
    content.removeFromLeft(pad);
    auto rightStrip = content.removeFromRight(ioStripW);
    content.removeFromRight(pad);

    draw3DModulePanel(g, leftStrip, "IN MONITOR");
    draw3DModulePanel(g, rightStrip, "OUT MONITOR");

    auto inMeterRect = leftStrip.withTrimmedTop(32.0f).withTrimmedBottom(leftStrip.getHeight() * 0.42f).reduced(8.0f, 4.0f);
    drawStereoMeter(g, inMeterRect,
                    audioProcessor.getInputLevelL(),
                    audioProcessor.getInputLevelR(),
                    audioProcessor.getActiveInputChannels(),
                    "IN");

    auto outMeterRect = rightStrip.withTrimmedTop(32.0f).withTrimmedBottom(rightStrip.getHeight() * 0.42f).reduced(8.0f, 4.0f);
    drawStereoMeter(g, outMeterRect,
                    audioProcessor.getOutputLevelL(),
                    audioProcessor.getOutputLevelR(),
                    audioProcessor.getActiveOutputChannels(),
                    "OUT");

    // 3. Center Core Workspace: Top Row (Front-End NR/Tight/Boost + Amp Head) & Bottom Row (Dual Cab IR Loader + Spread)
    auto topRow = content.removeFromTop(content.getHeight() * 0.50f);
    content.removeFromTop(pad);
    auto botRow = content;

    auto frontEndPanel = topRow.removeFromLeft(topRow.getWidth() * 0.34f);
    topRow.removeFromLeft(pad);
    auto ampHeadPanel = topRow;

    draw3DModulePanel(g, frontEndPanel, "SURGICAL FRONT-END // NR + TIGHT + SUB-BOOST");
    draw3DModulePanel(g, ampHeadPanel, "ZENITH HIGH-GAIN PREAMP & POWER AMP");

    auto spreadPanel = botRow.removeFromRight(botRow.getWidth() * 0.24f);
    botRow.removeFromRight(pad);
    auto cabPanel = botRow;

    draw3DModulePanel(g, cabPanel, "DUAL CABINET IMPULSE RESPONSE LOADER (LEFT & RIGHT CABS)");
    draw3DModulePanel(g, spreadPanel, "PHASE-SAFE SPREAD");
}

void TheNoizzPassageAudioProcessorEditor::resized()
{
    const auto fullBounds = getLocalBounds().toFloat();
    const float pad = fullBounds.getWidth() * 0.012f;
    auto content = fullBounds.reduced(pad);

    // 1. Header Layout
    auto headerArea = content.removeFromTop(content.getHeight() * 0.11f).reduced(10.0f, 10.0f);
    headerArea.removeFromLeft(headerArea.getWidth() * 0.30f); // skip brand title area

    const float btnH = headerArea.getHeight() * 0.82f;
    const float btnY = headerArea.getCentreY() - btnH * 0.5f;

    // Preset bar + Save & Open buttons
    const float presetW = headerArea.getWidth() * 0.36f;
    presetCombo.setBounds(int(headerArea.getX()), int(btnY), int(presetW), int(btnH));
    float curX = headerArea.getX() + presetW + 6.0f;

    savePresetBtn.setBounds(int(curX), int(btnY), 54, int(btnH));
    curX += 58.0f;
    loadPresetBtn.setBounds(int(curX), int(btnY), 54, int(btnH));
    curX += 66.0f;

    // Undo / Redo buttons
    undoBtn.setBounds(int(curX), int(btnY), 54, int(btnH));
    curX += 58.0f;
    redoBtn.setBounds(int(curX), int(btnY), 54, int(btnH));
    curX += 66.0f;

    // A / B / Copy comparison buttons
    abButtonA.setBounds(int(curX), int(btnY), 34, int(btnH));
    curX += 38.0f;
    abButtonB.setBounds(int(curX), int(btnY), 34, int(btnH));
    curX += 38.0f;
    abCopyBtn.setBounds(int(curX), int(btnY), 52, int(btnH));
    curX += 60.0f;

    // Bypass button
    bypassBtn.setBounds(int(curX), int(btnY), int(headerArea.getRight() - curX), int(btnH));

    content.removeFromTop(pad);

    // 2. Left & Right I/O Strips (Sliders occupy bottom 40% below meters)
    const float ioStripW = content.getWidth() * 0.105f;
    auto leftStrip  = content.removeFromLeft(ioStripW);
    content.removeFromLeft(pad);
    auto rightStrip = content.removeFromRight(ioStripW);
    content.removeFromRight(pad);

    auto inSliderArea = leftStrip.removeFromBottom(leftStrip.getHeight() * 0.40f).reduced(6.0f, 6.0f);
    inputGainLabel.setBounds(inSliderArea.removeFromTop(18.0f).toNearestInt());
    inputGainSlider.setBounds(inSliderArea.toNearestInt());

    auto outSliderArea = rightStrip.removeFromBottom(rightStrip.getHeight() * 0.40f).reduced(6.0f, 6.0f);
    outputGainLabel.setBounds(outSliderArea.removeFromTop(18.0f).toNearestInt());
    outputGainSlider.setBounds(outSliderArea.toNearestInt());

    // 3. Center Core Workspace
    auto topRow = content.removeFromTop(content.getHeight() * 0.50f);
    content.removeFromTop(pad);
    auto botRow = content;

    auto frontEndPanel = topRow.removeFromLeft(topRow.getWidth() * 0.34f).withTrimmedTop(30.0f).reduced(10.0f, 8.0f);
    topRow.removeFromLeft(pad);
    auto ampHeadPanel  = topRow.withTrimmedTop(30.0f).reduced(10.0f, 8.0f);

    // Front-End Layout: NR Knob | TIGHT Button | BOOST Knob
    const float feColW = frontEndPanel.getWidth() / 3.0f;
    auto nrArea = frontEndPanel.removeFromLeft(feColW).reduced(4.0f);
    nrLabel.setBounds(nrArea.removeFromTop(20.0f).toNearestInt());
    nrKnob.setBounds(nrArea.toNearestInt());

    auto tightArea = frontEndPanel.removeFromLeft(feColW).reduced(8.0f);
    tightButton.setBounds(tightArea.withSizeKeepingCentre(tightArea.getWidth(), 54.0f).toNearestInt());

    auto boostArea = frontEndPanel.reduced(4.0f);
    boostLabel.setBounds(boostArea.removeFromTop(20.0f).toNearestInt());
    boostKnob.setBounds(boostArea.toNearestInt());

    // Amp Head Layout: 6 Knobs (GAIN, BASS, MID, TREBLE, PRESENCE, DEPTH)
    const float ampColW = ampHeadPanel.getWidth() / 6.0f;
    juce::Slider* ampKnobs[6] = { &gainKnob, &bassKnob, &midKnob, &trebleKnob, &presenceKnob, &depthKnob };
    juce::Label*  ampLabels[6] = { &gainLabel, &bassLabel, &midLabel, &trebleLabel, &presenceLabel, &depthLabel };
    for (int i = 0; i < 6; ++i)
    {
        auto col = ampHeadPanel.removeFromLeft(ampColW).reduced(3.0f);
        ampLabels[i]->setBounds(col.removeFromTop(20.0f).toNearestInt());
        ampKnobs[i]->setBounds(col.toNearestInt());
    }

    // Bottom Row: Dual Cab IR Loader + Spread Panel
    auto spreadPanel = botRow.removeFromRight(botRow.getWidth() * 0.24f).withTrimmedTop(30.0f).reduced(10.0f, 8.0f);
    botRow.removeFromRight(pad);
    auto cabPanel = botRow.withTrimmedTop(30.0f).reduced(10.0f, 8.0f);

    // Cab Mix Slider across bottom of Cab Panel
    auto mixRow = cabPanel.removeFromBottom(44.0f);
    cabMixLabel.setBounds(mixRow.removeFromTop(16.0f).toNearestInt());
    cabMixSlider.setBounds(mixRow.reduced(40.0f, 0.0f).toNearestInt());

    const float halfCabW = (cabPanel.getWidth() - 12.0f) * 0.5f;
    auto cabLArea = cabPanel.removeFromLeft(halfCabW);
    cabPanel.removeFromLeft(12.0f);
    auto cabRArea = cabPanel;

    auto layoutCabUnit = [](juce::Rectangle<float> area,
                            juce::TextButton& onBtn,
                            juce::ComboBox& irCombo,
                            juce::TextButton& loadBtn,
                            juce::Slider& volKnob, juce::Label& volLbl,
                            juce::Slider& panKnob, juce::Label& panLbl)
    {
        auto barRow = area.removeFromTop(28.0f);
        onBtn.setBounds(barRow.removeFromLeft(78.0f).toNearestInt());
        barRow.removeFromLeft(5.0f);
        loadBtn.setBounds(barRow.removeFromRight(76.0f).toNearestInt());
        barRow.removeFromRight(5.0f);
        irCombo.setBounds(barRow.toNearestInt());

        area.removeFromTop(6.0f);
        const float halfW = area.getWidth() * 0.5f;
        auto vCol = area.removeFromLeft(halfW).reduced(4.0f);
        volLbl.setBounds(vCol.removeFromTop(18.0f).toNearestInt());
        volKnob.setBounds(vCol.toNearestInt());

        auto pCol = area.reduced(4.0f);
        panLbl.setBounds(pCol.removeFromTop(18.0f).toNearestInt());
        panKnob.setBounds(pCol.toNearestInt());
    };

    layoutCabUnit(cabLArea, cabLOnBtn, cabLIRCombo, cabLLoadBtn, cabLVolKnob, cabLVolLabel, cabLPanKnob, cabLPanLabel);
    layoutCabUnit(cabRArea, cabROnBtn, cabRIRCombo, cabRLoadBtn, cabRVolKnob, cabRVolLabel, cabRPanKnob, cabRPanLabel);

    // Spread Knob Layout
    spreadLabel.setBounds(spreadPanel.removeFromTop(22.0f).toNearestInt());
    spreadKnob.setBounds(spreadPanel.reduced(8.0f).toNearestInt());
}
