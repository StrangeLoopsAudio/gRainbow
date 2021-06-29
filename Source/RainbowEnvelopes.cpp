/*
  ==============================================================================

    RainbowEnvelopes.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "RainbowEnvelopes.h"

#include <JuceHeader.h>

void RainbowEnvelopes::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  juce::Colour envColour = mIsActive ? mColour : juce::Colours::darkgrey;
  g.setColour(envColour);

  float minEnvWidth = getWidth() / 10.0f;
  float maxEnvWidth = getWidth() / 3.0f;
  float envWidth = juce::jmap(mDuration, minEnvWidth, maxEnvWidth);
  float envOffset = juce::jmax(envWidth * (1.0f - mRate), 1.0f);
  float envTop = ((1.0f - mGain) * (getHeight() - 2)) + 2;
  float envBottom = getHeight();
  float envHeight = juce::jmax(0.0f, envBottom - envTop);
  juce::PathStrokeType pathStroke =
      juce::PathStrokeType(2, juce::PathStrokeType::JointStyle::curved,
                           juce::PathStrokeType::EndCapStyle::rounded);
  float curXStart = 0;
  while (curXStart < getWidth()) {
    juce::Path envPath;
    juce::Point<float> startPoint = juce::Point<float>(curXStart, envBottom);
    envPath.startNewSubPath(startPoint);
    envPath.cubicTo(startPoint.translated(envWidth / 4.0f, 0),
                    startPoint.translated(envWidth / 4.0f, -envHeight),
                    startPoint.translated(envWidth / 2.0f, -envHeight));
    startPoint = startPoint.translated(envWidth / 2.0f, -envHeight);
    envPath.cubicTo(startPoint.translated(envWidth / 4.0f, 0),
                    startPoint.translated(envWidth / 4.0f, envHeight),
                    startPoint.translated(envWidth / 2.0f, envHeight));
    g.strokePath(envPath, pathStroke);
    curXStart += envOffset;
  }
}

void RainbowEnvelopes::resized() {}

void RainbowEnvelopes::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void RainbowEnvelopes::setRate(float rate) {
  mRate = rate;
  repaint();
}

void RainbowEnvelopes::setDuration(float duration) {
  mDuration = duration;
  repaint();
}

void RainbowEnvelopes::setGain(float gain) {
  mGain = gain;
  repaint();
}

void RainbowEnvelopes::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();
}
