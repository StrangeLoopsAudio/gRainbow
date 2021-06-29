/*
  ==============================================================================

    RainbowEnvelopes.h
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class RainbowEnvelopes : public juce::Component {
 public:
  RainbowEnvelopes() {}
  ~RainbowEnvelopes() override {}

  void paint(juce::Graphics&) override;
  void resized() override;

  void setActive(bool isActive);
  void setRate(float rate);
  void setDuration(float duration);
  void setGain(float gain);
  void setColour(juce::Colour colour);

 private:

  /* Parameters */
  float mRate = 0.5;
  float mDuration = 0.5;
  float mGain = 0.8;
  bool mIsActive = false;
  juce::Colour mColour;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowEnvelopes)
};
