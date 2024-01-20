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
#include "PitchDetection/BasicPitch.h"
#include "DSP/Fft.h"
#include "DSP/HPCP.h"
#include "Parameters.h"
#include "Utils/Utils.h"
#include "Utils/DSP.h"
#include "Utils/MidiNote.h"
#include <bitset>
#include "ff_meters/ff_meters.h"

class GranularSynth : public juce::AudioProcessor, public juce::MidiKeyboardState::Listener, public juce::Thread {
 public:
  class GrainPool {
  public:
    GrainPool() {}
    ~GrainPool() {}

    Grain* getNextAvailableGrain() {
      auto nextGrain = std::find_if(mGrains.begin(), mGrains.end(), [](Grain& g) { return !g.isActive; });
      if (nextGrain != mGrains.end()) {
        return &(*nextGrain);
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

  void run() override;

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
  Utils::Result loadPreset(juce::MemoryBlock& fromBlock);
  Utils::Result savePreset(juce::File file);
  Utils::Result savePreset(juce::MemoryBlock& intoBlock);

  void extractPitches();
  std::vector<Utils::SpecBuffer*> getProcessedSpecs() {
    return std::vector<Utils::SpecBuffer*>(mProcessedSpecs.begin(), mProcessedSpecs.end());
  }

  Parameters& getParams() { return mParameters; }
  ParamsNote& getParamsNote() { return mParameters.note; }
  ParamGlobal& getParamGlobal() { return mParameters.global; }
  ParamUI& getParamUI() { return mParameters.ui; }

  const juce::Array<Utils::MidiNote>& getMidiNotes() { return mMidiNotes; }
  std::vector<ParamCandidate*> getActiveCandidates();
  Utils::PitchClass getLastPitchClass() { return mLastPitchClass; }

  // Reference tone control
  void startReferenceTone(Utils::PitchClass pitchClass) {
    mReferenceTone.setFrequency(juce::MidiMessage::getMidiNoteInHertz(60 + pitchClass));
    mReferenceTone.setAmplitude(juce::Decibels::decibelsToGain(-20.0f));
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
  static constexpr int MAX_MIDI_NOTE = 127;
  static constexpr int MAX_GRAINS = 100;  // Max grains active at once
  static constexpr double INVALID_SAMPLE_RATE = -1.0;  // Max grains active at once
  static constexpr int MAX_PITCH_BEND_SEMITONES = 2;  // Max pitch bend semitones allowed

  typedef struct GrainNote {
    int pitch; // MIDI note number
    Utils::PitchClass pitchClass;
    float velocity;
    int removeTs = -1; // Timestamp when note is released
    std::array<Utils::EnvelopeADSR, NUM_GENERATORS> genAmpEnvs;
    std::array<juce::Array<Grain*>, NUM_GENERATORS> genGrains;  // Active grains for note per generator
    std::array<float, NUM_GENERATORS> grainTriggers;           // Keeps track of triggering grains from each generator
    // Pitch in MIDI note #, velocity from 0 to 1, ts as current sample timestamp
    GrainNote(int _pitch, float _velocity, int ts)
        : pitch(_pitch), pitchClass(Utils::getPitchClass(_pitch)), velocity(_velocity) {
      // Initialize grain triggering timestamps
      grainTriggers.fill(-1.0f);  // Trigger first set of grains right away
      noteOn(ts);
    }

    void noteOn(int ts) {
      for (size_t i = 0; i < NUM_GENERATORS; ++i) {
        genAmpEnvs[i].noteOn(ts);  // Set note on for each position as well
      }
      removeTs = -1;
    }
  } GrainNote;

  // DSP-preprocessing
  Fft mFft;
  HPCP mHPCP;
  BasicPitch mPitchDetector;
  juce::AudioBuffer<float> mDownsampledAudio; // Used for feeding into pitch detector
  Utils::SpecBuffer mPitchSpecBuffer;

  // Bookkeeping
  juce::AudioBuffer<float> mInputBuffer;  // incoming buffer from file or other source
  juce::AudioBuffer<float> mAudioBuffer;  // final buffer used for actual synth
  std::array<Utils::SpecBuffer*, ParamUI::SpecType::COUNT> mProcessedSpecs;
  double mSampleRate = INVALID_SAMPLE_RATE;
  juce::MidiKeyboardState mKeyboardState;
  juce::AudioFormatManager mFormatManager;
  bool mNeedsResample = false;
  float mBarsPerSec = (1.0f / DEFAULT_BPM) * 60.0f * DEFAULT_BEATS_PER_BAR;
  float mCurPitchBendSemitones = 0.0f; // Current pitch bend value from MIDI in semitones

  // Process block variables saved to avoid alloc every block, should not be used outside of it EVER
  ParamGenerator* mParamGenerator;
  ParamCandidate* mParamCandidate;
  float mGain, mAttack, mDecay, mSustain, mRelease, mGrainGain, mGenSampleValue;
  juce::AudioPlayHead* mPlayhead;
  double mBpm;
  int mBeatsPerBar;
  float mDurSec, mGrainRate, mGrainDuration, mGrainSync, mPitchAdjust, mPitchSpray, mPosAdjust, mPosSpray, mPanAdjust, mPanSpray,
  mShape, mTilt, mReverse, mDiv;
  int mOctaveAdjust;
  float mDurSamples, mPosSprayOffset, mPosOffset, mPosSamples, mPanSprayOffset, mPanOffset, mPitchSprayOffset, mPitchBendOffset,
  mPbRate, mTotalGain;
  juce::Random mRandom;
  // ------- avoid using these unless within processBlock^ -------

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

  void resampleSynthBuffer(juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer,
                           double inputSampleRate, double outputSampleRate, bool clearInput = false);
  void handleNoteOn(juce::MidiKeyboardState* state, int midiChannel, int midiNoteNumber, float velocity) override;
  void handleNoteOff(juce::MidiKeyboardState* state, int midiChannel, int midiNoteNumber, float velocity) override;
  void handleGrainAddRemove(int blockSize);
  void makePitchSpec();
  void createCandidates();
};
