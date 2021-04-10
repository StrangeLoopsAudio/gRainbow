/*
  ==============================================================================

    RainbowLookAndFeel.cpp
    Created: 10 Apr 2021 3:07:42pm
    Author:  brady

  ==============================================================================
*/
#define _USE_MATH_DEFINES

#include "RainbowLookAndFeel.h"
#include "Utils.h"
#include <math.h>

void RainbowLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
  float sliderPosProportional, float rotaryStartAngle,
  float rotaryEndAngle, juce::Slider&)
{
  float endRadians = (1.5 * M_PI) + (sliderPosProportional * M_PI);
  juce::Path rainbowPath = juce::Path();
  g.setColour(juce::Colours::white);
  int curStripeStart = height / 4;
  int stripeInc = (height - curStripeStart) / 7;
  for (int i = 0; i < 7; i++)
  {
    rainbowPath.clear();
    rainbowPath.addCentredArc(width / 2.f, height,
        curStripeStart + (stripeInc / 2.0f), curStripeStart + (stripeInc / 2.0f),
        0, 1.5 * M_PI, endRadians, true);
    curStripeStart += stripeInc;
    g.setColour(Utils::getRainbowColour(1.0f - ((i + 0.5) / 7.0f)));
    g.strokePath(rainbowPath, juce::PathStrokeType(stripeInc));
  }
  // Draw boundaries
  rainbowPath.clear();
  rainbowPath.addCentredArc(width / 2.f, height,
    height / 4, height / 4,
    0, 1.5 * M_PI, 2.5 * M_PI, true);
  rainbowPath.addCentredArc(width / 2.f, height,
     curStripeStart, curStripeStart,
     0, 1.5 * M_PI, 2.5 * M_PI, true);
  g.setColour(juce::Colours::white);
  g.strokePath(rainbowPath, juce::PathStrokeType(2));
}