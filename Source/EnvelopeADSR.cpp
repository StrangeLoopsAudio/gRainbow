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

  juce::Colour envColour = mIsActive ? juce::Colour(Utils::POSITION_COLOURS[mColour]) : juce::Colours::darkgrey;
  g.setFillType(
      juce::ColourGradient(envColour, getLocalBounds().getTopLeft().toFloat(),
                           envColour.withAlpha(0.4f),
                           getLocalBounds().getBottomLeft().toFloat(), false));

  // Draw ADSR path
  juce::Path adsrPath;
  adsrPath.startNewSubPath(juce::Point<float>(0, getHeight()));
  adsrPath.lineTo(juce::Point<float>(mAttack * getWidth() * 0.375f, 0));
  adsrPath.lineTo(adsrPath.getCurrentPosition()
                      .translated(mDecay * getWidth() * 0.375f, 0.0f)
                      .withY((1.0f - mSustain) * getHeight()));
  adsrPath.lineTo(juce::Point<float>(getWidth() * 0.75f,
                                     (1.0f - mSustain) * getHeight()));
  adsrPath.lineTo(adsrPath.getCurrentPosition()
                      .translated(mRelease * getWidth() * 0.25f, 0.0f)
                      .withY(getHeight()));
  adsrPath.closeSubPath();
  g.fillPath(adsrPath);

  // Draw highlight lines on top of each segment
  float highlightWidth = 3.0f;
  g.setColour(mIsActive
                  ? juce::Colour(Utils::SECONDARY_POSITION_COLOURS[mColour][0])
                  : juce::Colours::darkgrey);
  g.drawLine(
      juce::Line<float>(1.0f, getHeight() - 1.0f, mAttack * getWidth() * 0.375f + 1.0f, 1.0f),
      highlightWidth);
  g.setColour(mIsActive
                  ? juce::Colour(Utils::SECONDARY_POSITION_COLOURS[mColour][1])
                  : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(
                 mAttack * getWidth() * 0.375f, 1.0f,
                 mAttack * getWidth() * 0.375f + mDecay * getWidth() * 0.375f + 1.0f,
                 (1.0f - mSustain) * getHeight()),
             highlightWidth);
  g.setColour(mIsActive
                  ? juce::Colour(Utils::SECONDARY_POSITION_COLOURS[mColour][2])
                  : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(
                 mAttack * getWidth() * 0.375f + mDecay * getWidth() * 0.375f,
                 (1.0f - mSustain) * getHeight(), getWidth() * 0.75f + 1.0f,
                 (1.0f - mSustain) * getHeight()),
             highlightWidth);
  g.setColour(mIsActive
                  ? juce::Colour(Utils::SECONDARY_POSITION_COLOURS[mColour][3])
                  : juce::Colours::darkgrey);
  g.drawLine(
      juce::Line<float>(getWidth() * 0.75f, (1.0f - mSustain) * getHeight(),
                        mRelease * getWidth() * 0.25f + getWidth() * 0.75f,
                        getHeight() - 1.0f),
      highlightWidth);
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

void EnvelopeADSR::setColour(Utils::PositionColour colour) {
  mColour = colour;
  repaint();
}