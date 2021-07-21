/*
  ==============================================================================

    PositionTabs.h
    Created: 21 Jul 2021 6:22:07pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Utils.h"

//==============================================================================
/*
 */
class PositionTabs : public juce::Component {
 public:
  PositionTabs();
  ~PositionTabs() override;

  void paint(juce::Graphics&) override;
  void resized() override;

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionTabs)
};
