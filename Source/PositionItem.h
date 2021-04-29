/*
  ==============================================================================

    PositionItem.h
    Created: 29 Apr 2021 8:18:02pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GrainPositionFinder.h"

//==============================================================================
/*
 */
class PositionItem : public juce::Component {
 public:
  PositionItem(GrainPositionFinder::GrainPosition gPos);
  ~PositionItem() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void addListener(juce::Button::Listener *listener);

  juce::ToggleButton btnEnabled;
  juce::ToggleButton btnSolo;
 private:

  /* UI Components */
  juce::Label mLabelDesc;
  juce::Label mLabelEnabled;
  juce::Label mLabelSolo;


  GrainPositionFinder::GrainPosition mGPos;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionItem)
};
