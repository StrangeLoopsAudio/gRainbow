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
class PositionMarker : public juce::Button {
 public:
  PositionMarker(GrainPositionFinder::GrainPosition gPos, juce::Colour colour);
  ~PositionMarker() override;

  void paintButton(juce::Graphics &g, bool shouldDrawButtonAsHighlighted,
                   bool shouldDrawButtonAsDown) override;
  void clicked() override;

  GrainPositionFinder::GrainPosition getGrainPosition() { return mGPos; }

 private:
  static constexpr auto RECT_RATIO = 0.66;

  GrainPositionFinder::GrainPosition mGPos;
  juce::Colour mColour;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionMarker)
};
