/*
  ==============================================================================

    GranularSynth.h
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "ArcSpectrogram.h"
#include "Grain.h"
#include "Parameters.h"
#include "PitchDetector.h"
#include "Utils.h"

class GranularSynth : public juce::AudioProcessor {
 public:
  enum ParameterType {
    ENABLED,  // If position is enabled and playing grains
    SOLO,     // If position is solo'd
    PITCH_ADJUST,
    POSITION_ADJUST,
    SHAPE,     // Grain env ramp width
    TILT,      // Grain env center tilt
    RATE,      // Grain rate
    DURATION,  // Grain duration
    GAIN,      // Max amplitude
    ATTACK,    // Position env attack
    DECAY,     // Position env decay
    SUSTAIN,   // Position env sustain
    RELEASE    // Position env release
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

  void getPresetParamsXml(juce::MemoryBlock& destData);
  void setPresetParamsXml(const void* data, int sizeInBytes);

  double getSampleRate() { return mSampleRate; }
  const juce::AudioBuffer<float>& getAudioBuffer() { return mFileBuffer; }
  juce::MidiKeyboardState& getKeyboardState() { return mKeyboardState; }

  // Callback functions
  std::function<void(Utils::PitchClass pitchClass, bool isNoteOn)> onNoteChanged = nullptr;

  void processFile(juce::AudioBuffer<float>* audioBuffer, double sampleRate, bool preset);
  std::vector<Utils::SpecBuffer*> getProcessedSpecs() {
    return std::vector<Utils::SpecBuffer*>(mProcessedSpecs.begin(), mProcessedSpecs.end());
  }

  ParamsNote& getParamsNote() { return mParamsNote; }
  ParamGlobal& getParamGlobal() { return mParamGlobal; }
  double& getLoadingProgress() { return mLoadingProgress; }

  ParamUI& getParamUI() { return mParamUI; }
  void resetParameters();
  int incrementPosition(int boxNum, bool lookRight);
  std::vector<ParamCandidate*> getActiveCandidates();

 private:
  // DSP constants
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 2048;
  static constexpr auto DEFAULT_BPM = 120;
  // Param bounds
  static constexpr auto MIN_RATE_RATIO = .25f;
  static constexpr auto MAX_RATE_RATIO = 1.0f;
  static constexpr auto MIN_CANDIDATE_SALIENCE = 0.5f;
  static constexpr auto MAX_GRAINS = 20;  // Max grains active at once

  typedef struct GrainNote {
    Utils::PitchClass pitchClass;
    float velocity;
    Utils::EnvelopeADSR ampEnv;
    std::array<Utils::EnvelopeADSR, NUM_GENERATORS> genAmpEnvs;
    std::array<juce::Array<Grain>, NUM_GENERATORS> genGrains;  // Active grains for note per generator
    std::vector<float> grainTriggers;                       // Keeps track of triggering grains from
                                                            // each generator
    GrainNote(Utils::PitchClass pitchClass, float velocity, Utils::EnvelopeADSR ampEnv)
        : pitchClass(pitchClass), velocity(velocity), ampEnv(ampEnv) {
      // Initialize grain triggering timestamps
      for (int i = 0; i < NUM_GENERATORS; ++i) {
        genGrains[i].ensureStorageAllocated(MAX_GRAINS);
        grainTriggers.push_back(-1);            // Trigger first set of grains right away
        genAmpEnvs[i].noteOn(ampEnv.noteOnTs);  // Set note on for each position as well
      }
    }
  } GrainNote;

  // DSP-preprocessing
  PitchDetector mPitchDetector;
  Fft mFft;

  // Bookkeeping
  juce::AudioBuffer<float> mFileBuffer;
  std::array<Utils::SpecBuffer*, ParamUI::SpecType::COUNT> mProcessedSpecs;
  double mSampleRate;
  double mBlockSize;
  juce::MidiKeyboardState mKeyboardState;
  double mLoadingProgress = 0.0;

  // Grain control
  long mTotalSamps;
  juce::Array<GrainNote, juce::CriticalSection> mActiveNotes;
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::C;

  // Parameters
  ParamsNote mParamsNote;
  ParamGlobal mParamGlobal;
  ParamUI mParamUI;

  void setNoteOn(Utils::PitchClass pitchClass, float velocity);
  void setNoteOff(Utils::PitchClass pitchClass);
  void handleGrainAddRemove(int blockSize);
  void createCandidates(juce::HashMap<Utils::PitchClass, std::vector<PitchDetector::Pitch>>& detectedPitches);
};