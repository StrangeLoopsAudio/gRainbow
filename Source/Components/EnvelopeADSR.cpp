/*
  ==============================================================================

    EnvelopeADSR.cpp
    Created: 12 Jul 2021 12:02:12am
    Author:  brady

  ==============================================================================
*/

#include "EnvelopeADSR.h"

#include <JuceHeader.h>

//==============================================================================
EnvelopeADSR::EnvelopeADSR() {
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
}

EnvelopeADSR::~EnvelopeADSR() {}

void EnvelopeADSR::paint(juce::Graphics& g) {
  juce::Colour envColour = mIsActive ? mColour : juce::Colours::darkgrey;
  g.setFillType(juce::ColourGradient(envColour, getLocalBounds().getTopLeft().toFloat(), envColour.withAlpha(0.4f),
                                     getLocalBounds().getBottomLeft().toFloat(), false));
  float envTop = juce::jlimit(1.0f, getHeight() - 1.0f, getHeight() - mGain * getHeight());
  float sustainY = juce::jlimit(1.0f, getHeight() - 1.0f, getHeight() - (mSustain * mGain * getHeight()));

  // Draw ADSR path
  juce::Path adsrPath;
  adsrPath.startNewSubPath(juce::Point<float>(0, getHeight()));
  adsrPath.lineTo(juce::Point<float>(mAttack * getWidth() * 0.375f, envTop));
  adsrPath.lineTo(adsrPath.getCurrentPosition().translated(mDecay * getWidth() * 0.375f, 0.0f).withY(sustainY));
  adsrPath.lineTo(juce::Point<float>(getWidth() * 0.75f, sustainY));
  adsrPath.lineTo(adsrPath.getCurrentPosition().translated(mRelease * getWidth() * 0.25f, 0.0f).withY(getHeight()));
  adsrPath.closeSubPath();
  g.fillPath(adsrPath);

  // Draw highlight lines on top of each segment
  float highlightWidth = 3.0f;
  g.setColour(mIsActive ? envColour.brighter() : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(1.0f, getHeight() - 1.0f, mAttack * getWidth() * 0.375f + 1.0f, envTop), highlightWidth);
  g.setColour(mIsActive ? envColour.brighter().brighter() : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(mAttack * getWidth() * 0.375f, envTop,
                               mAttack * getWidth() * 0.375f + mDecay * getWidth() * 0.375f + 1.0f, sustainY),
             highlightWidth);
  g.setColour(mIsActive ? envColour.brighter().brighter().brighter() : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(mAttack * getWidth() * 0.375f + mDecay * getWidth() * 0.375f, sustainY, getWidth() * 0.75f + 1.0f,
                               sustainY),
             highlightWidth);
  g.setColour(mIsActive ? envColour.brighter().brighter().brighter().brighter() : juce::Colours::darkgrey);
  g.drawLine(
      juce::Line<float>(getWidth() * 0.75f, sustainY, mRelease * getWidth() * 0.25f + getWidth() * 0.75f, getHeight() - 1.0f),
      highlightWidth);

  g.setColour(envColour);
  g.drawRect(getLocalBounds(), 2.0f);
}

void EnvelopeADSR::resized() {}

void EnvelopeADSR::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void EnvelopeADSR::setAttack(float attack) {
  mAttack = attack;
  repaint();
}

void EnvelopeADSR::setDecay(float decay) {
  mDecay = decay;
  repaint();
}

void EnvelopeADSR::setSustain(float sustain) {
  mSustain = sustain;
  repaint();
}

void EnvelopeADSR::setRelease(float release) {
  mRelease = release;
  repaint();
}

void EnvelopeADSR::setGain(float gain) {
  mGain = gain;
  repaint();
}

void EnvelopeADSR::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();
}