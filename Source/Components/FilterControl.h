/*
  ==============================================================================

    FilterControl.h
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Parameters.h"
#include "../RainbowLookAndFeel.h"

//==============================================================================
/*
 */
class FilterControl : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  FilterControl(Parameters& parameters);
  ~FilterControl() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void mouseMove(const juce::MouseEvent& event) override;
  void mouseExit(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event) override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();

 private:
  static constexpr int FILTER_TYPE_BUTTON_HEIGHT = 30;
  static constexpr const char* SECTION_TITLE = "filter control";

  // Components
  RainbowSlider mSliderCutoff;
  RainbowSlider mSliderResonance;
  juce::Label mLabelCutoff;
  juce::Label mLabelResonance;

  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour = Utils::GLOBAL_COLOUR;
  Utils::FilterType mCurHoverFilterType = Utils::FilterType::NO_FILTER;

  // UI values saved on resize
  juce::Rectangle<float> mLowPassRect;
  juce::Rectangle<float> mHighPassRect;
  juce::Rectangle<float> mBandPassRect;
  juce::Rectangle<float> mTitleRect;
  juce::Rectangle<float> mVizRect;

  Utils::FilterType getFilterTypeFromMouse(const juce::MouseEvent& event);
  float filterTypeToCutoff(Utils::FilterType filterType);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterControl)
};
