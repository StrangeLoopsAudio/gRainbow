/*
  ==============================================================================

    PositionBox.h
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GranularSynth.h"
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
  bool getActive() { return mIsActive; }
  void setActive(bool isActive);

  void setColour(GranularSynth::PositionColour colour);
  GranularSynth::PositionParams getParams();
  std::function<void(GranularSynth::PositionColour pos,
                     GranularSynth::ParameterType param,
                     float value)>
      onParameterChanged = nullptr;

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

  static constexpr juce::int64 POSITION_COLOURS[GranularSynth::NUM_POSITIONS] =
      {
      0xFF52C4FF, 0xFFE352FF,
                                                      0xFFFF8D52, 0xFF6EFF52};

  GranularSynth::PositionColour mColour;
  bool mIsActive = false;

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

  void parameterChanged(GranularSynth::ParameterType type, float value);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionBox)
};
