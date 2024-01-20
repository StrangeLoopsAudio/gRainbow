/*
  ==============================================================================

    FilterControl.h
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "Sliders.h"

//==============================================================================
/*
 */
class FilterControl : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  FilterControl(Parameters& parameters);
  ~FilterControl() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

 private:
  
  void updateFilterPath();

  // Bookkeeping
  Parameters& mParameters;
  std::atomic<bool> mParamHasChanged;
  juce::PathStrokeType mPathStroke;
  juce::Path mFilterPath;

  // Components
  ParamSlider mSliderCutoff;
  ParamSlider mSliderResonance;
  juce::Label mLabelCutoff;
  juce::Label mLabelResonance;
  juce::ComboBox mFilterType;

  // UI values saved on resize
  juce::Rectangle<float> mVizRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterControl)
};
