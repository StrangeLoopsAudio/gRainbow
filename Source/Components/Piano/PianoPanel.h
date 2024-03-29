/*
  ==============================================================================

    PianoPanel.h
    Created: 15 Dec 2023 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "RainbowKeyboard.h"
#include "WaveformPanel.h"
#include "Components/Sliders.h"

//==============================================================================
/*
 */
class PianoPanel : public juce::Component,
public Parameters::Listener,
public juce::AudioProcessorParameter::Listener,
public juce::Timer {
 public:
  PianoPanel(juce::MidiKeyboardState& state, Parameters& parameters);
  ~PianoPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void selectedCommonParamsChanged(ParamCommon* newParams) override;

  void timerCallback() override;

  // Components (public so that editor can access easily, this is more of a container than anything)
  WaveformPanel waveform;
  RainbowKeyboard keyboard;

 private:
  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoPanel)
};
