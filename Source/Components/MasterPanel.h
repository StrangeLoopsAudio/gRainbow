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
#include "Buttons.h"
#include "MeterLookAndFeel.h"
#include "ff_meters/ff_meters.h"

//==============================================================================
/*
 */
class MasterPanel : public juce::Component,
public Parameters::Listener,
public juce::AudioProcessorParameter::Listener,
public juce::Timer {
 public:
  MasterPanel(Parameters& parameters, foleys::LevelMeterSource& meterSource);
  ~MasterPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}
  
  void selectedCommonParamsChanged(ParamCommon* newParams) override;

  void timerCallback() override;

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
  ParamSlider mSliderMacro1;
  juce::Label mLabelMacro1;
  ParamSlider mSliderMacro2;
  juce::Label mLabelMacro2;
  ParamSlider mSliderMacro3;
  juce::Label mLabelMacro3;
  ParamSlider mSliderMacro4;
  juce::Label mLabelMacro4;
  MapButton mBtnMapMacro1;
  MapButton mBtnMapMacro2;
  MapButton mBtnMapMacro3;
  MapButton mBtnMapMacro4;
  foleys::LevelMeter mMeter{foleys::LevelMeter::Default};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterPanel)
};
