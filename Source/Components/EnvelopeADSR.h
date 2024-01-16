/*
  ==============================================================================

    EnvelopeADSR.h
    Created: 12 Jul 2021 12:02:12am
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "Sliders.h"

//==============================================================================
/*
 */
class EnvelopeADSR : public juce::Component,
public juce::AudioProcessorParameter::Listener,
public juce::Timer {
 public:
  EnvelopeADSR(Parameters& parameters);
  ~EnvelopeADSR();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}
  
  void timerCallback() override;

 private:
  // Bookkeeping
  Parameters& mParameters;
  std::atomic<bool> mParamHasChanged;

  // Components
  ParamSlider mSliderAttack;
  ParamSlider mSliderDecay;
  ParamSlider mSliderSustain;
  ParamSlider mSliderRelease;
  juce::Label mLabelAttack;
  juce::Label mLabelDecay;
  juce::Label mLabelSustain;
  juce::Label mLabelRelease;

  // UI rects updated at resize()
  juce::Rectangle<float> mVizRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeADSR)
};
