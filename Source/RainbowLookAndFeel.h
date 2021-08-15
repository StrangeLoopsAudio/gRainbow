/*
  ==============================================================================

    RainbowLookAndFeel.h
    Created: 10 Apr 2021 3:07:42pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Utils.h"

class RainbowLookAndFeel : public juce::LookAndFeel_V4
{
public:
 RainbowLookAndFeel() {}

private:
  void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle,
                       float rotaryEndAngle, juce::Slider& slider) override {
   float pos = juce::jmax(0.02f, sliderPosProportional);
    float startRadians = 1.5f * juce::MathConstants<float>::pi;
   float endRadians = startRadians + (pos * juce::MathConstants<float>::pi);
   
   int curStripeStart = height / 2.5;
   int stripeInc = (height - curStripeStart) / 7;
   juce::Colour rainbowCol =
       slider.findColour(juce::Slider::ColourIds::rotarySliderFillColourId);

   // Draw main arc
   g.setFillType(
       juce::ColourGradient(rainbowCol, slider.getLocalBounds().getBottomLeft().toFloat(),
                            rainbowCol.withAlpha(0.4f),
                            slider.getLocalBounds().getTopLeft().toFloat(), false));
   juce::Path rainbowPath = juce::Path();
   rainbowPath.addCentredArc(width / 2.0f, height, width / 2.7f, width / 2.7f,
                             0, startRadians, endRadians, true);
   g.strokePath(rainbowPath, juce::PathStrokeType(width / 4.0f));

   // Draw outline arcs
   juce::Colour outlineCol =
       slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);
   g.setColour(outlineCol);
   rainbowPath.clear();
   rainbowPath.addCentredArc(width / 2.0f, height, (width / 2.0f) - 2,
                             (width / 2.0f) - 2, 0, startRadians, endRadians,
                             true);
   g.strokePath(rainbowPath, juce::PathStrokeType(3));

   // Draw text label inside arc
   juce::Rectangle<int> textRect =
       juce::Rectangle<int>(0, height / 2, width, height / 2);
   juce::String text = slider.getTextFromValue(slider.getValue())
                           .trimCharactersAtEnd(slider.getTextValueSuffix())
                           .trimCharactersAtEnd("0") +
                       slider.getTextValueSuffix();
   if (text.getLastCharacter() == '.') text += "0";
   g.drawFittedText(text,
                    textRect, juce::Justification::centredBottom, 1);
  }

  void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn,
                       bool shouldDrawButtonAsHighlighted,
                       bool shouldDrawButtonAsDown) override {
   juce::Colour fillColour =
       btn.findColour(juce::ToggleButton::ColourIds::tickColourId);
   g.setColour(fillColour);
   g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), 4.0f,
                          2.0f);

   juce::Colour btnColour =
       btn.getToggleState() ? fillColour : juce::Colours::black;
   if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
     btnColour = fillColour.interpolatedWith(juce::Colours::black, 0.8);
   }
   g.setColour(btnColour);
   g.fillRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), 4.0f);
 }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowLookAndFeel)
};