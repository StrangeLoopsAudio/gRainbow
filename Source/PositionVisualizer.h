/*
  ==============================================================================

    PositionVisualizer.h
    Created: 27 Apr 2021 9:17:07pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <functional>

#include "GrainPositionFinder.h"
#include "GranularSynth.h"

//==============================================================================
/*
 */
class PositionVisualizer : public juce::AnimatedAppComponent {
 public:
  PositionVisualizer();
  ~PositionVisualizer() override;

  std::function<void(int, GrainPositionFinder::GrainPosition)>
      onPositionUpdated = nullptr;

  void setPositions(
      int midiNote,
      std::vector<GrainPositionFinder::GrainPosition> gPositions) {
    mCurNote = midiNote;
    mGPositions = gPositions;
  }

  void paint(juce::Graphics&) override;
  void resized() override;
  void update() override{};

 private:
  static constexpr auto ITEM_HEIGHT = 40;

  int mCurNote = 0;
  std::vector<GrainPositionFinder::GrainPosition> mGPositions;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionVisualizer)
};
