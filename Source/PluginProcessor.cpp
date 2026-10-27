#include "PluginProcessor.h"
#include "PluginEditor.h"

TheNoizzPassageAudioProcessor::TheNoizzPassageAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, &undoManager, "NOIZZ_PASSAGE_PARAMS", createParameterLayout())
{
    loadFactoryPreset(0);
    abStateSnapshots[0] = apvts.copyState();
    abStateSnapshots[1] = apvts.copyState();
}

juce::AudioProcessorValueTreeState::ParameterLayout TheNoizzPassageAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Global Bypass & I/O Sliders
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "bypass", 1 }, "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "input_gain", 1 }, "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "output_gain", 1 }, "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    // Signature Front-End Controls: NR, Tight, Boost
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "nr", 1 }, "NR",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.62f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "tight", 1 }, "Tight", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "boost", 1 }, "Boost",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));

    // Core Zenith High-Gain Amp Controls
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "amp_gain", 1 }, "Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.56f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "amp_bass", 1 }, "Bass",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.52f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "amp_mid", 1 }, "Mid",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.48f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "amp_treble", 1 }, "Treble",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.70f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "amp_presence", 1 }, "Presence",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.68f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "amp_depth", 1 }, "Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.54f));

    // Dual IR Loader Controls (Left Cab & Right Cab + Mix Slider)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "cabL_on", 1 }, "Cab L On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "cabL_vol", 1 }, "Cab L Vol",
        juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "cabL_pan", 1 }, "Cab L Pan",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), -0.85f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "cabR_on", 1 }, "Cab R On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "cabR_vol", 1 }, "Cab R Vol",
        juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "cabR_pan", 1 }, "Cab R Pan",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.85f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "cab_mix", 1 }, "Cab Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));

    // Dedicated "Spread" Knob (Mono Haas & Stereo 100%->200% Phase-Safe Widener)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "spread", 1 }, "Spread",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.60f));

    return { params.begin(), params.end() };
}

bool TheNoizzPassageAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto inSet  = layouts.getMainInputChannelSet();
    const auto outSet = layouts.getMainOutputChannelSet();

    // Support Mono->Mono, Stereo->Stereo, and Mono->Stereo
    if (inSet == juce::AudioChannelSet::mono() && outSet == juce::AudioChannelSet::mono())
        return true;
    if (inSet == juce::AudioChannelSet::stereo() && outSet == juce::AudioChannelSet::stereo())
        return true;
    if (inSet == juce::AudioChannelSet::mono() && outSet == juce::AudioChannelSet::stereo())
        return true;

    return false;
}

void TheNoizzPassageAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    noiseReduction.prepare(sampleRate);
    tightGate.prepare(sampleRate);
    subKickBoost.prepare(sampleRate);
    ampSim.prepare(sampleRate);
    for (int ch = 0; ch < 2; ++ch)
    {
        cabConvolverL[ch].prepare(sampleRate);
        cabConvolverR[ch].prepare(sampleRate);
    }
    spreadProcessor.prepare(sampleRate);
}

void TheNoizzPassageAudioProcessor::releaseResources() {}

void TheNoizzPassageAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    activeInChannels.store(totalNumInputChannels, std::memory_order_relaxed);
    activeOutChannels.store(totalNumOutputChannels, std::memory_order_relaxed);

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    if (numSamples <= 0)
        return;

    // Fetch atomic parameter values once per block for minimum CPU overhead
    const bool isBypassed   = apvts.getRawParameterValue("bypass")->load() > 0.5f;
    const float inGainLin   = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("input_gain")->load());
    const float outGainLin  = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("output_gain")->load());
    const float nrVal       = apvts.getRawParameterValue("nr")->load();
    const bool tightOn      = apvts.getRawParameterValue("tight")->load() > 0.5f;
    const float boostVal    = apvts.getRawParameterValue("boost")->load();
    const float ampGain     = apvts.getRawParameterValue("amp_gain")->load();
    const float ampBass     = apvts.getRawParameterValue("amp_bass")->load();
    const float ampMid      = apvts.getRawParameterValue("amp_mid")->load();
    const float ampTreble   = apvts.getRawParameterValue("amp_treble")->load();
    const float ampPresence = apvts.getRawParameterValue("amp_presence")->load();
    const float ampDepth    = apvts.getRawParameterValue("amp_depth")->load();
    const bool cabLOn       = apvts.getRawParameterValue("cabL_on")->load() > 0.5f;
    const float cabLGain    = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("cabL_vol")->load());
    const float cabLPan     = apvts.getRawParameterValue("cabL_pan")->load();
    const bool cabROn       = apvts.getRawParameterValue("cabR_on")->load() > 0.5f;
    const float cabRGain    = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("cabR_vol")->load());
    const float cabRPan     = apvts.getRawParameterValue("cabR_pan")->load();
    const float cabMix      = apvts.getRawParameterValue("cab_mix")->load();
    const float spreadVal   = apvts.getRawParameterValue("spread")->load();

    ampSim.updateToneFilters(ampBass, ampMid, ampTreble, ampPresence, ampDepth);

    // Equal-power stereo panning coefficients for Cab L and Cab R
    const float angleL = (cabLPan + 1.0f) * (juce::MathConstants<float>::pi * 0.25f);
    const float cabL_gainL = std::cos(angleL) * cabLGain * (1.0f - cabMix) * 2.0f;
    const float cabL_gainR = std::sin(angleL) * cabLGain * (1.0f - cabMix) * 2.0f;

    const float angleR = (cabRPan + 1.0f) * (juce::MathConstants<float>::pi * 0.25f);
    const float cabR_gainL = std::cos(angleR) * cabRGain * cabMix * 2.0f;
    const float cabR_gainR = std::sin(angleR) * cabRGain * cabMix * 2.0f;

    float* ch0 = buffer.getWritePointer(0);
    float* ch1 = (totalNumOutputChannels > 1) ? buffer.getWritePointer(1) : nullptr;
    const bool inputIsMono = (totalNumInputChannels == 1);

    // If Mono->Stereo (1 in, 2 out), copy input ch0 to ch1 before processing
    if (inputIsMono && ch1 != nullptr)
        juce::FloatVectorOperations::copy(ch1, ch0, numSamples);

    float blockInPeakL = 0.0f, blockInPeakR = 0.0f;
    float blockOutPeakL = 0.0f, blockOutPeakR = 0.0f;

    const juce::SpinLock::ScopedTryLockType tryLock(irLock);

    for (int i = 0; i < numSamples; ++i)
    {
        float sL = ch0[i] * inGainLin;
        float sR = (ch1 != nullptr) ? (ch1[i] * inGainLin) : sL;

        blockInPeakL = std::max(blockInPeakL, std::fabs(sL));
        blockInPeakR = std::max(blockInPeakR, std::fabs(sR));

        if (!isBypassed)
        {
            // 1. NR (Cable Hum + Static Noise Reduction)
            sL = noiseReduction.processSample(sL, 0, nrVal);
            sR = inputIsMono ? sL : noiseReduction.processSample(sR, 1, nrVal);

            // 2. Tight (Intelligent Gunshot Gate for Palm Mutes + Long Sustain)
            sL = tightGate.processSample(sL, 0, tightOn);
            sR = inputIsMono ? sL : tightGate.processSample(sR, 1, tightOn);

            // 3. Boost (Sub-Bass Kick Drum + Backing Bass Note Generator on Pick Attack)
            sL = subKickBoost.processSample(sL, 0, boostVal);
            sR = inputIsMono ? sL : subKickBoost.processSample(sR, 1, boostVal);

            // 4. High-Gain Zenith Amp Simulator
            sL = ampSim.processSample(sL, 0, ampGain);
            sR = inputIsMono ? sL : ampSim.processSample(sR, 1, ampGain);

            // 5. Dual Cabinet IR Loader (Cab L & Cab R with Mix, Volume, Pan, On/Off)
            if (tryLock.isLocked() && (cabLOn || cabROn))
            {
                const float irOutL = cabLOn ? cabConvolverL[0].processSample(sL) : 0.0f;
                const float irOutR = cabROn ? cabConvolverR[1].processSample(sR) : 0.0f;

                if (ch1 != nullptr)
                {
                    sL = irOutL * cabL_gainL + irOutR * cabR_gainL;
                    sR = irOutL * cabL_gainR + irOutR * cabR_gainR;
                }
                else
                {
                    sL = (irOutL * cabLGain * (1.0f - cabMix) + irOutR * cabRGain * cabMix) * 1.414f;
                    sR = sL;
                }
            }

            // 6. Spread (Phase-Safe Mono Haas & Stereo 100%->200% Widener)
            if (ch1 != nullptr)
            {
                spreadProcessor.processStereo(sL, sR, spreadVal, inputIsMono);
            }
        }

        sL *= outGainLin;
        sR *= outGainLin;

        ch0[i] = sL;
        if (ch1 != nullptr)
            ch1[i] = sR;

        blockOutPeakL = std::max(blockOutPeakL, std::fabs(sL));
        blockOutPeakR = std::max(blockOutPeakR, std::fabs(sR));
    }

    // Smooth ballistic decay on meters
    const float prevInL = inPeakL.load(std::memory_order_relaxed);
    const float prevInR = inPeakR.load(std::memory_order_relaxed);
    const float prevOutL = outPeakL.load(std::memory_order_relaxed);
    const float prevOutR = outPeakR.load(std::memory_order_relaxed);

    inPeakL.store(std::max(blockInPeakL, prevInL * 0.86f), std::memory_order_relaxed);
    inPeakR.store(std::max(blockInPeakR, prevInR * 0.86f), std::memory_order_relaxed);
    outPeakL.store(std::max(blockOutPeakL, prevOutL * 0.86f), std::memory_order_relaxed);
    outPeakR.store(std::max(blockOutPeakR, prevOutR * 0.86f), std::memory_order_relaxed);
}

int TheNoizzPassageAudioProcessor::getNumPrograms()
{
    return int(NoizzStudio::FactoryPresets::getPresets().size());
}

void TheNoizzPassageAudioProcessor::setCurrentProgram(int index)
{
    loadFactoryPreset(index);
}

const juce::String TheNoizzPassageAudioProcessor::getProgramName(int index)
{
    const auto& presets = NoizzStudio::FactoryPresets::getPresets();
    if (index >= 0 && index < int(presets.size()))
        return presets[size_t(index)].name;
    return {};
}

void TheNoizzPassageAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void TheNoizzPassageAudioProcessor::loadFactoryPreset(int index)
{
    const auto& presets = NoizzStudio::FactoryPresets::getPresets();
    if (index < 0 || index >= int(presets.size()))
        return;

    currentPresetIndex = index;
    const auto& p = presets[size_t(index)];

    auto setParam = [this](const juce::String& id, float plainVal)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(plainVal));
    };

    setParam("input_gain", p.inputGainDb);
    setParam("output_gain", p.outputGainDb);
    setParam("nr", p.nr);
    setParam("tight", p.tight ? 1.0f : 0.0f);
    setParam("boost", p.boost);
    setParam("amp_gain", p.ampGain);
    setParam("amp_bass", p.ampBass);
    setParam("amp_mid", p.ampMid);
    setParam("amp_treble", p.ampTreble);
    setParam("amp_presence", p.ampPresence);
    setParam("amp_depth", p.ampDepth);
    setParam("cabL_on", p.cabLOn ? 1.0f : 0.0f);
    setParam("cabL_vol", p.cabLVolDb);
    setParam("cabL_pan", p.cabLPan);
    setParam("cabR_on", p.cabROn ? 1.0f : 0.0f);
    setParam("cabR_vol", p.cabRVolDb);
    setParam("cabR_pan", p.cabRPan);
    setParam("cab_mix", p.cabMix);
    setParam("spread", p.spread);

    selectFactoryIR(0, p.cabLIRIndex);
    selectFactoryIR(1, p.cabRIRIndex);
}

void TheNoizzPassageAudioProcessor::selectFactoryIR(int cabSide, int irIndex)
{
    const juce::SpinLock::ScopedLockType lock(irLock);
    if (cabSide == 0)
    {
        cabConvolverL[0].loadFactoryIR(irIndex);
        cabConvolverL[1].loadFactoryIR(irIndex);
    }
    else
    {
        cabConvolverR[0].loadFactoryIR(irIndex);
        cabConvolverR[1].loadFactoryIR(irIndex);
    }
}

bool TheNoizzPassageAudioProcessor::loadExternalIR(int cabSide, const juce::File& wavFile)
{
    const juce::SpinLock::ScopedLockType lock(irLock);
    if (cabSide == 0)
    {
        const bool ok0 = cabConvolverL[0].loadExternalIRFile(wavFile);
        const bool ok1 = cabConvolverL[1].loadExternalIRFile(wavFile);
        return ok0 && ok1;
    }
    else
    {
        const bool ok0 = cabConvolverR[0].loadExternalIRFile(wavFile);
        const bool ok1 = cabConvolverR[1].loadExternalIRFile(wavFile);
        return ok0 && ok1;
    }
}

juce::String TheNoizzPassageAudioProcessor::getActiveIRName(int cabSide) const
{
    return (cabSide == 0) ? cabConvolverL[0].getCurrentIRName() : cabConvolverR[0].getCurrentIRName();
}

int TheNoizzPassageAudioProcessor::getActiveFactoryIRIndex(int cabSide) const
{
    return (cabSide == 0) ? cabConvolverL[0].getFactoryIndex() : cabConvolverR[0].getFactoryIndex();
}

void TheNoizzPassageAudioProcessor::switchABState(int targetSlot)
{
    const int clamped = std::clamp(targetSlot, 0, 1);
    if (clamped == activeABSlot)
        return;

    // Save current state into current slot before switching
    abStateSnapshots[activeABSlot] = apvts.copyState();
    activeABSlot = clamped;
    if (abStateSnapshots[activeABSlot].isValid())
        apvts.replaceState(abStateSnapshots[activeABSlot].createCopy());
}

void TheNoizzPassageAudioProcessor::copyCurrentToOtherABSlot()
{
    const int other = 1 - activeABSlot;
    abStateSnapshots[other] = apvts.copyState();
}

bool TheNoizzPassageAudioProcessor::saveUserPresetToFile(const juce::File& file)
{
    auto state = apvts.copyState();
    state.setProperty("cabL_factoryIdx", getActiveFactoryIRIndex(0), nullptr);
    state.setProperty("cabR_factoryIdx", getActiveFactoryIRIndex(1), nullptr);
    state.setProperty("cabL_extPath", cabConvolverL[0].getExternalFilePath(), nullptr);
    state.setProperty("cabR_extPath", cabConvolverR[0].getExternalFilePath(), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml == nullptr)
        return false;
    return xml->writeTo(file);
}

bool TheNoizzPassageAudioProcessor::loadUserPresetFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;
    std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || !xml->hasTagName(apvts.state.getType()))
        return false;

    auto tree = juce::ValueTree::fromXml(*xml);
    apvts.replaceState(tree);

    const juce::String extL = tree.getProperty("cabL_extPath", "").toString();
    const juce::String extR = tree.getProperty("cabR_extPath", "").toString();
    if (extL.isNotEmpty() && juce::File(extL).existsAsFile())
        loadExternalIR(0, juce::File(extL));
    else
        selectFactoryIR(0, int(tree.getProperty("cabL_factoryIdx", 0)));

    if (extR.isNotEmpty() && juce::File(extR).existsAsFile())
        loadExternalIR(1, juce::File(extR));
    else
        selectFactoryIR(1, int(tree.getProperty("cabR_factoryIdx", 1)));

    return true;
}

void TheNoizzPassageAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("cabL_factoryIdx", getActiveFactoryIRIndex(0), nullptr);
    state.setProperty("cabR_factoryIdx", getActiveFactoryIRIndex(1), nullptr);
    state.setProperty("cabL_extPath", cabConvolverL[0].getExternalFilePath(), nullptr);
    state.setProperty("cabR_extPath", cabConvolverR[0].getExternalFilePath(), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TheNoizzPassageAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        auto tree = juce::ValueTree::fromXml(*xmlState);
        apvts.replaceState(tree);
        const juce::String extL = tree.getProperty("cabL_extPath", "").toString();
        const juce::String extR = tree.getProperty("cabR_extPath", "").toString();
        if (extL.isNotEmpty() && juce::File(extL).existsAsFile())
            loadExternalIR(0, juce::File(extL));
        else
            selectFactoryIR(0, int(tree.getProperty("cabL_factoryIdx", 0)));

        if (extR.isNotEmpty() && juce::File(extR).existsAsFile())
            loadExternalIR(1, juce::File(extR));
        else
            selectFactoryIR(1, int(tree.getProperty("cabR_factoryIdx", 1)));
    }
}

juce::AudioProcessorEditor* TheNoizzPassageAudioProcessor::createEditor()
{
    return new TheNoizzPassageAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheNoizzPassageAudioProcessor();
}
