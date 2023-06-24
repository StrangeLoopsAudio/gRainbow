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
#include "RainbowLookAndFeel.h"

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

  void updateSelectedParams();

 private:
  static constexpr const char* SECTION_TITLE = "filter";

  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour = Utils::GLOBAL_COLOUR;

  // Components
  RainbowSlider mSliderCutoff;
  RainbowSlider mSliderResonance;
  juce::Label mLabelCutoff;
  juce::Label mLabelResonance;
  juce::ComboBox mFilterType;

  // UI values saved on resize
  juce::Rectangle<float> mTitleRect;
  juce::Rectangle<float> mVizRect;

  float filterTypeToCutoff(Utils::FilterType filterType);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterControl)
};
