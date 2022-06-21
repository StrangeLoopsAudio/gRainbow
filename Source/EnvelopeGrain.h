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
  EnvelopeGrain();
  ~EnvelopeGrain() override {}

  void paint(juce::Graphics&) override;
  void resized() override;

  void setActive(bool isActive);
  void setRate(float rate);
  void setDuration(float duration);
  void setShape(float shape);
  void setTilt(float tilt);
  void setSync(bool sync);
  void setColour(juce::Colour colour);

 private:
  static constexpr auto MIN_RATE_RATIO = .25f;
  static constexpr auto MAX_RATE_RATIO = 1.0f;
  static constexpr auto GAIN_HEIGHT = 0.8f;

  juce::PathStrokeType mPathStroke;

  // Parameters
  float mShape = 0.5f;
  float mTilt = 0.5f;
  float mRate = 0.5f;
  float mDuration = 0.5f;
  bool mIsActive = false;
  bool mSync = false;
  juce::Colour mColour;

  // UI values saved on resize
  float mMinEnvWidth;
  float mMaxEnvWidth;
  float mEnvTop;
  float mEnvBottom;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeGrain)
};
