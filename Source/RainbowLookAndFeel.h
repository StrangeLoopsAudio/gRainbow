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
   g.setFillType(
       juce::ColourGradient(rainbowCol, slider.getLocalBounds().getBottomLeft().toFloat(),
                            rainbowCol.withAlpha(0.4f),
                            slider.getLocalBounds().getTopLeft().toFloat(), false));
   juce::Path rainbowPath = juce::Path();
   rainbowPath.addCentredArc(width / 2.0f, height, width / 3.0f, width / 3.0f,
                             0, startRadians, endRadians, true);
   g.strokePath(rainbowPath, juce::PathStrokeType(width / 4.0f));
   // Draw boundaries
   rainbowPath.clear();
   rainbowPath.addCentredArc(width / 2.f, height, height / 2.5, height / 2.5, 0,
                             1.5 * juce::MathConstants<float>::pi,
                             2.5 * juce::MathConstants<float>::pi,
                             true);
   rainbowPath.addCentredArc(width / 2.f, height, curStripeStart, curStripeStart, 0,
                             1.5 * juce::MathConstants<float>::pi,
                             2.5 * juce::MathConstants<float>::pi, true);
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