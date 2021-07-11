/*
  ==============================================================================

    EnvelopeADSR.h
    Created: 12 Jul 2021 12:02:12am
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class EnvelopeADSR : public juce::Component {
 public:
  EnvelopeADSR();
  ~EnvelopeADSR() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void setActive(bool isActive);
  void setAttack(float attack);
  void setDecay(float decay);
  void setSustain(float sustain);
  void setRelease(float release);
  void setColour(juce::Colour colour);

 private:

  /* Parameters */
  float mAttack = 0.2f;
  float mDecay = 0.1f;
  float mSustain = 0.8f;
  float mRelease = 0.2f;
  bool mIsActive = false;
  juce::Colour mColour;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeADSR)
};
