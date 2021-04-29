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
    float posRatio;
    float pbRate;  // timestretching ratio based on frequency offset from target
    float gain;    // Gain of spectrogram at that position
    bool isEnabled = false;
    bool solo = false;
    GrainPosition() : posRatio(0.0f), pbRate(1.0f), gain(0.0f) {}
    GrainPosition(float posRatio, float pbRate, float gain)
        : posRatio(posRatio), pbRate(pbRate), gain(gain) {}
    bool operator<(const GrainPosition& other) const {
      bool isFurther =
          (std::abs(1.0f - pbRate) > std::abs(1.0f - other.pbRate));
      return isFurther;
    }
  } GrainPosition;

  GrainPositionFinder() {}
  ~GrainPositionFinder() {}

  void setPitches(std::vector<PitchDetector::Pitch>* pitches) {
    mGPositions.clear();
    mPitches = pitches;
  }
  std::vector<GrainPositionFinder::GrainPosition> findPositions(int k,
                                                                int midiNote);

  void updatePosition(int midiNote, GrainPositionFinder::GrainPosition gPos);

 private:
  static constexpr auto TIMESTRETCH_RATIO = 1.0594f;

  std::vector<PitchDetector::Pitch>* mPitches = nullptr;
  juce::HashMap<int, std::vector<GrainPosition>> mGPositions;

  void pushPositions(int midiNote, std::vector<GrainPosition> gPositions);
};