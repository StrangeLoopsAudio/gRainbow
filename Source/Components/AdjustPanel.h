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

//==============================================================================
/*
 */
class AdjustPanel : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  AdjustPanel(Parameters& parameters);
  ~AdjustPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();

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
  juce::ToggleButton mBtnReverse;
  juce::Label mLabelReverse;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdjustPanel)
};
