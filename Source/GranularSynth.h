/*
  ==============================================================================

    GranularSynth.h
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Grain.h"
#include "GrainPositionFinder.h"
#include "PitchDetector.h"
#include "Utils.h"

class GranularSynth : juce::Thread {
 public:
  GranularSynth();
  ~GranularSynth();

  void setFileBuffer(juce::AudioBuffer<float>* buffer, double sr);
  void setRate(float rate) { mRate = rate; }
  void setDuration(float duration) { mDuration = duration; }

  void process(juce::AudioBuffer<float>* blockBuffer);
  void setPositions(PitchDetector::PitchClass pitchClass,
                    std::vector<GrainPositionFinder::GrainPosition> gPositions);
  void stopNote(PitchDetector::PitchClass pitchClass);

  //==============================================================================
  void run() override;

 private:
  static constexpr auto MIN_RATE = 10.f;  // Grains per second
  static constexpr auto MAX_RATE = 20.f;
  static constexpr auto MIN_DURATION_MS = 60.0f;
  static constexpr auto MAX_DURATION_MS = 300.0f;
  static constexpr auto MAX_GRAINS = 100; // Max grains active at once

  typedef struct GrainNote {
    PitchDetector::PitchClass pitchClass;
    std::vector<GrainPositionFinder::GrainPosition> positions;
    GrainNote(PitchDetector::PitchClass pitchClass,
              std::vector<GrainPositionFinder::GrainPosition> positions)
        : pitchClass(pitchClass), positions(positions) {}
  } GrainNote;

  juce::AudioBuffer<float>* mFileBuffer = nullptr;
  std::array<float, 512> mGaussianEnv;

  /* Grain control */
  juce::Array<Grain> mGrains;
  long mTotalSamps;
  juce::Array<GrainNote, juce::CriticalSection> mActiveNotes;
  double mSampleRate;

  /* Grain parameters */
  float mRate = 0.1;   // Grain rate normalized to 0-1
  float mDuration = 0.0f; // Grain duration from 0-1

  // Generate gaussian envelope to be used for each grain
  void generateGaussianEnvelope();
};