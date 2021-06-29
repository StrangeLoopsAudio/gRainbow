/*
  ==============================================================================

    GrainPositionFinder.h
    Created: 28 Apr 2021 8:55:40pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PitchDetector.h"

class GrainPositionFinder {
 public:
  typedef struct GrainPosition {
    PitchDetector::Pitch pitch;
    float pbRate;  // timestretching ratio based on frequency offset from target
    bool isActive = false;
    GrainPosition() : pbRate(1.0f) {}
    GrainPosition(PitchDetector::Pitch pitch, float pbRate)
        : pitch(pitch), pbRate(pbRate) {}
    bool operator<(const GrainPosition& other) const {
      bool isFurther =
          (std::abs(1.0f - pbRate) > std::abs(1.0f - other.pbRate));
      return isFurther;
    }
  } GrainPosition;

  GrainPositionFinder() {}
  ~GrainPositionFinder() {}

  void setPitches(juce::HashMap<PitchDetector::PitchClass,
                                std::vector<PitchDetector::Pitch>>* pitches) {
    mGPositions.clear();
    mPitches = pitches;
  }
  std::vector<GrainPositionFinder::GrainPosition> findPositions(
      int numPosToFind, PitchDetector::PitchClass pitchClass);

  void updatePosition(int midiNote, GrainPositionFinder::GrainPosition gPos);

 private:
  static constexpr auto TIMESTRETCH_RATIO = 1.0594f;

  juce::HashMap<PitchDetector::PitchClass, std::vector<PitchDetector::Pitch>>*
      mPitches = nullptr;
  juce::HashMap<int, std::vector<GrainPosition>> mGPositions;

  void pushPositions(PitchDetector::PitchClass pitchClass,
                     std::vector<GrainPosition> gPositions);
};