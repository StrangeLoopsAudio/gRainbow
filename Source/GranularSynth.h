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

class GranularSynth : public juce::AudioProcessor, juce::Thread {
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

  //=====================start-inherited-functions================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;
  //=====================end-inherited-functions==================================

  double getSampleRate() { return mSampleRate; }
  juce::MidiKeyboardState& getKeyboardState() { return mKeyboardState; }

  // Callback functions
  std::function<void(Utils::PitchClass pitchClass, bool isNoteOn)>
      onNoteChanged = nullptr;
  std::function<void(std::vector<std::vector<float>>* buffer,
                     Utils::SpecType type)>
      onBufferProcessed = nullptr;
  std::function<void(double progress)>
      onProgressUpdated = nullptr;

  void processFile(juce::File file);
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
  // DSP constants
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 2048;
  // Param bounds
  static constexpr auto MAX_PITCH_ADJUST = 0.25; // In either direction, this equals one octave total
  static constexpr auto MAX_POS_ADJUST = 0.5f; // Max position adjust in terms of pitch duration
  static constexpr auto MIN_DURATION_MS = 60.0f;
  static constexpr auto MAX_DURATION_MS = 300.0f;
  static constexpr auto MIN_RATE_RATIO = .25f;
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
    std::vector<float>
        grainTriggersMs;  // Keeps track of triggering grains from each
                          // position
    GrainNote(Utils::PitchClass pitchClass,
              std::vector<Utils::GeneratorParams> genParams,
              std::vector<GrainPositionFinder::GrainPosition> positions,
              long ts)
        : pitchClass(pitchClass),
          positions(positions),
          noteOnTs(ts),
          noteOffTs(-1) {
      // Initialize grain triggering timestamps
      for (int i = 0; i < genParams.size(); ++i) {
        float durMs =
            juce::jmap(genParams[i].duration, MIN_DURATION_MS, MAX_DURATION_MS);
        grainTriggersMs.push_back(juce::jmap(1.0f - genParams[i].rate,
                                             durMs * MIN_RATE_RATIO,
                                             durMs * MAX_RATE_RATIO));
      }
    }
  } GrainNote;

  /* DSP pre-processing */
  PitchDetector mPitchDetector;
  Fft mFft;

  /* Bookkeeping */
  juce::AudioBuffer<float> mFileBuffer;
  double mSampleRate;
  juce::MidiKeyboardState mKeyboardState;
  bool mIsProcessingComplete = false;
  double mLoadingProgress = 0.0;
  juce::AudioFormatManager mFormatManager;

  /* Grain control */
  juce::Array<Grain> mGrains; // Active grains
  long mTotalSamps;
  juce::Array<GrainNote, juce::CriticalSection> mActiveNotes;
  GrainPositionFinder mPositionFinder;
  std::vector<GrainPositionFinder::GrainPosition> mCurPositions;
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::C;

  /* Global parameters */
  Utils::GlobalParams mGlobalParams;

  /* Grain generator parameters */
  std::array<std::array<Utils::GeneratorParams, Utils::GeneratorColour::NUM_GEN>,
             Utils::PitchClass::COUNT>
      mNoteSettings;

  // Generate gaussian envelope to be used for each grain
  std::vector<float> generateGrainEnvelope(float shape);
  // Returns maximum release time out of all positions in samples
  void updateCurPositions();
  void updateEnvelopeState(GrainNote& gNote);
};