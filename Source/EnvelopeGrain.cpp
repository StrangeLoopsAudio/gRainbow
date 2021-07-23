/*
  ==============================================================================

    EnvelopeGrain.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "EnvelopeGrain.h"
#include <JuceHeader.h>

void EnvelopeGrain::paint(juce::Graphics& g) {
  juce::Colour envColour = mIsActive ? mColour : juce::Colours::darkgrey;
  g.setColour(envColour);

  float minEnvWidth = getWidth() / 10.0f;
  float maxEnvWidth = getWidth() / 3.0f;
  float envWidth = juce::jmap(mDuration, minEnvWidth, maxEnvWidth);
  float envOffset = juce::jmap(1.0f - mRate, envWidth * MIN_RATE_RATIO, envWidth * MAX_RATE_RATIO);
  float envTop = ((1.0f - mGain) * (getHeight() - 2)) + 2;
  float envBottom = getHeight() - 1.0f;
  float envHeight = juce::jmax(0.0f, envBottom - envTop);
  juce::PathStrokeType pathStroke =
      juce::PathStrokeType(2, juce::PathStrokeType::JointStyle::curved,
                           juce::PathStrokeType::EndCapStyle::rounded);
  float curXStart = 0;
  while (curXStart < getWidth()) {
    juce::Path envPath;
    juce::Point<float> startPoint = juce::Point<float>(curXStart, envBottom);
    float envCtrl = juce::jmap(1.0f - mShape, 0.0f, envWidth / 2.0f);
    envPath.startNewSubPath(startPoint);
    envPath.cubicTo(startPoint.translated(envCtrl, 0),
                    startPoint.translated(envCtrl, -envHeight),
                    startPoint.translated(envWidth / 2.0f, -envHeight));
    startPoint = startPoint.translated(envWidth / 2.0f, -envHeight);
    envPath.cubicTo(startPoint.translated(envWidth / 2.0f - envCtrl, 0),
                    startPoint.translated(envWidth / 2.0f - envCtrl, envHeight),
                    startPoint.translated(envWidth / 2.0f, envHeight));
    g.strokePath(envPath, pathStroke);
    curXStart += envOffset;
  }
}

void EnvelopeGrain::resized() {}

void EnvelopeGrain::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void EnvelopeGrain::setShape(float shape) { 
  mShape = shape;
  repaint();
}

void EnvelopeGrain::setRate(float rate) {
  mRate = rate;
  repaint();
}

void EnvelopeGrain::setDuration(float duration) {
  mDuration = duration;
  repaint();
}

void EnvelopeGrain::setGain(float gain) {
  mGain = gain;
  repaint();
}



void EnvelopeGrain::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();
}
