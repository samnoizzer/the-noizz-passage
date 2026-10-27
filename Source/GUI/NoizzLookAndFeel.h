#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace NoizzStudio
{

class NoizzLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour kBgPitchBlack   { 0xff060709 };
    static inline const juce::Colour kPanelDarkGray  { 0xff111319 };
    static inline const juce::Colour kChassisGray    { 0xff1b1e27 };
    static inline const juce::Colour kMetallicEdge   { 0xff2c3140 };
    static inline const juce::Colour kNeonRed        { 0xffff1744 };
    static inline const juce::Colour kNeonRedBright  { 0xffff4569 };
    static inline const juce::Colour kNeonRedGlow    { 0x66ff1744 };
    static inline const juce::Colour kTraceDarkRed   { 0xff5a0c1d };
    static inline const juce::Colour kBrightText     { 0xfff5f7fa };

    NoizzLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, kNeonRed);
        setColour(juce::Slider::rotarySliderFillColourId, kNeonRed);
        setColour(juce::Slider::trackColourId, kNeonRed);
        setColour(juce::ComboBox::backgroundColourId, kPanelDarkGray);
        setColour(juce::ComboBox::textColourId, kBrightText);
        setColour(juce::ComboBox::outlineColourId, kNeonRed);
        setColour(juce::PopupMenu::backgroundColourId, kBgPitchBlack);
        setColour(juce::PopupMenu::textColourId, kBrightText);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, kNeonRed.withAlpha(0.35f));
        setColour(juce::TextButton::buttonColourId, kPanelDarkGray);
        setColour(juce::TextButton::buttonOnColourId, kNeonRed);
        setColour(juce::TextButton::textColourOffId, kBrightText);
        setColour(juce::TextButton::textColourOnId, kBrightText);
    }

    static juce::Font getBoldBrightFont(float height)
    {
        return juce::Font(juce::FontOptions(height, juce::Font::bold));
    }

    //==========================================================================
    // Futuristic 3D Metallic Rotary Knob with Neon Red PCB Ring & Glow
    //==========================================================================
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        const float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const float cx = bounds.getCentreX();
        const float cy = bounds.getCentreY();
        const float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // 1. Outer PCB Via Tick Marks
        const int numTicks = 21;
        for (int i = 0; i < numTicks; ++i)
        {
            const float norm = float(i) / float(numTicks - 1);
            const float ang = rotaryStartAngle + norm * (rotaryEndAngle - rotaryStartAngle);
            const bool active = (norm <= sliderPos + 0.001f);
            const float rIn = radius * 0.84f;
            const float rOut = radius * 0.96f;
            const float x1 = cx + std::sin(ang) * rIn;
            const float y1 = cy - std::cos(ang) * rIn;
            const float x2 = cx + std::sin(ang) * rOut;
            const float y2 = cy - std::cos(ang) * rOut;

            g.setColour(active ? kNeonRed : kMetallicEdge);
            g.drawLine(x1, y1, x2, y2, active ? 2.2f : 1.2f);
        }

        // 2. Glowing Value Arc
        const float arcRadius = radius * 0.78f;
        juce::Path bgArc;
        bgArc.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff0a0b0e));
        g.strokePath(bgArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (sliderPos > 0.001f)
        {
            juce::Path valArc;
            valArc.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
            g.setColour(kNeonRedGlow);
            g.strokePath(valArc, juce::PathStrokeType(7.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(kNeonRed);
            g.strokePath(valArc, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // 3. 3D Outer Shadow & Beveled Anodized Dark Gray Cylinder
        const float knobR = radius * 0.66f;
        g.setColour(juce::Colours::black.withAlpha(0.75f));
        g.fillEllipse(cx - knobR + 2.0f, cy - knobR + 3.5f, knobR * 2.0f, knobR * 2.0f);

        juce::ColourGradient outerBevel(juce::Colour(0xff383d4e), cx - knobR * 0.6f, cy - knobR * 0.6f,
                                        juce::Colour(0xff0b0c10), cx + knobR * 0.6f, cy + knobR * 0.6f, false);
        g.setGradientFill(outerBevel);
        g.fillEllipse(cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);

        g.setColour(kNeonRed.withAlpha(0.45f));
        g.drawEllipse(cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f, 1.2f);

        // 4. Inner 3D Concave Cap
        const float innerR = knobR * 0.78f;
        juce::ColourGradient innerCap(juce::Colour(0xff12141c), cx, cy - innerR,
                                      juce::Colour(0xff252936), cx, cy + innerR, false);
        g.setGradientFill(innerCap);
        g.fillEllipse(cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f);

        // 5. Neon Red Laser Pointer Indicator
        const float pIn = innerR * 0.22f;
        const float pOut = knobR * 0.92f;
        const float px1 = cx + std::sin(toAngle) * pIn;
        const float py1 = cy - std::cos(toAngle) * pIn;
        const float px2 = cx + std::sin(toAngle) * pOut;
        const float py2 = cy - std::cos(toAngle) * pOut;

        g.setColour(kNeonRedGlow);
        g.drawLine(px1, py1, px2, py2, 5.0f);
        g.setColour(kNeonRedBright);
        g.drawLine(px1, py1, px2, py2, 2.4f);

        // Center PCB Via Dot
        g.setColour(kNeonRed.withAlpha(0.8f));
        g.fillEllipse(cx - 2.5f, cy - 2.5f, 5.0f, 5.0f);
        juce::ignoreUnused(slider);
    }

    //==========================================================================
    // Futuristic 3D Linear Fader / Slider (Input, Output, Cab Mix)
    //==========================================================================
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        juce::ignoreUnused(minSliderPos, maxSliderPos);
        const bool isVertical = (style == juce::Slider::LinearVertical || style == juce::Slider::LinearBarVertical);

        if (isVertical)
        {
            const float trackW = 6.0f;
            const float cx = x + width * 0.5f;
            juce::Rectangle<float> track(cx - trackW * 0.5f, float(y + 6), trackW, float(height - 12));

            g.setColour(juce::Colour(0xff08090c));
            g.fillRoundedRectangle(track, 3.0f);
            g.setColour(kMetallicEdge);
            g.drawRoundedRectangle(track, 3.0f, 1.0f);

            const float fillTop = juce::jlimit(track.getY(), track.getBottom(), sliderPos);
            juce::Rectangle<float> fillRect(track.getX(), fillTop, trackW, track.getBottom() - fillTop);
            g.setColour(kNeonRedGlow);
            g.fillRoundedRectangle(fillRect.expanded(1.5f, 0.0f), 3.0f);
            g.setColour(kNeonRed);
            g.fillRoundedRectangle(fillRect, 3.0f);

            // 3D Fader Cap
            juce::Rectangle<float> thumb(cx - 11.0f, fillTop - 6.0f, 22.0f, 12.0f);
            g.setColour(juce::Colours::black.withAlpha(0.7f));
            g.fillRoundedRectangle(thumb.translated(1.0f, 2.0f), 2.5f);

            juce::ColourGradient capGrad(juce::Colour(0xff343948), thumb.getX(), thumb.getY(),
                                         juce::Colour(0xff14171f), thumb.getX(), thumb.getBottom(), false);
            g.setGradientFill(capGrad);
            g.fillRoundedRectangle(thumb, 2.5f);
            g.setColour(kNeonRed);
            g.drawRoundedRectangle(thumb, 2.5f, 1.2f);
            g.drawHorizontalLine(int(fillTop), thumb.getX() + 3.0f, thumb.getRight() - 3.0f);
        }
        else
        {
            const float trackH = 6.0f;
            const float cy = y + height * 0.5f;
            juce::Rectangle<float> track(float(x + 6), cy - trackH * 0.5f, float(width - 12), trackH);

            g.setColour(juce::Colour(0xff08090c));
            g.fillRoundedRectangle(track, 3.0f);
            g.setColour(kMetallicEdge);
            g.drawRoundedRectangle(track, 3.0f, 1.0f);

            const float fillX = juce::jlimit(track.getX(), track.getRight(), sliderPos);
            juce::Rectangle<float> fillRect(track.getX(), track.getY(), fillX - track.getX(), trackH);
            g.setColour(kNeonRed.withAlpha(0.75f));
            g.fillRoundedRectangle(fillRect, 3.0f);

            juce::Rectangle<float> thumb(fillX - 7.0f, cy - 10.0f, 14.0f, 20.0f);
            juce::ColourGradient capGrad(juce::Colour(0xff343948), thumb.getX(), thumb.getY(),
                                         juce::Colour(0xff14171f), thumb.getRight(), thumb.getBottom(), false);
            g.setGradientFill(capGrad);
            g.fillRoundedRectangle(thumb, 2.5f);
            g.setColour(kNeonRedBright);
            g.drawRoundedRectangle(thumb, 2.5f, 1.3f);
            g.drawVerticalLine(int(fillX), thumb.getY() + 3.0f, thumb.getBottom() - 3.0f);
        }
        juce::ignoreUnused(slider);
    }

    //==========================================================================
    // 3D Hardware PCB Switch Button with Bold Bright Label & Neon Red LED
    //==========================================================================
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(backgroundColour);
        auto bounds = button.getLocalBounds().toFloat().reduced(1.5f);
        const bool isOn = button.getToggleState();

        // Drop shadow
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRoundedRectangle(bounds.translated(1.0f, 2.0f), 4.0f);

        juce::Colour topCol = isOn ? juce::Colour(0xff7a091e) : juce::Colour(0xff252936);
        juce::Colour botCol = isOn ? juce::Colour(0xff38040e) : juce::Colour(0xff101218);
        if (shouldDrawButtonAsHighlighted)
            topCol = topCol.brighter(0.15f);
        if (shouldDrawButtonAsDown)
            std::swap(topCol, botCol);

        juce::ColourGradient grad(topCol, bounds.getX(), bounds.getY(),
                                  botCol, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 4.0f);

        if (isOn)
        {
            g.setColour(kNeonRedGlow);
            g.drawRoundedRectangle(bounds.expanded(1.0f), 4.5f, 2.5f);
            g.setColour(kNeonRedBright);
            g.drawRoundedRectangle(bounds, 4.0f, 1.6f);
        }
        else
        {
            g.setColour(shouldDrawButtonAsHighlighted ? kNeonRed.withAlpha(0.7f) : kMetallicEdge);
            g.drawRoundedRectangle(bounds, 4.0f, 1.2f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        g.setFont(getBoldBrightFont(std::min(14.0f, button.getHeight() * 0.48f)));
        g.setColour(button.getToggleState() ? juce::Colours::white : kBrightText);
        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(4, 2),
                         juce::Justification::centred, 1);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH, box);
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);
        juce::ColourGradient grad(juce::Colour(0xff181b24), 0.0f, 0.0f,
                                  juce::Colour(0xff0b0d12), 0.0f, float(height), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(kNeonRed.withAlpha(0.75f));
        g.drawRoundedRectangle(bounds, 4.0f, 1.3f);

        // Arrow triangle
        juce::Path arrow;
        const float ax = width - 16.0f;
        const float ay = height * 0.5f;
        arrow.addTriangle(ax - 4.5f, ay - 2.5f, ax + 4.5f, ay - 2.5f, ax, ay + 3.5f);
        g.setColour(kNeonRedBright);
        g.fillPath(arrow);
    }

    juce::Font getComboBoxFont(juce::ComboBox& box) override
    {
        return getBoldBrightFont(std::min(13.5f, box.getHeight() * 0.52f));
    }
};

} // namespace NoizzStudio
