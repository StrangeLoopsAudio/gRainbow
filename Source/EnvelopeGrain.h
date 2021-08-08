/*
  ==============================================================================

    EnvelopeGrain.h
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class EnvelopeGrain : public juce::Component {
 public:
  EnvelopeGrain() {}
  ~EnvelopeGrain() override {}

  void paint(juce::Graphics&) override;
  void resized() override;

  void setActive(bool isActive);
  void setRate(float rate);
  void setDuration(float duration);
  void setGain(float gain);
  void setShape(float shape);
  void setTilt(float tilt);
  void setColour(juce::Colour colour);

 private:

  static constexpr auto MIN_RATE_RATIO = .25f;
  static constexpr auto MAX_RATE_RATIO = 1.0f;

  /* Parameters */
  float mShape = 0.5f;
  float mTilt = 0.5f;
  float mRate = 0.5f;
  float mDuration = 0.5f;
  float mGain = 0.8f;
  bool mIsActive = false;
  juce::Colour mColour;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeGrain)
};
