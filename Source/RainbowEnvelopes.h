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
  RainbowEnvelopes(float rate, float duration)
      : mRate(rate), mDuration(duration) {}
  ~RainbowEnvelopes() override {}

  void paint(juce::Graphics&) override;
  void resized() override;

  void setRate(float rate);
  void setDuration(float duration);
  void setNumEnvelopes(int numEnvs);

 private:
  static constexpr auto MAX_ENVELOPES = 4;
  static constexpr juce::int64 ENVELOPE_COLOURS[MAX_ENVELOPES] = {
      0xFF52C4FF, 0xFFE352FF, 0xFFFF8D52, 0xFF6EFF52};

  /* Parameters */
  float mRate;
  float mDuration;
  int mNumEnvelopes = 1;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowEnvelopes)
};
