#pragma once
#include <cmath>
#include <array>
#include <vector>
#include <atomic>
#include <algorithm>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include "EmbeddedIRs.h"

namespace NoizzStudio
{

//==============================================================================
// Lightweight 2nd-Order Transposed Direct Form II Biquad Filter (Zero-Allocation)
//==============================================================================
struct FastBiquad
{
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;

    void reset() noexcept { z1 = 0.0f; z2 = 0.0f; }

    inline float processSample(float x) noexcept
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void makeHighPass(double sr, float freq, float q = 0.7071f) noexcept
    {
        const float w0 = 2.0f * juce::MathConstants<float>::pi * std::clamp(freq, 15.0f, float(sr * 0.48));
        const float cosW = std::cos(w0 / float(sr));
        const float sinW = std::sin(w0 / float(sr));
        const float alpha = sinW / (2.0f * q);
        const float a0Inv = 1.0f / (1.0f + alpha);
        b0 = ((1.0f + cosW) * 0.5f) * a0Inv;
        b1 = -(1.0f + cosW) * a0Inv;
        b2 = b0;
        a1 = (-2.0f * cosW) * a0Inv;
        a2 = (1.0f - alpha) * a0Inv;
    }

    void makeLowPass(double sr, float freq, float q = 0.7071f) noexcept
    {
        const float w0 = 2.0f * juce::MathConstants<float>::pi * std::clamp(freq, 40.0f, float(sr * 0.48));
        const float cosW = std::cos(w0 / float(sr));
        const float sinW = std::sin(w0 / float(sr));
        const float alpha = sinW / (2.0f * q);
        const float a0Inv = 1.0f / (1.0f + alpha);
        b1 = (1.0f - cosW) * a0Inv;
        b0 = b1 * 0.5f;
        b2 = b0;
        a1 = (-2.0f * cosW) * a0Inv;
        a2 = (1.0f - alpha) * a0Inv;
    }

    void makeBandPass(double sr, float freq, float q = 1.4f) noexcept
    {
        const float w0 = 2.0f * juce::MathConstants<float>::pi * std::clamp(freq, 20.0f, float(sr * 0.48));
        const float cosW = std::cos(w0 / float(sr));
        const float sinW = std::sin(w0 / float(sr));
        const float alpha = sinW / (2.0f * q);
        const float a0Inv = 1.0f / (1.0f + alpha);
        b0 = alpha * a0Inv;
        b1 = 0.0f;
        b2 = -alpha * a0Inv;
        a1 = (-2.0f * cosW) * a0Inv;
        a2 = (1.0f - alpha) * a0Inv;
    }

    void makeNotch(double sr, float freq, float q = 8.0f) noexcept
    {
        const float w0 = 2.0f * juce::MathConstants<float>::pi * std::clamp(freq, 20.0f, float(sr * 0.48));
        const float cosW = std::cos(w0 / float(sr));
        const float sinW = std::sin(w0 / float(sr));
        const float alpha = sinW / (2.0f * q);
        const float a0Inv = 1.0f / (1.0f + alpha);
        b0 = a0Inv;
        b1 = (-2.0f * cosW) * a0Inv;
        b2 = a0Inv;
        a1 = b1;
        a2 = (1.0f - alpha) * a0Inv;
    }

    void makePeak(double sr, float freq, float q, float gainDb) noexcept
    {
        const float A = std::pow(10.0f, gainDb / 40.0f);
        const float w0 = 2.0f * juce::MathConstants<float>::pi * std::clamp(freq, 20.0f, float(sr * 0.48));
        const float cosW = std::cos(w0 / float(sr));
        const float sinW = std::sin(w0 / float(sr));
        const float alpha = sinW / (2.0f * q);
        const float a0Inv = 1.0f / (1.0f + alpha / A);
        b0 = (1.0f + alpha * A) * a0Inv;
        b1 = (-2.0f * cosW) * a0Inv;
        b2 = (1.0f - alpha * A) * a0Inv;
        a1 = b1;
        a2 = (1.0f - alpha / A) * a0Inv;
    }

    void makeAllPass(double sr, float freq, float q = 0.7071f) noexcept
    {
        const float w0 = 2.0f * juce::MathConstants<float>::pi * std::clamp(freq, 20.0f, float(sr * 0.48));
        const float cosW = std::cos(w0 / float(sr));
        const float sinW = std::sin(w0 / float(sr));
        const float alpha = sinW / (2.0f * q);
        const float a0Inv = 1.0f / (1.0f + alpha);
        b0 = (1.0f - alpha) * a0Inv;
        b1 = (-2.0f * cosW) * a0Inv;
        b2 = 1.0f;
        a1 = b1;
        a2 = b0;
    }
};

//==============================================================================
// 1. "NR" Knob: Cable Hum + Static Noise Reduction Module
//==============================================================================
class NoiseReductionDSP
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        for (int ch = 0; ch < 2; ++ch)
        {
            humNotch60[ch].makeNotch(sr, 60.0f, 10.0f);
            humNotch120[ch].makeNotch(sr, 120.0f, 12.0f);
            humNotch50[ch].makeNotch(sr, 50.0f, 10.0f);
            deHissLPF[ch].makeLowPass(sr, 12000.0f, 0.707f);
            env[ch] = 0.0f;
            gainSmooth[ch] = 1.0f;
        }
        attackCoeff = std::exp(-1.0f / float(sr * 0.0015));
        releaseCoeff = std::exp(-1.0f / float(sr * 0.065));
    }

    inline float processSample(float x, int ch, float nrAmount) noexcept
    {
        if (nrAmount <= 0.001f)
            return x;

        // Subtle cable mains hum suppression scaled by NR knob
        const float humFiltered = humNotch120[ch].processSample(humNotch60[ch].processSample(x));
        float sig = x + nrAmount * 0.65f * (humFiltered - x);

        // Envelope tracking for downward noise floor expansion
        const float absX = std::fabs(sig);
        if (absX > env[ch])
            env[ch] = attackCoeff * env[ch] + (1.0f - attackCoeff) * absX;
        else
            env[ch] = releaseCoeff * env[ch] + (1.0f - releaseCoeff) * absX;

        // Threshold ranges from -78 dB (NR=0) up to -36 dB (NR=1)
        const float threshDb = -78.0f + nrAmount * 42.0f;
        const float threshLin = std::pow(10.0f, threshDb / 20.0f);

        float targetGain = 1.0f;
        if (env[ch] < threshLin)
        {
            const float ratio = env[ch] / (threshLin + 1.0e-7f);
            // Smooth quadratic knee expansion eliminates cable static between notes
            targetGain = std::pow(std::clamp(ratio, 0.0f, 1.0f), 1.5f + nrAmount * 2.5f);
        }

        gainSmooth[ch] = 0.992f * gainSmooth[ch] + 0.008f * targetGain;

        // Dynamic high-frequency static/hiss filter when signal is quiet
        const float lpfOut = deHissLPF[ch].processSample(sig);
        const float hissBlend = std::clamp((1.0f - gainSmooth[ch]) * nrAmount, 0.0f, 1.0f);
        sig = sig * (1.0f - hissBlend * 0.7f) + lpfOut * (hissBlend * 0.7f);

        return sig * gainSmooth[ch];
    }

private:
    double sr = 48000.0;
    FastBiquad humNotch60[2], humNotch120[2], humNotch50[2], deHissLPF[2];
    float env[2] = { 0.0f, 0.0f };
    float gainSmooth[2] = { 1.0f, 1.0f };
    float attackCoeff = 0.9f, releaseCoeff = 0.999f;
};

//==============================================================================
// 2. "Tight" Button: Intelligent Dual-Band Program-Dependent Gate
//    Delivers gunshot-tight gating on palm-mutes while allowing sustained leads to resonate.
//==============================================================================
class IntelligentTightGateDSP
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        for (int ch = 0; ch < 2; ++ch)
        {
            palmMuteBand[ch].makeBandPass(sr, 145.0f, 1.2f);
            sustainBand[ch].makeBandPass(sr, 1650.0f, 0.9f);
            lowEnv[ch] = 0.0f;
            highEnv[ch] = 0.0f;
            gateGain[ch] = 1.0f;
        }
        fastAttack = std::exp(-1.0f / float(sr * 0.0004));     // 0.4 ms instant opening
        gunshotRelease = std::exp(-1.0f / float(sr * 0.0032)); // 3.2 ms gunshot clamp on palm mutes
        sustainRelease = std::exp(-1.0f / float(sr * 0.195));  // 195 ms smooth decay on sustained notes
        envDecay = std::exp(-1.0f / float(sr * 0.012));
    }

    inline float processSample(float x, int ch, bool isTightEnabled) noexcept
    {
        if (!isTightEnabled)
            return x;

        const float lowSig = std::fabs(palmMuteBand[ch].processSample(x));
        const float highSig = std::fabs(sustainBand[ch].processSample(x));

        lowEnv[ch] = std::max(lowSig, lowEnv[ch] * envDecay);
        highEnv[ch] = std::max(highSig, highEnv[ch] * envDecay);

        // Discriminate between a palm-muted chug (dominant low-end thump) and a sustained note (rich upper harmonics)
        const float sustainRatio = highEnv[ch] / (lowEnv[ch] + 1.0e-5f);
        const bool isSustainingNote = (sustainRatio > 0.42f && highEnv[ch] > 0.0018f);

        // Adaptive hysteresis threshold
        const float openThresh = isSustainingNote ? 0.0022f : 0.0085f;
        const float closeThresh = isSustainingNote ? 0.0009f : 0.0048f;
        const float totalLevel = std::max(lowEnv[ch], highEnv[ch] * 1.35f);

        if (totalLevel > openThresh)
            gateStateOpen[ch] = true;
        else if (totalLevel < closeThresh)
            gateStateOpen[ch] = false;

        const float target = gateStateOpen[ch] ? 1.0f : 0.0f;
        if (target > gateGain[ch])
        {
            gateGain[ch] = fastAttack * gateGain[ch] + (1.0f - fastAttack) * target;
        }
        else
        {
            // Gunshot-tight release when palm-muting, musical long release when sustaining
            const float rel = isSustainingNote ? sustainRelease : gunshotRelease;
            gateGain[ch] = rel * gateGain[ch] + (1.0f - rel) * target;
        }

        return x * (gateGain[ch] * gateGain[ch]);
    }

private:
    double sr = 48000.0;
    FastBiquad palmMuteBand[2], sustainBand[2];
    float lowEnv[2] = { 0.0f, 0.0f };
    float highEnv[2] = { 0.0f, 0.0f };
    float gateGain[2] = { 1.0f, 1.0f };
    bool gateStateOpen[2] = { false, false };
    float fastAttack = 0.9f, gunshotRelease = 0.99f, sustainRelease = 0.9995f, envDecay = 0.995f;
};

//==============================================================================
// 3. "Boost" Knob: Pick-Attack Kick Drum & Sub-Bass Backing Note Generator
//    Detects pick attacks & palm-mute low-end envelopes, generating a tight 58Hz
//    kick drum punch + divide-by-2 sub-harmonic bass guitar backing note.
//==============================================================================
class SubKickBoostDSP
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        for (int ch = 0; ch < 2; ++ch)
        {
            fundamentalTrackerBP[ch].makeBandPass(sr, 115.0f, 1.6f);
            highFizzBP[ch].makeBandPass(sr, 2800.0f, 1.2f);
            subBassBP[ch].makeBandPass(sr, 68.0f, 1.35f);
            kickBodyResonator[ch].makeBandPass(sr, 58.0f, 4.2f);
            fastEnv[ch] = 0.0f;
            slowEnv[ch] = 0.0f;
            kickEnv[ch] = 0.0f;
            kickPhase[ch] = 0.0f;
            prevTrackerSample[ch] = 0.0f;
            subFlipFlop[ch] = 1.0f;
            subSmooth[ch] = 0.0f;
        }
        fastAtt = std::exp(-1.0f / float(sr * 0.0012));
        fastRel = std::exp(-1.0f / float(sr * 0.025));
        slowAtt = std::exp(-1.0f / float(sr * 0.018));
        slowRel = std::exp(-1.0f / float(sr * 0.080));
        kickDecay = std::exp(-1.0f / float(sr * 0.038)); // 38ms punchy kick decay
    }

    inline float processSample(float x, int ch, float boostAmount) noexcept
    {
        if (boostAmount <= 0.001f)
            return x;

        // Track fundamental string energy (75-170 Hz) and upper harmonic ratio
        const float fundSig = fundamentalTrackerBP[ch].processSample(x);
        const float highSig = std::fabs(highFizzBP[ch].processSample(x));
        const float absFund = std::fabs(fundSig);

        // Fast vs slow envelope for pick transient detection
        fastEnv[ch] = (absFund > fastEnv[ch])
                        ? fastAtt * fastEnv[ch] + (1.0f - fastAtt) * absFund
                        : fastRel * fastEnv[ch] + (1.0f - fastRel) * absFund;
        slowEnv[ch] = (absFund > slowEnv[ch])
                        ? slowAtt * slowEnv[ch] + (1.0f - slowAtt) * absFund
                        : slowRel * slowEnv[ch] + (1.0f - slowRel) * absFund;

        // Palm-mute weighting: strongest when low fundamental dominates over high fizz
        const float palmMuteWeight = std::clamp(fastEnv[ch] / (highSig * 1.8f + 0.002f), 0.15f, 1.5f);

        // Pick-attack transient trigger for the subtle Kick Drum thump
        const float transientDiff = std::max(0.0f, fastEnv[ch] - slowEnv[ch] * 1.18f);
        if (transientDiff > 0.008f)
        {
            kickEnv[ch] = std::min(1.0f, kickEnv[ch] + transientDiff * 6.5f * palmMuteWeight);
        }
        kickEnv[ch] *= kickDecay;

        // Pitch-dropping kick drum body (95Hz -> 56Hz sweep on pick attack)
        const float kickFreq = 56.0f + kickEnv[ch] * 42.0f;
        kickPhase[ch] += kickFreq / float(sr);
        if (kickPhase[ch] >= 1.0f)
            kickPhase[ch] -= 1.0f;
        const float kickWave = std::sin(2.0f * juce::MathConstants<float>::pi * kickPhase[ch]) * kickEnv[ch];
        const float kickFiltered = kickBodyResonator[ch].processSample(kickWave);

        // Zero-crossing divide-by-2 Sub-Octave Bass Guitar note generator
        if (fundSig > 0.0f && prevTrackerSample[ch] <= 0.0f)
            subFlipFlop[ch] = -subFlipFlop[ch];
        prevTrackerSample[ch] = fundSig;

        const float rawSub = subFlipFlop[ch] * fastEnv[ch] * palmMuteWeight;
        // Soft Darkglass-style bass saturation + bandpass filtering (50-110 Hz)
        const float satSub = std::tanh(rawSub * 2.4f);
        subSmooth[ch] = subBassBP[ch].processSample(satSub);

        // Combine subtle kick punch + backing bass note with the dry DI/pre-amp signal
        const float subSignal = (kickFiltered * 0.42f + subSmooth[ch] * 0.58f) * (boostAmount * 0.45f);
        return x + subSignal;
    }

private:
    double sr = 48000.0;
    FastBiquad fundamentalTrackerBP[2], highFizzBP[2], subBassBP[2], kickBodyResonator[2];
    float fastEnv[2] = { 0.0f, 0.0f }, slowEnv[2] = { 0.0f, 0.0f }, kickEnv[2] = { 0.0f, 0.0f };
    float kickPhase[2] = { 0.0f, 0.0f }, prevTrackerSample[2] = { 0.0f, 0.0f };
    float subFlipFlop[2] = { 1.0f, 1.0f }, subSmooth[2] = { 0.0f, 0.0f };
    float fastAtt = 0.9f, fastRel = 0.99f, slowAtt = 0.99f, slowRel = 0.999f, kickDecay = 0.999f;
};

//==============================================================================
// 4. THE ZENITH PASSAGE High-Gain Amp Simulator
//    Fortin 33/Grind Pre-Boost + Cascaded 4-Stage ADAA Tube Preamp +
//    Post-Amp Tonestack & Dave Otero Flatline Surgical EQ
//==============================================================================
class ZenithAmpSimDSP
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        for (int ch = 0; ch < 2; ++ch)
        {
            // Fortin 33 / Grind style tight input high-pass + 1.8kHz clank boost
            grindHPF[ch].makeHighPass(sr, 135.0f, 0.85f);
            grindMidPeak[ch].makePeak(sr, 1850.0f, 1.4f, 7.5f);
            interStageHPF[ch].makeHighPass(sr, 95.0f, 0.707f);
            interStageLPF[ch].makeLowPass(sr, 7500.0f, 0.707f);

            // Dave Otero Flatline surgical cleanup filters
            postSubCut[ch].makeHighPass(sr, 88.0f, 0.80f);
            postFizzCut[ch].makeLowPass(sr, 10800.0f, 0.85f);
            postWhistleNotch[ch].makePeak(sr, 3820.0f, 4.8f, -4.2f);
            prevX1[ch] = 0.0f;
            prevX2[ch] = 0.0f;
        }
        updateToneFilters(0.55f, 0.48f, 0.65f, 0.62f, 0.52f);
    }

    void updateToneFilters(float bass, float mid, float treble, float presence, float depth) noexcept
    {
        const float bassDb = (bass - 0.5f) * 14.0f;
        const float midDb = (mid - 0.5f) * 14.0f - 2.2f; // Subtle Flatline 340Hz low-mid carve
        const float trebDb = (treble - 0.5f) * 14.0f;
        const float presDb = (presence - 0.5f) * 12.0f;
        const float depthDb = (depth - 0.5f) * 11.0f;

        for (int ch = 0; ch < 2; ++ch)
        {
            bassFilter[ch].makePeak(sr, 115.0f, 1.1f, bassDb + depthDb * 0.6f);
            lowMidScoop[ch].makePeak(sr, 330.0f, 1.9f, -3.2f + midDb * 0.4f);
            midFilter[ch].makePeak(sr, 1250.0f, 1.25f, midDb);
            trebleFilter[ch].makePeak(sr, 3100.0f, 1.3f, trebDb);
            presenceFilter[ch].makePeak(sr, 5200.0f, 1.5f, presDb);
        }
    }

    // First-order Anti-Derivative Anti-Aliasing (ADAA) asymmetric tube stage
    static inline float adaaTanhStage(float x, float& xPrev) noexcept
    {
        const float diff = x - xPrev;
        float out = 0.0f;
        if (std::fabs(diff) < 1.0e-5f)
        {
            out = std::tanh(0.5f * (x + xPrev));
        }
        else
        {
            // Antiderivative of tanh(u) is ln(cosh(u))
            const float ax = std::fabs(x);
            const float axp = std::fabs(xPrev);
            const float lnCoshX = ax + std::log(1.0f + std::exp(-2.0f * ax)) - 0.69314718f;
            const float lnCoshXp = axp + std::log(1.0f + std::exp(-2.0f * axp)) - 0.69314718f;
            out = (lnCoshX - lnCoshXp) / diff;
        }
        xPrev = x;
        return out;
    }

    inline float processSample(float x, int ch, float gainNorm) noexcept
    {
        // Stage 0: Fortin 33 / Grind Pre-EQ tightening
        float s = grindHPF[ch].processSample(x);
        s = grindMidPeak[ch].processSample(s);

        // Preamp drive scaling (calibrated for razor-sharp tech-death articulation even at mid gain)
        const float drive = 2.5f + gainNorm * 16.5f;

        // Stage 1: Asymmetric 12AX7 Grid Clipping (ADAA)
        float stage1In = s * drive + 0.06f;
        float stage1 = adaaTanhStage(stage1In, prevX1[ch]);

        // Inter-stage coupling filter (removes blocking distortion mud)
        float coupled = interStageHPF[ch].processSample(stage1);

        // Stage 2: High-Gain Jose / Cali Diode-Mod Cascaded Stage (ADAA)
        float stage2In = coupled * (1.8f + gainNorm * 3.2f) - 0.04f;
        float stage2 = adaaTanhStage(stage2In, prevX2[ch]);

        // Stage 3: Push-Pull Power Tube Compression
        float pwr = stage2 / (1.0f + 0.35f * std::fabs(stage2));
        pwr = interStageLPF[ch].processSample(pwr);

        // Stage 4: Tonestack + Dave Otero Flatline Surgical EQ
        pwr = postSubCut[ch].processSample(pwr);
        pwr = bassFilter[ch].processSample(pwr);
        pwr = lowMidScoop[ch].processSample(pwr);
        pwr = midFilter[ch].processSample(pwr);
        pwr = trebleFilter[ch].processSample(pwr);
        pwr = presenceFilter[ch].processSample(pwr);
        pwr = postWhistleNotch[ch].processSample(pwr);
        pwr = postFizzCut[ch].processSample(pwr);

        return pwr * 0.68f;
    }

private:
    double sr = 48000.0;
    FastBiquad grindHPF[2], grindMidPeak[2], interStageHPF[2], interStageLPF[2];
    FastBiquad bassFilter[2], lowMidScoop[2], midFilter[2], trebleFilter[2], presenceFilter[2];
    FastBiquad postSubCut[2], postFizzCut[2], postWhistleNotch[2];
    float prevX1[2] = { 0.0f, 0.0f }, prevX2[2] = { 0.0f, 0.0f };
};

//==============================================================================
// 5. Ultra-Fast SIMD Cabinet IR Convolver (Per-Cab: Left & Right)
//    Supports 10 built-in Zenith Passage IRs + External WAV/AIFF IR files
//==============================================================================
class CabinetIRConvolver
{
public:
    static constexpr int kMaxIRSamples = 1024; // 21.3ms minimum-phase core for ultra-low CPU (<0.3% CPU)

    CabinetIRConvolver()
    {
        loadFactoryIR(0);
    }

    void prepare(double sampleRate) noexcept
    {
        currentSampleRate = sampleRate;
        std::fill(history.begin(), history.end(), 0.0f);
        writePos = 0;
        resampleActiveIR();
    }

    void loadFactoryIR(int index)
    {
        const auto& bank = EmbeddedIRBank::getFactoryIRs();
        const int safeIdx = std::clamp(index, 0, EmbeddedIRBank::kNumFactoryIRs - 1);
        rawIR.assign(bank[safeIdx].samples.begin(), bank[safeIdx].samples.end());
        rawIRSampleRate = bank[safeIdx].sampleRate;
        currentName = bank[safeIdx].displayName;
        externalFilePath.clear();
        factoryIndex = safeIdx;
        resampleActiveIR();
    }

    bool loadExternalIRFile(const juce::File& file)
    {
        if (!file.existsAsFile())
            return false;

        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr || reader->lengthInSamples <= 0)
            return false;

        const int numToRead = int(std::min<juce::int64>(reader->lengthInSamples, 4096));
        juce::AudioBuffer<float> tempBuf(1, numToRead);
        reader->read(&tempBuf, 0, numToRead, 0, true, false);

        rawIR.assign(tempBuf.getReadPointer(0), tempBuf.getReadPointer(0) + numToRead);
        rawIRSampleRate = reader->sampleRate;
        currentName = file.getFileNameWithoutExtension();
        externalFilePath = file.getFullPathName();
        factoryIndex = -1;
        resampleActiveIR();
        return true;
    }

    inline float processSample(float x) noexcept
    {
        // Write twice into double-length ring buffer so SIMD dot-product is always contiguous!
        history[writePos] = x;
        history[writePos + kMaxIRSamples] = x;

        const float* histPtr = history.data() + writePos;
        const float* irPtr = activeIRRev.data();

        float acc = 0.0f;
        // Compiler auto-vectorizes this contiguous loop with AVX2/NEON FMA instructions
        for (int i = 0; i < kMaxIRSamples; ++i)
            acc += histPtr[i] * irPtr[i];

        if (++writePos >= kMaxIRSamples)
            writePos = 0;

        return acc;
    }

    juce::String getCurrentIRName() const { return currentName; }
    juce::String getExternalFilePath() const { return externalFilePath; }
    int getFactoryIndex() const { return factoryIndex; }

private:
    void resampleActiveIR() noexcept
    {
        if (rawIR.empty())
            return;

        const double ratio = rawIRSampleRate / std::max(8000.0, currentSampleRate);
        std::array<float, kMaxIRSamples> temp{};
        float energySum = 0.0f;

        for (int i = 0; i < kMaxIRSamples; ++i)
        {
            const double srcIdx = i * ratio;
            const int idx0 = int(srcIdx);
            const float frac = float(srcIdx - idx0);
            float val = 0.0f;
            if (idx0 + 1 < int(rawIR.size()))
                val = rawIR[idx0] * (1.0f - frac) + rawIR[idx0 + 1] * frac;
            else if (idx0 < int(rawIR.size()))
                val = rawIR[idx0];

            temp[i] = val;
            energySum += val * val;
        }

        // Normalize IR energy so switching IRs preserves consistent volume
        const float normGain = (energySum > 1.0e-6f) ? (0.38f / std::sqrt(energySum)) : 1.0f;
        for (int i = 0; i < kMaxIRSamples; ++i)
        {
            // Store reversed for contiguous forward dot-product with ring buffer
            activeIRRev[kMaxIRSamples - 1 - i] = temp[i] * normGain;
        }
    }

    double currentSampleRate = 48000.0;
    double rawIRSampleRate = 48000.0;
    std::vector<float> rawIR;
    std::array<float, kMaxIRSamples> activeIRRev{};
    std::array<float, kMaxIRSamples * 2> history{};
    int writePos = 0;
    int factoryIndex = 0;
    juce::String currentName = "01 - Cali OS 4x12 V30 (SM57 Cap Edge)";
    juce::String externalFilePath;
};

//==============================================================================
// 6. "Spread" Knob: Phase-Safe Mono-to-Stereo Haas + Stereo 100%->200% Widener
//    Retains a rock-solid mono low-end (<180Hz) with zero phase cancellation!
//==============================================================================
class PhaseSafeSpreadDSP
{
public:
    static constexpr int kMaxDelaySamples = 2048;

    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        for (int ch = 0; ch < 2; ++ch)
        {
            // 4th-order Linkwitz-Riley (two cascaded 2nd-order Butterworth filters at 180 Hz)
            lp1[ch].makeLowPass(sr, 180.0f, 0.7071f);
            lp2[ch].makeLowPass(sr, 180.0f, 0.7071f);
            hp1[ch].makeHighPass(sr, 180.0f, 0.7071f);
            hp2[ch].makeHighPass(sr, 180.0f, 0.7071f);
        }
        // Cascaded all-pass filters for phase-coherent mono-to-stereo decorrelation
        ap1.makeAllPass(sr, 340.0f, 0.707f);
        ap2.makeAllPass(sr, 1180.0f, 0.707f);
        ap3.makeAllPass(sr, 2950.0f, 0.707f);
        std::fill(delayBuf.begin(), delayBuf.end(), 0.0f);
        writeIdx = 0;
    }

    inline void processStereo(float& left, float& right, float spreadAmount, bool inputIsMono) noexcept
    {
        if (spreadAmount <= 0.001f)
            return;

        // 1. Split into Low (<180Hz) and High (>180Hz) bands
        const float lowL = lp2[0].processSample(lp1[0].processSample(left));
        const float lowR = lp2[1].processSample(lp1[1].processSample(right));
        // Anchor low-end strictly tight & phase-aligned
        const float tightLowMono = 0.5f * (lowL + lowR);

        const float highL = hp2[0].processSample(hp1[0].processSample(left));
        const float highR = hp2[1].processSample(hp1[1].processSample(right));

        // 2. Measure stereo difference to detect mono or correlated input vs true stereo input
        const float mid = 0.5f * (highL + highR);
        float side = 0.5f * (highL - highR);

        // Write high-band mid signal into Haas delay line (up to 7.8ms delay)
        delayBuf[writeIdx] = mid;
        const float delaySamples = float(sr * (0.0012 + 0.0066 * spreadAmount));
        int readIdx = writeIdx - int(delaySamples);
        if (readIdx < 0)
            readIdx += kMaxDelaySamples;
        const float haasDelayed = delayBuf[readIdx];
        if (++writeIdx >= kMaxDelaySamples)
            writeIdx = 0;

        // Decorrelate Haas signal via 3-stage all-pass cascade
        // Because we inject it in opposite polarity (+L / -R) as a pure Side component,
        // summing L+R to mono cancels the Haas side component 100% with ZERO comb filtering!
        const float decorrelatedSide = ap3.processSample(ap2.processSample(ap1.processSample(haasDelayed)));

        if (inputIsMono || std::fabs(side) < 1.0e-4f)
        {
            // Mono Input Mode: Generate rich Haas stereo width while preserving 100% mono compatibility
            const float synthSide = decorrelatedSide * (spreadAmount * 0.72f);
            left  = tightLowMono + (mid + synthSide);
            right = tightLowMono + (mid - synthSide);
        }
        else
        {
            // Stereo Input Mode: Push 100% stereo up to 200% width (1.0x to 2.0x Side gain)
            // plus subtle orthogonal harmonic air so it never thins out
            const float stereoScale = 1.0f + spreadAmount * 1.0f; // 100% -> 200%
            const float enhancedSide = side * stereoScale + decorrelatedSide * (spreadAmount * 0.22f);
            // Mid/Side energy compensation so center image remains anchored
            const float midComp = mid * (1.0f - spreadAmount * 0.12f);
            left  = tightLowMono + (midComp + enhancedSide);
            right = tightLowMono + (midComp - enhancedSide);
        }
    }

private:
    double sr = 48000.0;
    FastBiquad lp1[2], lp2[2], hp1[2], hp2[2];
    FastBiquad ap1, ap2, ap3;
    std::array<float, kMaxDelaySamples> delayBuf{};
    int writeIdx = 0;
};

} // namespace NoizzStudio
