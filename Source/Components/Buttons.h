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

class MapButton : public juce::TextButton, public juce::Timer {
public:
  MapButton(Parameters& parameters, ModSource& modSource): mParameters(parameters), mModSource(modSource) {
    setClickingTogglesState(true);
    setButtonText("MAP");
    startTimer(300);
    onClick = [this]() {
      mIsBright = true;
      setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour);
      mParameters.mappingModSource = getToggleState() ? &mModSource : nullptr;
    };
  }
  ~MapButton() {
    stopTimer();
  }
  
  void visibilityChanged() override {
    if (!isVisible()) resetMappingStatus();
  }
  
  void timerCallback() override {
    if (getToggleState()) {
      mIsBright = !mIsBright;
      setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour.withLightness(getLightness()));
      repaint();
    }
  }
  
private:
  float getLightness() {
    return mIsBright ? 1.0f : 0.8f;
  }
  
  void resetMappingStatus() {
    setToggleState(false, juce::dontSendNotification);
    mIsBright = true;
    setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour);
    mParameters.mappingModSource = nullptr;
  }
  
  Parameters& mParameters;
  ModSource& mModSource;
  bool mIsBright = false;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MapButton)
};
