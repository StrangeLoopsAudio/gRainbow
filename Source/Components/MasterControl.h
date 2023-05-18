/*
  ==============================================================================

    MasterControl.h
    Created: 16 May 2023 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Parameters.h"
#include "../RainbowLookAndFeel.h"
#include "ff_meters/ff_meters.h"

//==============================================================================
/*
 */
class MasterControl : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  MasterControl(Parameters& parameters, foleys::LevelMeterSource& meterSource);
  ~MasterControl() override {}

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();

 private:
  static constexpr const char* SECTION_TITLE = "master control";

  // Components
  // -- Generator Adjustments
  RainbowSlider mSliderGain;
  juce::Label mLabelGain;
  foleys::LevelMeter mMeter{foleys::LevelMeter::Minimal | foleys::LevelMeter::SingleChannel | foleys::LevelMeter::Horizontal};

  // Bookkeeping
  Parameters& mParameters;
  std::atomic<bool> mParamHasChanged;
  ParamCommon* mCurSelectedParams;
  juce::Colour mParamColour = Utils::GLOBAL_COLOUR;

  // UI values saved on resize
  juce::Rectangle<float> mTitleRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterControl)
};
