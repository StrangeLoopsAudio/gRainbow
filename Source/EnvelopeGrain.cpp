/*
  ==============================================================================

    EnvelopeGrain.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "EnvelopeGrain.h"
#include "Parameters.h"
#include <JuceHeader.h>

EnvelopeGrain::EnvelopeGrain()
    : mPathStroke(2, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::rounded) {}

void EnvelopeGrain::paint(juce::Graphics& g) {
  juce::Colour mainColour = mIsActive ? mColour : juce::Colours::darkgrey;

  float envWidth;
  float envOffset;
  if (mSync) {
    float durDiv = std::pow(2, (int)(ParamRanges::SYNC_DIV_MAX * mDuration));
    envWidth = mMaxEnvWidth / durDiv;
    float rateDiv = std::pow(2, (int)(ParamRanges::SYNC_DIV_MAX * mRate));
    envOffset = envWidth / rateDiv;
  } else {
    envWidth = juce::jmap(mDuration, mMinEnvWidth, mMaxEnvWidth);
    envOffset = juce::jmap(mRate, envWidth * MIN_RATE_RATIO, envWidth * MAX_RATE_RATIO);
  }

  float shapeWidth = envWidth * mShape / 2.0f;

  // Draw darker odd numbered envelopes
  float curXStart = envOffset;
  juce::Colour envColour = mainColour.darker(0.5f);
  while (curXStart < getWidth()) {
    juce::Path envPath;
    envPath.startNewSubPath(juce::Point<float>(curXStart, mEnvBottom));
    envPath.lineTo(juce::Point<float>(juce::jmax(curXStart, curXStart + (mTilt * envWidth) - shapeWidth), mEnvTop));
    envPath.lineTo(juce::Point<float>(juce::jmin(curXStart + envWidth, curXStart + (mTilt * envWidth) + shapeWidth), mEnvTop));
    envPath.lineTo(juce::Point<float>(curXStart + envWidth, mEnvBottom));
    g.setColour(envColour);
    g.strokePath(envPath, mPathStroke);
    curXStart += (envOffset * 2.0f);
  }
  curXStart = 0;
  // Draw brighter even numbered envelopes
  envColour = mainColour.brighter(0.5f);
  while (curXStart < getWidth()) {
    juce::Path envPath;
    envPath.startNewSubPath(juce::Point<float>(curXStart, mEnvBottom));
    envPath.lineTo(juce::Point<float>(juce::jmax(curXStart, curXStart + (mTilt * envWidth) - shapeWidth), mEnvTop));
    envPath.lineTo(juce::Point<float>(juce::jmin(curXStart + envWidth, curXStart + (mTilt * envWidth) + shapeWidth), mEnvTop));
    envPath.lineTo(juce::Point<float>(curXStart + envWidth, mEnvBottom));
    g.setColour(envColour);
    g.strokePath(envPath, mPathStroke);
    curXStart += (envOffset * 2.0f);
  }
  g.setColour(mainColour);
  g.drawRect(getLocalBounds(), 2.0f);
}

void EnvelopeGrain::resized() {
  mMinEnvWidth = getWidth() / 6.0f;
  mMaxEnvWidth = getWidth();
  mEnvTop = ((1.0f - GAIN_HEIGHT) * (getHeight() - 2)) + 2;
  mEnvBottom = getHeight() - 1.0f;
}

void EnvelopeGrain::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void EnvelopeGrain::setShape(float shape) {
  mShape = shape;
  repaint();
}

void EnvelopeGrain::setTilt(float tilt) {
  mTilt = tilt;
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

void EnvelopeGrain::setSync(bool sync) {
  mSync = sync;
  repaint();
}

void EnvelopeGrain::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();
}
