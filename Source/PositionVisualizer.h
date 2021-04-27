/*
  ==============================================================================

    PositionVisualizer.h
    Created: 27 Apr 2021 9:17:07pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "GranularSynth.h"

//==============================================================================
/*
 */
class PositionVisualizer : public juce::AnimatedAppComponent {
 public:
  PositionVisualizer();
  ~PositionVisualizer() override;

  void setPositions(std::vector<GranularSynth::GrainPosition> gPositions);

  void paint(juce::Graphics&) override;
  void resized() override;
  void update() override{};

 private:
  static constexpr auto ITEM_HEIGHT = 40;

  std::vector<GranularSynth::GrainPosition> mGPositions;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionVisualizer)
};
