/*
  ==============================================================================

    GranularSynth.h
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "Grain.h"
#include "PitchDetector.h"
#include "Parameters.h"
#include "Utils/Utils.h"
#include "Utils/MidiNote.h"
#include <bitset>
#include "ff_meters/ff_meters.h"

class GranularSynth : public juce::AudioProcessor, juce::MidiKeyboardState::Listener {
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
  
  class GrainPool {
  public:
    GrainPool() {}
    ~GrainPool() {}
    
    Grain* getNextAvailableGrain() {
      auto nextGrain = std::find_if(mGrains.begin(), mGrains.end(), [](Grain& g) { return !g.isActive; });
      if (nextGrain != mGrains.end()) {
        return nextGrain;
      }
      return nullptr;
    }
    void reclaimExpiredGrains(int totalSamples) {
      for (Grain& g : mGrains) {
        if (totalSamples > (g.trigTs + g.duration)) {
          g.isActive = false;
        }
      }
    }
    
  private:
    std::array<Grain, Utils::MAX_GRAINS> mGrains;
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
  
  foleys::LevelMeterSource& getMeterSource() { return mMeterSource; }

  void getPresetParamsXml(juce::MemoryBlock& destData);
  void setPresetParamsXml(const void* data, int sizeInBytes);

  double getSampleRate() { return mSampleRate; }
  juce::AudioBuffer<float>& getAudioBuffer() { return mAudioBuffer; }
  juce::MidiKeyboardState& getKeyboardState() { return mKeyboardState; }
  juce::AudioFormatManager& getFormatManager() { return mFormatManager; }
  juce::AudioBuffer<float>& getInputBuffer() { return mInputBuffer; }
  Utils::Result loadAudioFile(juce::File file, bool process);
  Utils::Result loadPreset(juce::File file);
  Utils::Result loadPreset(juce::String name, juce::MemoryBlock& block);
  // Audio buffer processing
  void resampleAudioBuffer(juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer, double inputSampleRate,
                           double outputSampleRate, bool clearInput = false);

  void trimAudioBuffer(juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer,
                       juce::Range<juce::int64> range, bool clearInput = false);

  void extractPitches();
  void extractSpectrograms();
  std::vector<Utils::SpecBuffer*> getProcessedSpecs() {
    return std::vector<Utils::SpecBuffer*>(mProcessedSpecs.begin(), mProcessedSpecs.end());
  }

  Parameters& getParams() { return mParameters; }
  ParamsNote& getParamsNote() { return mParameters.note; }
  ParamGlobal& getParamGlobal() { return mParameters.global; }
  ParamUI& getParamUI() { return mParameters.ui; }
  void resetParameters(bool fullClear = true);

  const juce::Array<Utils::MidiNote>& getMidiNotes() { return mMidiNotes; }
  std::vector<ParamCandidate*> getActiveCandidates();
  Utils::PitchClass getLastPitchClass() { return mLastPitchClass; }

  // Reference tone control
  void startReferenceTone(Utils::PitchClass pitchClass) {
    mReferenceTone.setFrequency(juce::MidiMessage::getMidiNoteInHertz(60 + pitchClass));
    mReferenceTone.setAmplitude(0.05f);
  }
  void stopReferenceTone() { mReferenceTone.setAmplitude(0.0f); }

 private:
  // DSP constants
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 4096;  // Larger because don't need high resolution for spectrogram
  static constexpr double DEFAULT_BPM = 120.0f;
  static constexpr int DEFAULT_BEATS_PER_BAR = 4;
  // Param bounds
  static constexpr float MIN_CANDIDATE_SALIENCE = 0.5f;
  static constexpr int MAX_GRAINS = 20;  // Max grains active at once
  static constexpr double INVALID_SAMPLE_RATE = -1.0;  // Max grains active at once

  typedef struct GrainNote {
    Utils::PitchClass pitchClass;
    float velocity;
    int removeTs = -1; // Timestamp when note is released
    std::array<Utils::EnvelopeADSR, NUM_GENERATORS> genAmpEnvs;
    std::array<juce::Array<Grain*>, NUM_GENERATORS> genGrains;  // Active grains for note per generator
    std::array<float, NUM_GENERATORS> grainTriggers;           // Keeps track of triggering grains from each generator
    GrainNote(Utils::PitchClass pitchClass_, float velocity_, Utils::EnvelopeADSR ampEnv)
        : pitchClass(pitchClass_), velocity(velocity_) {
      // Initialize grain triggering timestamps
      grainTriggers.fill(-1.0f);  // Trigger first set of grains right away
      for (size_t i = 0; i < NUM_GENERATORS; ++i) {
        genGrains[i].ensureStorageAllocated(MAX_GRAINS);
        genAmpEnvs[i].noteOn(ampEnv.noteOnTs);  // Set note on for each position as well
      }
    }
  } GrainNote;

  // DSP-preprocessing
  Fft mFft;
  PitchDetector mPitchDetector;

  // Bookkeeping
  juce::AudioBuffer<float> mInputBuffer;  // incoming buffer from file or other source
  juce::AudioBuffer<float> mAudioBuffer;  // final buffer used for actual synth
  std::array<Utils::SpecBuffer*, ParamUI::SpecType::COUNT> mProcessedSpecs;
  double mSampleRate = INVALID_SAMPLE_RATE;
  juce::MidiKeyboardState mKeyboardState;
  juce::AudioFormatManager mFormatManager;
  bool mNeedsResample = false;
  double mBpm = DEFAULT_BPM;
  int mBeatsPerBar = DEFAULT_BEATS_PER_BAR;

  // Reference sine tone
  juce::ToneGeneratorAudioSource mReferenceTone;

  // Grain control
  int mTotalSamps;
  juce::OwnedArray<GrainNote, juce::CriticalSection> mActiveNotes;
  GrainPool mGrainPool;

  Utils::PitchClass mLastPitchClass;
  // Holds all the notes being played. The synth is the only class who will write to it so no need to worrying about multiple
  // threads writing to it.
  // Currently the difference between "midiNotes" and "grainNotes" are midi is a subset mainly for the UI
  juce::Array<Utils::MidiNote> mMidiNotes;
  // Level meter source
  foleys::LevelMeterSource mMeterSource;

  // Parameters
  Parameters mParameters;

  void handleNoteOn(juce::MidiKeyboardState* state, int midiChannel, int midiNoteNumber, float velocity) override;
  void handleNoteOff(juce::MidiKeyboardState* state, int midiChannel, int midiNoteNumber, float velocity) override;
  void handleGrainAddRemove(int blockSize);
  void createCandidates(juce::HashMap<Utils::PitchClass, std::vector<PitchDetector::Pitch>>& detectedPitches);
};
