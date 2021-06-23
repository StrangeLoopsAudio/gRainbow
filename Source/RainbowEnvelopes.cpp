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

  float minEnvWidth = getWidth() / 10.0f;
  float maxEnvWidth = getWidth() / (float)MAX_ENVELOPES;
  float envWidth = juce::jmap(mDuration, minEnvWidth, maxEnvWidth);
  float envOffset = envWidth * (1.0f - mRate);
  
  for (int i = 0; i < mNumEnvelopes; ++i) {
    g.setColour(juce::Colour(ENVELOPE_COLOURS[i]));
    int envLeft = i * envOffset;
    juce::Path envPath;
    juce::Point<float> startPoint = juce::Point<float>(envLeft, getHeight());
    envPath.startNewSubPath(startPoint);
    envPath.cubicTo(startPoint.translated(envWidth / 4.0f, 0),
                    startPoint.translated(envWidth / 4.0f, -getHeight()),
                    startPoint.translated(envWidth / 2.0f, -getHeight()));
    startPoint = startPoint.translated(envWidth / 2.0f, -getHeight());
    envPath.cubicTo(startPoint.translated(envWidth / 4.0f, 0),
                    startPoint.translated(envWidth / 4.0f, getHeight()),
                    startPoint.translated(envWidth / 2.0f, getHeight()));
    g.strokePath(envPath, juce::PathStrokeType(2));
  }
}

void RainbowEnvelopes::resized() {}

void RainbowEnvelopes::setRate(float rate) {
  mRate = rate;
  repaint();
}

void RainbowEnvelopes::setDuration(float duration) {
  mDuration = duration;
  repaint();
}

void RainbowEnvelopes::setNumEnvelopes(int numEnvs) {
  mNumEnvelopes = numEnvs;
  repaint(); 
}
