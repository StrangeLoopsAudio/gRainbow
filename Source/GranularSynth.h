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

  enum ParameterType {
    ENABLED,  // If position is enabled and playing grains
    SOLO,     // If position is solo'd
    PITCH_ADJUST,
    POSITION_ADJUST,
    SHAPE,    // Grain curve shape
    RATE,     // Grain rate
    DURATION, // Grain duration
    GAIN,     // Grain max amplitude
    ATTACK,   // Position env attack
    DECAY,    // Position env decay
    SUSTAIN,  // Position env sustain
    RELEASE   // Position env release
  };
  typedef struct PositionParams {
    bool  isActive;
    float pitchAdjust;
    float posAdjust;
    float shape;
    float rate;
    float duration;
    float gain;
    float attack;
    float decay;
    float sustain;
    float release;
    PositionParams() {}
    PositionParams(bool isActive, float pitchAdjust, float posAdjust,
                   float shape, float rate, float duration, float gain,
                   float attack, float decay, float sustain, float release)
        : isActive(isActive),
          shape(shape),
          rate(rate),
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
  void setPitches(juce::HashMap<Utils::PitchClass,
                                std::vector<PitchDetector::Pitch>>* pitches);
  std::vector<GrainPositionFinder::GrainPosition> getCurrentPositions() {
    return mCurPositions;
  }
  std::vector<int> getBoxPositions();
  int getNumFoundPositions() {
    return mPositionFinder.findPositions(Utils::MAX_POSITIONS, mCurPitchClass)
        .size();
  }
  void resetPositions();
  int incrementPosition(int boxNum, bool lookRight);

  void process(juce::AudioBuffer<float>& buffer);
  void setNoteOn(Utils::PitchClass pitchClass);
  void setNoteOff(Utils::PitchClass pitchClass);
  void updatePositionStates(std::vector<bool> states);
  void updateParameters(Utils::PositionColour colour, PositionParams params);
  void updateParameter(Utils::PositionColour colour, ParameterType param,
                              float value);

  //==============================================================================
  void run() override;

 private:
  static constexpr auto MAX_PITCH_ADJUST = 0.25; // In either direction, this equals one octave total
  static constexpr auto MAX_POS_ADJUST = 0.5f; // Max position adjust in terms of pitch duration
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
    Utils::PitchClass pitchClass;
    std::vector<GrainPositionFinder::GrainPosition> positions;
    long noteOnTs;
    long noteOffTs;
    GrainNote(Utils::PitchClass pitchClass,
              std::vector<GrainPositionFinder::GrainPosition> positions,
              long ts)
        : pitchClass(pitchClass),
          positions(positions),
          noteOnTs(ts),
          noteOffTs(-1) {}
  } GrainNote;

  juce::AudioBuffer<float>* mFileBuffer = nullptr;

  /* Grain control */
  juce::Array<Grain> mGrains;
  long mTotalSamps;
  juce::Array<GrainNote, juce::CriticalSection> mActiveNotes;
  double mSampleRate;
  GrainPositionFinder mPositionFinder;
  std::array<std::array<int, Utils::PositionColour::NUM_POS>,
             Utils::PitchClass::COUNT>
      mPositions;
  std::vector<GrainPositionFinder::GrainPosition> mCurPositions;
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::NONE;

  /* Grain position parameters */
  std::array<PositionParams, Utils::PositionColour::NUM_POS> mPositionSettings;
  std::array<float, Utils::PositionColour::NUM_POS>
      mGrainTriggersMs;  // Keeps track of triggering grains from each position
  Utils::PositionColour mNextPositionToPlay = Utils::PositionColour::BLUE;

  // Generate gaussian envelope to be used for each grain
  std::vector<float> generateGrainEnvelope(float shape);
  // Returns maximum release time out of all positions in samples
  long getMaxReleaseTime();
  void updateCurPositions();
};