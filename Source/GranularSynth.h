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

  enum ParameterType { ENABLED, SOLO, RATE, DURATION, GAIN, ATTACK, DECAY, SUSTAIN, RELEASE };
  enum PositionColour { BLUE = 0, PURPLE, ORANGE, GREEN, NUM_BOXES };
  typedef struct PositionParams {
    float rate;
    float duration;
    float gain;
    float attack;
    float decay;
    float sustain;
    float release;
    PositionParams() {}
    PositionParams(float rate, float duration, float gain, float attack,
                   float decay, float sustain, float release)
        : rate(rate),
          duration(duration),
          gain(gain),
          attack(attack),
          decay(decay),
          sustain(sustain),
          release(release) {}
  } PositionParams;

  GranularSynth();
  ~GranularSynth();

  void setFileBuffer(juce::AudioBuffer<float>* buffer, double sr);

  void process(juce::AudioBuffer<float>* blockBuffer);
  void setPositions(PitchDetector::PitchClass pitchClass,
                    std::vector<GrainPositionFinder::GrainPosition> gPositions);
  void updateParameters(PositionColour colour, PositionParams params);
  void updateParameter(PositionColour colour, ParameterType param,
                              float value);
  void stopNote(PitchDetector::PitchClass pitchClass);

  //==============================================================================
  void run() override;

 private:
  static constexpr auto MIN_RATE = 10.f;  // Grains per second
  static constexpr auto MAX_RATE = 20.f;
  static constexpr auto MIN_DURATION_MS = 60.0f;
  static constexpr auto MAX_DURATION_MS = 300.0f;
  static constexpr auto MIN_RATE_RATIO = .125f;
  static constexpr auto MAX_RATE_RATIO = 1.0f;
  static constexpr auto MIN_ATTACK_MS = 10.0f;
  static constexpr auto MAX_ATTACK_MS = 10000.0f;
  static constexpr auto MIN_DECAY_MS = 10.0f;
  static constexpr auto MAX_DECAY_MS = 10000.0f;
  static constexpr auto MIN_RELEASE_MS = 10.0f;
  static constexpr auto MAX_RELEASE_MS = 10000.0f;
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
  std::array<PositionParams, NUM_BOXES> mPositionSettings;
  std::array<float, NUM_BOXES>
      mGrainTriggersMs;  // Keeps track of triggering grains from each position
  PositionColour mNextPositionToPlay = PositionColour::BLUE;

  // Generate gaussian envelope to be used for each grain
  void generateGaussianEnvelope();
};