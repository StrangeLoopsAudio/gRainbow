/*
  ==============================================================================

    FxPanel.h
    Created: 15 Dec 2023 8:34:54pm
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
class FxPanel : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  FxPanel(Parameters& parameters);
  ~FxPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();

 private:
  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour;

  // Components

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FxPanel)
};
