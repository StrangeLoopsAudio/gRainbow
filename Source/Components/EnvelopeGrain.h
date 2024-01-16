/*
  ==============================================================================

    EnvelopeGrain.h
    Created: 23 Jun 2021 8:34:54pm
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
class EnvelopeGrain : public juce::Component,
public Parameters::Listener,
public juce::AudioProcessorParameter::Listener,
public juce::Timer {
 public:
  EnvelopeGrain(Parameters& parameters);
  ~EnvelopeGrain();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}
  
  void selectedCommonParamsChanged(ParamCommon* newParams) override;

  void timerCallback() override;

 private:
  static constexpr double WINDOW_SECONDS = 0.4; // Width in seconds of the display window

  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour;

  // Components
  CommonSlider mSliderShape;
  CommonSlider mSliderTilt;
  QuantizedCommonSlider mSliderRate;
  QuantizedCommonSlider mSliderDuration;
  juce::TextButton mBtnSync;
  juce::Label mLabelShape;
  juce::Label mLabelTilt;
  juce::Label mLabelRate;
  juce::Label mLabelDuration;

  juce::PathStrokeType mPathStroke;

  // UI values saved on resize
  juce::Rectangle<float> mVizRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeGrain)
};
