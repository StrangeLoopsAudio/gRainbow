/*
  ==============================================================================

    GlobalParamBox.h
    Created: 30 Jul 2021 2:50:47pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "FilterControl.h"
#include "../DSP/GranularSynth.h"

//==============================================================================
/*
 */
class GlobalParamBox : public juce::Component {
 public:
  GlobalParamBox(ParamGlobal& paramGlobal);
  ~GlobalParamBox() override;

  void paint(juce::Graphics&) override;
  void resized() override;

 private:
  // UI Layout
  static constexpr int NUM_AMP_ENV_PARAMS = 4;
  static constexpr float PADDING_SIZE = 0.01f;
  static constexpr float SECTION_TITLE_HEIGHT = 0.1f;
  static constexpr float KNOB_HEIGHT = 0.3f;
  static constexpr float LABEL_HEIGHT = 0.1f;
  static constexpr auto MAIN_TITLE = "global parameters";
  static constexpr const char* SECTION_FILTER_ENV_TITLE = "filter control";

  // Parameters
  ParamGlobal& mParamGlobal;

  // UI Components
  std::unique_ptr<Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment>> mSliderGain;
  std::unique_ptr<Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment>> mSliderCutoff;
  std::unique_ptr<Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment>> mSliderResonance;
  // -- Filter Control
  juce::Label mLabelCutoff;
  juce::Label mLabelResonance;
  FilterControl mFilterControl;
  // -- ADSR Env
  juce::Label mLabelGain;
  // UI values saved on resize
  juce::Rectangle<float> mTitleRect;
  juce::Rectangle<int> mFilterTitleRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalParamBox)
};
