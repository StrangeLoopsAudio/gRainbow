/*
  ==============================================================================

    FilterControl.h
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Utils.h"

//==============================================================================
/*
*/
class FilterControl : public juce::Component {
 public:
  FilterControl();
  ~FilterControl() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void setActive(bool isActive);

 private:
  bool mIsActive = false;
  Utils::GeneratorColour mColour;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterControl)
};
