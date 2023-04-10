/*
  ==============================================================================

    GrainControl.h
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PositionChanger.h"
#include "../Parameters.h"

//==============================================================================
/*
 */
class GrainControl : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  GrainControl(Parameters& parameters);
  ~GrainControl() override {}

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

 private:
  static constexpr const char* SECTION_TITLE = "grain control";

  // Components
  // -- Generator Adjustments
  PositionChanger mPositionChanger;
  juce::Slider mSliderPitchAdjust;
  juce::Label mLabelPitchAdjust;
  juce::Slider mSliderPitchSpray;
  juce::Label mLabelPitchSpray;
  juce::Slider mSliderPosAdjust;
  juce::Label mLabelPosAdjust;
  juce::Slider mSliderPosSpray;
  juce::Label mLabelPosSpray;

  // Bookkeeping
  Parameters& mParameters;
  std::atomic<bool> mParamHasChanged;

  // UI values saved on resize
  juce::Rectangle<float> mTitleRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainControl)
};
