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
class PositionMarker : public juce::TextButton {
 public:
  PositionMarker(GrainPositionFinder::GrainPosition gPos);
  ~PositionMarker() override;

  void paintButton(juce::Graphics &g, bool shouldDrawButtonAsHighlighted,
                   bool shouldDrawButtonAsDown) override;

  void clicked() override;

 private:
  static constexpr auto RECT_RATIO = 0.66;

  GrainPositionFinder::GrainPosition mGPos;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionMarker)
};
