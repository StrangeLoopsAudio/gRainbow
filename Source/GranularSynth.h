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
    GAIN,     // Max amplitude
    ATTACK,   // Position env attack
    DECAY,    // Position env decay
    SUSTAIN,  // Position env sustain
    RELEASE   // Position env release
  };

  GranularSynth();
  ~GranularSynth();

  void setFileBuffer(juce::AudioBuffer<float>* buffer, double sr);
  void setPitches(juce::HashMap<Utils::PitchClass,
                                std::vector<PitchDetector::Pitch>>* pitches);
  std::vector<GrainPositionFinder::GrainPosition> getCurrentPositions() {
    return mCurPositions;
  }
  Utils::GeneratorParams getGeneratorParams(Utils::GeneratorColour colour);
  Utils::GlobalParams getGlobalParams() { return mGlobalParams; }
  int getNumFoundPositions() {
    return mPositionFinder.findPositions(Utils::MAX_POSITIONS, mCurPitchClass)
        .size();
  }
  void resetParameters();
  int incrementPosition(int boxNum, bool lookRight);

  void process(juce::AudioBuffer<float>& buffer);
  void setNoteOn(Utils::PitchClass pitchClass);
  void setNoteOff(Utils::PitchClass pitchClass);
  void updateGeneratorStates(std::vector<bool> genStates);
  void updateGeneratorParameter(Utils::GeneratorColour colour, ParameterType param,
                              float value);
  void updateGlobalParameter(ParameterType param,
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
  // Param defaults
  static constexpr auto PARAM_PITCH_DEFAULT = 0.5f;
  static constexpr auto PARAM_POSITION_DEFAULT = 0.5f;
  static constexpr auto PARAM_SHAPE_DEFAULT = 0.5f;
  static constexpr auto PARAM_RATE_DEFAULT = 0.5f;
  static constexpr auto PARAM_DURATION_DEFAULT = 0.5f;
  static constexpr auto PARAM_GAIN_DEFAULT = 0.8f;
  static constexpr auto PARAM_ATTACK_DEFAULT = 0.2f;
  static constexpr auto PARAM_DECAY_DEFAULT = 0.2f;
  static constexpr auto PARAM_SUSTAIN_DEFAULT = 0.8f;
  static constexpr auto PARAM_RELEASE_DEFAULT = 0.5f;

  typedef struct GrainNote {
    Utils::PitchClass pitchClass;
    std::vector<GrainPositionFinder::GrainPosition> positions;
    Utils::EnvelopeState envState = Utils::EnvelopeState::ATTACK;
    float ampEnvLevel = 0.0f;  // Current amplitude envelope level
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
  std::vector<GrainPositionFinder::GrainPosition> mCurPositions;
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::C;

  /* Global parameters */
  Utils::GlobalParams mGlobalParams;

  /* Grain position parameters */
  std::array<std::array<Utils::GeneratorParams, Utils::GeneratorColour::NUM_GEN>,
             Utils::PitchClass::COUNT>
      mNoteSettings;
  std::array<float, Utils::GeneratorColour::NUM_GEN>
      mGrainTriggersMs;  // Keeps track of triggering grains from each position
  Utils::GeneratorColour mNextPositionToPlay = Utils::GeneratorColour::BLUE;

  // Generate gaussian envelope to be used for each grain
  std::vector<float> generateGrainEnvelope(float shape);
  // Returns maximum release time out of all positions in samples
  long getMaxReleaseTime();
  void updateCurPositions();
  void updateEnvelopeState(GrainNote& gNote);
};