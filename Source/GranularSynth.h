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

  enum ParameterType { ENABLED, SOLO, RATE, DURATION, GAIN };
  enum PositionColour { BLUE = 0, PURPLE, ORANGE, GREEN, NUM_POSITIONS };
  typedef struct PositionSettings {
    bool isEnabled = false;
    bool solo = false;
    float rate;
    float duration;
    float gain;
  } PositionSettings;

  GranularSynth();
  ~GranularSynth();

  void setFileBuffer(juce::AudioBuffer<float>* buffer, double sr);

  void process(juce::AudioBuffer<float>* blockBuffer);
  void setPositions(PitchDetector::PitchClass pitchClass,
                    std::vector<GrainPositionFinder::GrainPosition> gPositions);
  void updatePositionSettings(PositionColour colour, ParameterType param,
                              float value);
  void updatePositionSettings(PositionColour colour, ParameterType param,
                              bool value);
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

  /* Grain position parameters */
  std::array<PositionSettings, NUM_POSITIONS> mPositionSettings;
  std::array<float, NUM_POSITIONS>
      mGrainTriggersMs;  // Keeps track of triggering grains from each position
  PositionColour mNextPositionToPlay = PositionColour::BLUE;

  // Generate gaussian envelope to be used for each grain
  void generateGaussianEnvelope();
};