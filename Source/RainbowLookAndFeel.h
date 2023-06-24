/*
  ==============================================================================

    RainbowLookAndFeel.h
    Created: 10 Apr 2021 3:07:42pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils.h"
#include "Parameters.h"
#include "ff_meters/ff_meters.h"

// Enables more than one slider value to be displayed at once in a rainbow fashion
class RainbowSlider : public juce::Slider {
 public:
  RainbowSlider(Parameters& parameters, ParamCommon::Type type) : juce::Slider(), mType(type), mParameters(parameters) {
    // Knob params
    auto rotaryParams = juce::Slider::RotaryParameters();
    rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
    rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
    rotaryParams.stopAtEnd = true;
    setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    setRotaryParameters(rotaryParams);
    setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Utils::GLOBAL_COLOUR);
    setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::GLOBAL_COLOUR);
    onValueChange = [this] {
      ParamHelper::setCommonParam(mParameters.selectedParams, mType, (float)getValue());
      if (mParameters.selectedParams->type == ParamType::NOTE) {
        Utils::PitchClass pitchClass = (Utils::PitchClass) dynamic_cast<ParamNote*>(mParameters.selectedParams)->noteIdx;
        float posNorm = juce::jmap(getValue(), getMinimum(), getMaximum(), 0.0, 1.0);
        mArcs.set(pitchClass, posNorm);
      }
    };
  }

  // Update slider colours for new selected group
  void updateSelectedParams() {
    setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, mParameters.getSelectedParamColour());
  }

  friend class RainbowLookAndFeel;

 private:
  ParamCommon::Type mType;
  Parameters& mParameters;

  juce::HashMap<Utils::PitchClass, float> mArcs;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowSlider)
};

class RainbowLookAndFeel : public juce::LookAndFeel_V4 {
 public:
  RainbowLookAndFeel() {}

 private:
  void drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPosProportional, float, float,
                        juce::Slider& slider) override {
    // Get RainbowSlider version of the slider
    RainbowSlider& rbSlider = dynamic_cast<RainbowSlider&>(slider);

    const float pos = juce::jmax(0.02f, sliderPosProportional);
    const float startRadians = 1.5f * juce::MathConstants<float>::pi;
    const float endRadians = startRadians + (pos * juce::MathConstants<float>::pi);
    const float startRadius = width / 4.0f;
    const float endRadius = width / 2.0f - 2;
    const float noteStripeInterval = (endRadius - startRadius) / static_cast<float>(Utils::PitchClass::COUNT);

    const juce::Point<float> center = juce::Point<float>(width / 2.0f, height);

    juce::Colour rainbowCol = slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);

    // Draw main arc
    g.setFillType(juce::ColourGradient(rainbowCol, slider.getLocalBounds().getBottomLeft().toFloat(), rainbowCol.withAlpha(0.4f),
                                       slider.getLocalBounds().getTopLeft().toFloat(), false));
    juce::Path rainbowPath = juce::Path();
    rainbowPath.addCentredArc(center.x, center.y, width / 2.7f, width / 2.7f, 0, startRadians, endRadians, true);
    g.strokePath(rainbowPath, juce::PathStrokeType(width / 4.0f));

    // Draw note-level arcs
    float curStripeRadius = startRadius;
    for (int i = 0; i < Utils::PitchClass::COUNT; ++i) {
      if (rbSlider.mArcs.contains((Utils::PitchClass)i)) {
        float stripeEndRadians = startRadians + (rbSlider.mArcs[(Utils::PitchClass)i] * juce::MathConstants<float>::pi);
        g.setColour(Utils::getRainbow12Colour(i));
        rainbowPath.clear();
        rainbowPath.addCentredArc(center.x, center.y, curStripeRadius, curStripeRadius, 0, startRadians, stripeEndRadians,
                                  true);
        g.strokePath(rainbowPath, juce::PathStrokeType(noteStripeInterval));
      }
      curStripeRadius += noteStripeInterval;
    }

    // Draw current value line on end of arc
    g.setColour(rainbowCol);
    g.drawLine(juce::Line<float>(center.getPointOnCircumference(startRadius, startRadius, endRadians),
                                 center.getPointOnCircumference(endRadius, endRadius, endRadians)),
               3.0f);

    // Draw outline arcs
    g.setColour(rainbowCol);
    rainbowPath.clear();
    rainbowPath.addCentredArc(center.x, center.y, endRadius, endRadius, 0, startRadians,
                              2.5f * juce::MathConstants<float>::pi, true);
    g.strokePath(rainbowPath, juce::PathStrokeType(3));

    // Draw text label inside arc
    juce::Rectangle<int> textRect = juce::Rectangle<int>(0, height / 2, width, height / 2);
    juce::String text =
        slider.getTextFromValue(slider.getValue()).trimCharactersAtEnd(slider.getTextValueSuffix()).trimCharactersAtEnd("0") +
        slider.getTextValueSuffix();
    if (text.getLastCharacter() == '.') text += "0";
    g.setColour(juce::Colours::black);
    g.drawFittedText(text, textRect, juce::Justification::centredBottom, 1);
  }

  void drawLinearSlider(juce::Graphics& g, int, int, int width, int height, float sliderPos, float, float,
                        const juce::Slider::SliderStyle, juce::Slider& slider) override {
    // Get RainbowSlider version of the slider
    RainbowSlider& rbSlider = dynamic_cast<RainbowSlider&>(slider);
    juce::Colour rainbowCol = slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);

    // Draw Current value bar
    g.setColour(rainbowCol.withAlpha(0.4f));
    g.fillRect(0, 0, (int)sliderPos, height);

    // Draw note-level lines
    const float noteStripeInterval = (float)(height - 4) / (float)Utils::PitchClass::COUNT;
    float curStripeY = static_cast<float>(height - 2);
    for (int i = 0; i < Utils::PitchClass::COUNT; ++i) {
      if (rbSlider.mArcs.contains((Utils::PitchClass)i)) {
        float stripeX = rbSlider.mArcs[(Utils::PitchClass)i] * width;
        g.setColour(Utils::getRainbow12Colour(i));
        g.drawLine(0, curStripeY, stripeX, curStripeY, noteStripeInterval);
      }
      curStripeY -= noteStripeInterval;
    }

    // Vertical line at position
    g.setColour(rainbowCol);
    g.drawLine(sliderPos, 0, sliderPos, (float)height, 2);

    // Outline
    g.setColour(rainbowCol);
    g.drawRect(1, 1, width, height, 2);
  }

  void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted, bool) override {
    juce::Colour fillColour = btn.findColour(juce::ToggleButton::ColourIds::tickColourId);
    g.setColour(fillColour);
    g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT, 2.0f);

    juce::Colour btnColour = btn.getToggleState() ? fillColour : juce::Colours::black;
    if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
      btnColour = fillColour.interpolatedWith(juce::Colours::black, 0.8);
    }
    g.setColour(btnColour);
    g.fillRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT);
  }
  
  void drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour&, bool shouldDrawButtonAsHighlighted, bool) override {
    juce::Colour btnColour = btn.getToggleState() ? btn.findColour(juce::TextButton::ColourIds::buttonOnColourId) : btn.findColour(juce::TextButton::ColourIds::buttonColourId);
    if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
      btnColour = btnColour.interpolatedWith(juce::Colours::black, 0.8);
    }
    g.setColour(btnColour);
    g.fillRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT);
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT, 2.0f);
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowLookAndFeel)
};

class MeterLookAndFeel : public foleys::LevelMeterLookAndFeel {
 public:
  MeterLookAndFeel() {}

 private:
  // Disables meter tick marks
  juce::Rectangle<float> getMeterTickmarksBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Disables meter clip light
  juce::Rectangle<float> getMeterClipIndicatorBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Disable meter max number
  juce::Rectangle<float> getMeterMaxNumberBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Just use the regular bounds, no margins
  juce::Rectangle<float> getMeterBarBounds(juce::Rectangle<float> bounds, foleys::LevelMeter::MeterFlags) const override {
    return bounds;
  }
};
