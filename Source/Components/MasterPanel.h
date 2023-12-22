/*
  ==============================================================================

    MasterPanel.h
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "Sliders.h"
#include "MeterLookAndFeel.h"
#include "ff_meters/ff_meters.h"

//==============================================================================
/*
 */
class MasterPanel : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  MasterPanel(Parameters& parameters, foleys::LevelMeterSource& meterSource);
  ~MasterPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();
  
  std::function<void(void)> onRefToneOn = nullptr;
  std::function<void(void)> onRefToneOff = nullptr;

 private:
  // Bookkeeping
  Parameters& mParameters;
  std::atomic<bool> mParamHasChanged;
  ParamCommon* mCurSelectedParams;
  juce::Colour mParamColour;
  MeterLookAndFeel mMeterLookAndFeel;

  // Components
  juce::Label mLabelTitle;
  juce::Label mLabelMacros;
  CommonSlider mSliderGain;
  juce::Label mLabelGain;
  juce::ToggleButton mBtnRefTone;
  juce::Label mLabelRefTone;
  GlobalSlider mSliderMacro1;
  juce::Label mLabelMacro1;
  GlobalSlider mSliderMacro2;
  juce::Label mLabelMacro2;
  GlobalSlider mSliderMacro3;
  juce::Label mLabelMacro3;
  GlobalSlider mSliderMacro4;
  juce::Label mLabelMacro4;
  foleys::LevelMeter mMeter{foleys::LevelMeter::Default};


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterPanel)
};
