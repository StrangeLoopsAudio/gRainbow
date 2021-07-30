/*
  ==============================================================================

    GlobalParamBox.h
    Created: 30 Jul 2021 2:50:47pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "EnvelopeADSR.h"
#include "GranularSynth.h"

//==============================================================================
/*
 */
class GlobalParamBox : public juce::Component {
 public:
  GlobalParamBox();
  ~GlobalParamBox() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void setParams(Utils::GlobalParams params);

  std::function<void(GranularSynth::ParameterType param, float value)>
      onParameterChanged = nullptr;

 private:
  /* UI Layout */
  static constexpr auto NUM_AMP_ENV_PARAMS = 4;
  static constexpr auto PADDING_SIZE = 6;
  static constexpr auto MAIN_TITLE_HEIGHT = 30;
  static constexpr auto SECTION_TITLE_HEIGHT = 20;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto ENVELOPE_HEIGHT = 60;
  static constexpr auto MAIN_TITLE = "global parameters";
  static constexpr auto SECTION_AMP_ENV_TITLE = "amplitude envelope";

  /* UI Components */
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

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalParamBox)
};
