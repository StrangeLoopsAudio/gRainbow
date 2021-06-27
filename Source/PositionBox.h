/*
  ==============================================================================

    PositionBox.h
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "RainbowEnvelopes.h"

//==============================================================================
/*
 */
class PositionBox : public juce::Component {
 public:
  PositionBox();
  ~PositionBox() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void setColour(juce::Colour colour);

 private:
  /* Params */
  static constexpr auto NUM_PARAMS = 3;
  static constexpr auto PARAM_DURATION_DEFAULT = 0.5f;
  static constexpr auto PARAM_RATE_DEFAULT = 0.5f;
  static constexpr auto PARAM_GAIN_DEFAULT = 0.8f;

  /* UI Layout */
  static constexpr auto PADDING_SIZE = 5;
  static constexpr auto TOGGLE_SIZE = 20;
  static constexpr auto KNOB_HEIGHT = 40;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto ENVELOPE_HEIGHT = 60;

  juce::Colour mColour;

  /* UI Components */
  juce::ToggleButton mBtnEnabled;
  juce::ToggleButton mBtnSolo;
  juce::Slider mSliderRate;
  juce::Label mLabelRate;
  juce::Slider mSliderDuration;
  juce::Label mLabelDuration;
  juce::Slider mSliderGain;
  juce::Label mLabelGain;
  RainbowEnvelopes mGrainEnvelopes;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionBox)
};
