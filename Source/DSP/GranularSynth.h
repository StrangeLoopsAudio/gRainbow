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
  static constexpr int MAX_GRAINS = 150;  // Max grains active at once
  class GrainPool {
  public:
    GrainPool() {
      // Reserve to avoid re-alloc in process block
      mGrainsActive.reserve(MAX_GRAINS);
      mGrainsInactive.reserve(MAX_GRAINS);
      for (Grain& g : mGrains) {
        mGrainsInactive.push_back(&g);
      }
    }
    ~GrainPool() {}

    Grain* getNextAvailableGrain() {
      if (mGrainsInactive.size() > 0) {
        mGrainsActive.push_back(mGrainsInactive.back());
        mGrainsInactive.pop_back();
        return mGrainsActive.back();
      }
      return nullptr;
    }
    void reclaimExpiredGrains(int totalSamples) {
      
      for (auto it = mGrainsActive.begin(); it != mGrainsActive.end(); ) {
        Grain* g = *it;
        if (totalSamples > (g->trigTs + g->duration)) {
          g->isActive = false;
          mGrainsInactive.push_back(*it);
          it = mGrainsActive.erase(it);
        } else {
          it++;
        }
      }
    }
    
    int getNumUsedGrains() {
      return mGrainsActive.size();
    }

  private:
    std::array<Grain, MAX_GRAINS> mGrains;
    std::vector<Grain*> mGrainsActive;
    std::vector<Grain*> mGrainsInactive;
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
  Utils::Result loadAudioFile(juce::File file);
  Utils::Result loadPreset(juce::File file);
  Utils::Result loadPreset(juce::MemoryBlock& fromBlock);
  Utils::Result savePreset(juce::File file);
  Utils::Result savePreset(juce::MemoryBlock& intoBlock);

  void trimAndExtractPitches(juce::Range<double> range);
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
  
  int getNumUsedGrains() {
    return mGrainPool.getNumUsedGrains();
  }

 private:
  // DSP constants
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 4096;  // Larger because don't need high resolution for spectrogram
  static constexpr double DEFAULT_BPM = 120.0f;
  static constexpr int DEFAULT_BEATS_PER_BAR = 4;
  // Param bounds
  static constexpr float MIN_CANDIDATE_SALIENCE = 0.5f;
  static constexpr int MAX_MIDI_NOTE = 127;
  static constexpr double DEFAULT_SAMPLE_RATE = 48000;  // Sample rate to use before it's officially set in prepareToPlay()
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
  double mSampleRate = DEFAULT_SAMPLE_RATE;
  juce::MidiKeyboardState mKeyboardState;
  juce::AudioFormatManager mFormatManager;
  float mBarsPerSec = (1.0f / DEFAULT_BPM) * 60.0f * DEFAULT_BEATS_PER_BAR;
  float mCurPitchBendSemitones = 0.0f; // Current pitch bend value from MIDI in semitones

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
  void makePitchSpec();
  void createCandidates();
};
