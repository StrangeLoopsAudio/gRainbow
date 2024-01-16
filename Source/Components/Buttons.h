/*
 ==============================================================================
 
 Sliders.h
 Created: 23 Dec 2023 8:34:54pm
 Author:  brady
 
 ==============================================================================
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"

class MapButton : public juce::TextButton,
public Parameters::Listener,
public juce::Timer {
public:
  MapButton(Parameters& parameters, ModSource& modSource): mParameters(parameters), mModSource(modSource) {
    setColour(juce::TextButton::ColourIds::buttonColourId, mModSource.colour);
    setClickingTogglesState(true);
    setButtonText("MAP");
    setTooltip("Once enabled, drag sliders to create modulations");
    startTimer(300);
    onClick = [this]() {
      mParameters.setMappingModSource(getToggleState() ? &mModSource : nullptr);
    };
    mParameters.addListener(this);
  }
  ~MapButton() {
    mParameters.removeListener(this);
    stopTimer();
  }
  
  void timerCallback() override {
    if (getToggleState()) {
      mIsBright = !mIsBright;
      setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour.brighter(mIsBright ? 0.2f : 0.0f));
      repaint();
    }
  }
  
  void mappingSourceChanged(ModSource* mod) override {
    if (mod == &mModSource) {
      mIsBright = true;
      setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour);
    } else {
      setToggleState(false, juce::dontSendNotification);
      mIsBright = true;
      setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour);
    }
  }
  
private:
  Parameters& mParameters;
  ModSource& mModSource;
  bool mIsBright = false;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MapButton)
};
