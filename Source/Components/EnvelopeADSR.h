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
#include "../RainbowLookAndFeel.h"

//==============================================================================
/*
 */
class EnvelopeADSR : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  EnvelopeADSR(Parameters& parameters);
  ~EnvelopeADSR();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();

 private:
  static constexpr const char* SECTION_TITLE = "amplitude envelope";

  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour = Utils::GLOBAL_COLOUR;

  // Components
  RainbowSlider mSliderAttack;
  RainbowSlider mSliderDecay;
  RainbowSlider mSliderSustain;
  RainbowSlider mSliderRelease;
  juce::Label mLabelAttack;
  juce::Label mLabelDecay;
  juce::Label mLabelSustain;
  juce::Label mLabelRelease;

  // UI rects updated at resize()
  juce::Rectangle<float> mTitleRect;
  juce::Rectangle<float> mVizRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeADSR)
};
