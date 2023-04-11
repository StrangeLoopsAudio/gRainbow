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
#include "../Parameters.h"

//==============================================================================
/*
 */
class EnvelopeADSR : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  EnvelopeADSR(Parameters& parameters);
  ~EnvelopeADSR();

  void paint(juce::Graphics&) override;
  void resized() override;

  void selectPitchClass(Utils::PitchClass pitchClass);

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();

 private:
  static constexpr const char* SECTION_TITLE = "amplitude envelope";

  // Components
  juce::Slider mSliderAttack;
  juce::Slider mSliderDecay;
  juce::Slider mSliderSustain;
  juce::Slider mSliderRelease;
  juce::Label mLabelAttack;
  juce::Label mLabelDecay;
  juce::Label mLabelSustain;
  juce::Label mLabelRelease;

  // Bookkeeping
  Parameters& mParameters;
  std::atomic<bool> mParamHasChanged;
  Utils::PitchClass mSelPitchClass = Utils::PitchClass::NONE;
  juce::Rectangle<float> mTitleRect;
  juce::Rectangle<float> mVizRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeADSR)
};
