/*
  ==============================================================================

    EnvPanel.h
    Created: 15 Dec 2023 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "../Sliders.h"
#include "../Buttons.h"

//==============================================================================
/*
 */
class EnvPanel : public juce::Component,
public juce::AudioProcessorParameter::Listener,
public juce::Timer {
 public:
  EnvPanel(int modIdx, Parameters& parameters);
  ~EnvPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}
  void visibilityChanged() override;
  
  void timerCallback() override;

 private:
  // Bookkeeping
  Parameters& mParameters;
  EnvModSource& mModEnv;
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
  MapButton mBtnMap;

  // UI values saved on resize
  juce::Rectangle<float> mVizRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvPanel)
};
