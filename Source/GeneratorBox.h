/*
  ==============================================================================

    GeneratorBox.h
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GranularSynth.h"
#include "EnvelopeGrain.h"
#include "EnvelopeADSR.h"
#include "PositionChanger.h"
#include "Utils.h"

//==============================================================================
/*
 */
class GeneratorBox : public juce::Component {
 public:
  GeneratorBox();
  ~GeneratorBox() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  Utils::GeneratorState getState() { return mState; }
  void setGeneratorEnabled(bool isEnabled) {
    mState.isEnabled = isEnabled;
    refreshState();
  }
  void setGeneratorSolo(bool isSolo) {
    mState.isSolo = isSolo;
    refreshState();
  }
  void setWaiting(bool isWaiting) {
    mState.isWaiting = isWaiting;
    refreshState();
  }

  void setPositionNumber(int positionNumber);
  void setParams(Utils::GeneratorParams params);
  void setNumPositions(int numPositions);

  void setColour(Utils::GeneratorColour colour);
  Utils::GeneratorParams getParams();
  std::function<void(Utils::GeneratorColour pos,
                     GranularSynth::ParameterType param,
                     float value)>
      onParameterChanged = nullptr;

  std::function<void(bool isRight)> onPositionChanged = nullptr;

 private:
  /* Params */
  static constexpr auto NUM_AMP_ENV_PARAMS = 4;
  static constexpr auto NUM_GRAIN_ENV_PARAMS = 4;

  /* UI Layout */
  static constexpr auto PADDING_SIZE = 6;
  static constexpr auto ADJUSTMENT_HEIGHT = 40;
  static constexpr auto TOGGLE_SIZE = 16;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto ENVELOPE_HEIGHT = 60;
  static constexpr auto SECTION_TITLE_HEIGHT = 20;
  static constexpr auto SECTION_AMP_ENV_TITLE = "amplitude envelope";
  static constexpr auto SECTION_GRAIN_ENV_TITLE = "grain envelope";
  static constexpr auto SECTION_ADJUST_TITLE = "generator adjustments";

  /* Bookkeeping */
  Utils::GeneratorColour mColour;
  Utils::GeneratorState mState;

  /* UI Components */
  /* -- Generator Adjustments */
  PositionChanger mPositionChanger;
  juce::Slider mSliderPitch;
  juce::Label mLabelPitch;
  juce::Slider mSliderPosition;
  juce::Label mLabelPosition;
  /* -- Grain Env */
  juce::Slider mSliderShape;
  juce::Label mLabelShape;
  juce::Slider mSliderRate;
  juce::Label mLabelRate;
  juce::Slider mSliderDuration;
  juce::Label mLabelDuration;
  juce::Slider mSliderGain;
  juce::Label mLabelGain;
  EnvelopeGrain mEnvelopeGrain;
  /* -- ADSR Env */
  juce::Slider mSliderAttack;
  juce::Label mLabelAttack;
  juce::Slider mSliderDecay;
  juce::Label mLabelDecay;
  juce::Slider mSliderSustain;
  juce::Label mLabelSustain;
  juce::Slider mSliderRelease;
  juce::Label mLabelRelease;
  EnvelopeADSR mEnvelopeAmp;

  void parameterChanged(GranularSynth::ParameterType type, float value);
  void refreshState();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratorBox)
};
