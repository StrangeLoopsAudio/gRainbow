/*
  ==============================================================================

    EnvelopeADSR.cpp
    Created: 12 Jul 2021 12:02:12am
    Author:  brady

  ==============================================================================
*/

#include <JuceHeader.h>
#include "EnvelopeADSR.h"

//==============================================================================
EnvelopeADSR::EnvelopeADSR()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

EnvelopeADSR::~EnvelopeADSR()
{
}

void EnvelopeADSR::paint (juce::Graphics& g)
{
  g.fillAll(juce::Colours::black);

  juce::Colour envColour = mIsActive ? mColour : juce::Colours::darkgrey;
  g.setFillType(
      juce::ColourGradient(envColour, getLocalBounds().getTopLeft().toFloat(),
                           envColour.withAlpha(0.4f),
                           getLocalBounds().getBottomLeft().toFloat(), false));

  // Draw ADSR path
  float maxParamWidth = getWidth() / 4.0f;
  juce::Path adsrPath;
  adsrPath.startNewSubPath(juce::Point<float>(0, getHeight()));
  adsrPath.lineTo(juce::Point<float>(mAttack * maxParamWidth, 0));
  adsrPath.lineTo(adsrPath.getCurrentPosition()
                      .translated(mDecay * maxParamWidth, 0.0f)
                      .withY((1.0f - mSustain) * getHeight()));
  adsrPath.lineTo(juce::Point<float>(maxParamWidth * 3.0f,
                                     (1.0f - mSustain) * getHeight()));
  adsrPath.lineTo(adsrPath.getCurrentPosition()
                      .translated(mRelease * maxParamWidth, 0.0f)
                      .withY(getHeight()));
  adsrPath.closeSubPath();
  g.fillPath(adsrPath);
}

void EnvelopeADSR::resized()
{
}

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

void EnvelopeADSR::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();
}