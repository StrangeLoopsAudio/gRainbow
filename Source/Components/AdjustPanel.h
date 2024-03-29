/*
  ==============================================================================

    AdjustPanel.h
    Created: 15 Dec 2023 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "Sliders.h"
#include "Buttons.h"

//==============================================================================
/*
 */
class AdjustPanel : public juce::Component,
public Parameters::Listener,
public juce::AudioProcessorParameter::Listener,
public juce::Timer {
 public:
  AdjustPanel(Parameters& parameters);
  ~AdjustPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void selectedCommonParamsChanged(ParamCommon* newParams) override;

  void timerCallback() override;
  
  std::function<void(void)> onRefToneOn = nullptr;
  std::function<void(void)> onRefToneOff = nullptr;

 private:
  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour;

  // Components
  CommonSlider mSliderPitchAdjust;
  juce::Label mLabelPitchAdjust;
  CommonSlider mSliderPitchSpray;
  juce::Label mLabelPitchSpray;
  CommonSlider mSliderPosAdjust;
  juce::Label mLabelPosAdjust;
  CommonSlider mSliderPosSpray;
  juce::Label mLabelPosSpray;
  CommonSlider mSliderPanAdjust;
  juce::Label mLabelPanAdjust;
  CommonSlider mSliderPanSpray;
  juce::Label mLabelPanSpray;
  CommonSlider mSliderOctaveAdjust;
  juce::Label mLabelOctaveAdjust;
  CommonButton mBtnReverse;
  juce::ToggleButton mBtnRefTone;
  juce::Label mLabelRefTone;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdjustPanel)
};
