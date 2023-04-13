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

// Enables more than one slider value to be displayed at once in a rainbow fashion
class RainbowSlider : public juce::Slider {
 public:
  RainbowSlider(Parameters& parameters, ParamCommon::Type type) : mParameters(parameters), mType(type), juce::Slider() {
    // Knob params
    auto rotaryParams = juce::Slider::RotaryParameters();
    rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
    rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
    rotaryParams.stopAtEnd = true;
    setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    setSliderStyle(juce::Slider::SliderStyle::Rotary);
    setRotaryParameters(rotaryParams);
    setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Utils::GLOBAL_COLOUR);
    setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::GLOBAL_COLOUR);
  }

 private:
  ParamCommon::Type mType;
  Parameters& mParameters;

  struct Arc {
    float value;
    Utils::PitchClass pitchClass;
  };

  std::vector<Arc> arcs;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowSlider)
};

class RainbowLookAndFeel : public juce::LookAndFeel_V4 {
 public:
  RainbowLookAndFeel() {}

 private:
  void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle,
                        float rotaryEndAngle, juce::Slider& slider) override {
    RainbowSlider& rbSlider = dynamic_cast<RainbowSlider&>(slider);
    float pos = juce::jmax(0.02f, sliderPosProportional);
    float startRadians = 1.5f * juce::MathConstants<float>::pi;
    float endRadians = startRadians + (pos * juce::MathConstants<float>::pi);
    juce::Point<float> center = juce::Point<float>(width / 2.0f, height);

    int curStripeStart = height / 2.5;
    juce::Colour rainbowCol = slider.findColour(juce::Slider::ColourIds::rotarySliderFillColourId);

    // Draw main arc
    g.setFillType(juce::ColourGradient(rainbowCol, slider.getLocalBounds().getBottomLeft().toFloat(), rainbowCol.withAlpha(0.4f),
                                       slider.getLocalBounds().getTopLeft().toFloat(), false));
    juce::Path rainbowPath = juce::Path();
    rainbowPath.addCentredArc(center.x, center.y, width / 2.7f, width / 2.7f, 0, startRadians, endRadians, true);
    g.strokePath(rainbowPath, juce::PathStrokeType(width / 4.0f));

    // Draw current value line on end of arc
    juce::Colour outlineCol = slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);
    g.setColour(outlineCol);
    g.drawLine(juce::Line<float>(center.getPointOnCircumference(width / 4.0f, width / 4.0f, endRadians),
                                 center.getPointOnCircumference((width / 2.0f) - 2, (width / 2.0f) - 2, endRadians)), 3.0f);

    // Draw outline arcs
    g.setColour(outlineCol);
    rainbowPath.clear();
    rainbowPath.addCentredArc(center.x, center.y, (width / 2.0f) - 2, (width / 2.0f) - 2, 0, startRadians, 2.5f * juce::MathConstants<float>::pi, true);
    g.strokePath(rainbowPath, juce::PathStrokeType(3));

    // Draw text label inside arc
    juce::Rectangle<int> textRect = juce::Rectangle<int>(0, height / 2, width, height / 2);
    juce::String text =
        slider.getTextFromValue(slider.getValue()).trimCharactersAtEnd(slider.getTextValueSuffix()).trimCharactersAtEnd("0") +
        slider.getTextValueSuffix();
    if (text.getLastCharacter() == '.') text += "0";
    g.drawFittedText(text, textRect, juce::Justification::centredBottom, 1);
  }

  void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override {
    juce::Colour fillColour = btn.findColour(juce::ToggleButton::ColourIds::tickColourId);
    g.setColour(fillColour);
    g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), 4.0f, 2.0f);

    juce::Colour btnColour = btn.getToggleState() ? fillColour : juce::Colours::black;
    if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
      btnColour = fillColour.interpolatedWith(juce::Colours::black, 0.8);
    }
    g.setColour(btnColour);
    g.fillRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), 4.0f);
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowLookAndFeel)
};