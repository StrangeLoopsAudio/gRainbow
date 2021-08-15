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
    SHAPE,    // Grain env ramp width
    TILT,    // Grain env center tilt
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

  void getPresetParamsXml(juce::MemoryBlock& destData);
  void setPresetParamsXml(const void* data, int sizeInBytes);

  double getSampleRate() { return mSampleRate; }
  const juce::AudioBuffer<float>& getAudioBuffer() { return mFileBuffer; }
  juce::MidiKeyboardState& getKeyboardState() { return mKeyboardState; }

  // Callback functions
  std::function<void(Utils::PitchClass pitchClass, bool isNoteOn)>
      onNoteChanged = nullptr;
  std::function<void(std::vector<std::vector<float>>* buffer,
                     ArcSpectrogram::SpecType type)>
      onBufferProcessed = nullptr;
  std::function<void(double progress)>
      onProgressUpdated = nullptr;

  void processFile(juce::AudioBuffer<float>* audioBuffer, double sampleRate);
  ParamsNote& getParamsNote() { return mParamsNote; }
  ParamGlobal& getParamGlobal() { return mParamGlobal; }
  ParamUI& getParamUI() { return mParamUI; }
  void resetParameters();
  int incrementPosition(int boxNum, bool lookRight);
  std::vector<ParamCandidate*> getActiveCandidates();

  void setNoteOn(Utils::PitchClass pitchClass);
  void setNoteOff(Utils::PitchClass pitchClass);

 private:
  // DSP constants
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 2048;
  // Param bounds
  static constexpr auto MIN_RATE_RATIO = .25f;
  static constexpr auto MAX_RATE_RATIO = 1.0f;
  static constexpr auto MIN_CANDIDATE_SALIENCE = 0.5f;
  static constexpr auto MAX_GRAINS = 20; // Max grains active at once

  typedef struct GrainNote {
    Utils::PitchClass pitchClass;
    Utils::EnvelopeADSR ampEnv;
    std::array<Utils::EnvelopeADSR, NUM_GENERATORS> genAmpEnvs;
    juce::Array<Grain> grains; // Active grains for note
    std::vector<float>
        grainTriggers;  // Keeps track of triggering grains from each
                          // generator
    GrainNote(Utils::PitchClass pitchClass,
              Utils::EnvelopeADSR ampEnv)
        : pitchClass(pitchClass),
          ampEnv(ampEnv) {
      grains.ensureStorageAllocated(MAX_GRAINS);
      // Initialize grain triggering timestamps
      for (int i = 0; i < NUM_GENERATORS; ++i) {
        grainTriggers.push_back(-1); // Trigger first set of grains right away
        genAmpEnvs[i].noteOn(ampEnv.noteOnTs); // Set note on for each position as well
      }
    }
  } GrainNote;

  // DSP pre-processing
  PitchDetector mPitchDetector;
  Fft mFft;

  // Bookkeeping
  juce::AudioBuffer<float> mFileBuffer;
  double mSampleRate;
  juce::MidiKeyboardState mKeyboardState;
  bool mIsProcessingComplete = false;
  double mLoadingProgress = 0.0;

  // Grain control
  long mTotalSamps;
  juce::Array<GrainNote, juce::CriticalSection> mActiveNotes;
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::C;

  // Parameters
  ParamsNote mParamsNote;
  ParamGlobal mParamGlobal;
  ParamUI mParamUI;

  void handleGrainAddRemove(int blockSize);
  void createCandidates(
      juce::HashMap<Utils::PitchClass, std::vector<PitchDetector::Pitch>>&
          detectedPitches);
};