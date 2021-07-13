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
  void setNoteOn(PitchDetector::PitchClass pitchClass,
                    std::vector<GrainPositionFinder::GrainPosition> gPositions);
  void setNoteOff(PitchDetector::PitchClass pitchClass);
  void updateParameters(Utils::PositionColour colour, PositionParams params);
  void updateParameter(Utils::PositionColour colour, ParameterType param,
                              float value);

  //==============================================================================
  void run() override;

 private:
  static constexpr auto MIN_RATE = 10.f;  // Grains per second
  static constexpr auto MAX_RATE = 20.f;
  static constexpr auto MIN_DURATION_MS = 60.0f;
  static constexpr auto MAX_DURATION_MS = 300.0f;
  static constexpr auto MIN_RATE_RATIO = .125f;
  static constexpr auto MAX_RATE_RATIO = 1.0f;
  static constexpr auto MIN_ATTACK_SEC = 0.01f;
  static constexpr auto MAX_ATTACK_SEC = 1.0f;
  static constexpr auto MIN_DECAY_SEC = 0.01f;
  static constexpr auto MAX_DECAY_SEC = 1.0f;
  static constexpr auto MIN_RELEASE_SEC = 0.01f;
  static constexpr auto MAX_RELEASE_SEC = 1.0f;
  static constexpr auto MAX_GRAINS = 100; // Max grains active at once

  typedef struct GrainNote {
    PitchDetector::PitchClass pitchClass;
    std::vector<GrainPositionFinder::GrainPosition> positions;
    long noteOnTs;
    long noteOffTs;
    GrainNote(PitchDetector::PitchClass pitchClass,
              std::vector<GrainPositionFinder::GrainPosition> positions, long ts)
        : pitchClass(pitchClass),
          positions(positions),
          noteOnTs(ts),
          noteOffTs(-1) {}
  } GrainNote;

  juce::AudioBuffer<float>* mFileBuffer = nullptr;
  std::array<float, 512> mGaussianEnv;

  /* Grain control */
  juce::Array<Grain> mGrains;
  long mTotalSamps;
  juce::Array<GrainNote, juce::CriticalSection> mActiveNotes;
  double mSampleRate;

  /* Grain position parameters */
  std::array<PositionParams, Utils::NUM_BOXES> mPositionSettings;
  std::array<float, Utils::NUM_BOXES>
      mGrainTriggersMs;  // Keeps track of triggering grains from each position
  Utils::PositionColour mNextPositionToPlay = Utils::PositionColour::BLUE;

  // Generate gaussian envelope to be used for each grain
  void generateGaussianEnvelope();
  // Returns maximum release time out of all positions in samples
  long getMaxReleaseTime();
};