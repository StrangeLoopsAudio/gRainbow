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
#include "Utils.h"

class GranularSynth: juce::Thread
{
public:

  typedef struct GrainPosition
  {
    float posRatio;
    float gain;
    float quality; // 0-1 for gain quality
    float pbRate; // timestretching ratio based on frequency offset from target
    GrainPosition(float posRatio, float gain, float quality, float pbRate)
      : posRatio(posRatio), gain(gain), quality(quality), pbRate(pbRate) {}
    bool operator<(const GrainPosition& other) const
    {
      return gain < other.gain;
    }
  } GrainPosition;

  GranularSynth(juce::MidiKeyboardState& midiState);
  ~GranularSynth();

  void setFileBuffer(juce::AudioBuffer<float>* buffer,
    std::vector<std::vector<float>>* fftData, 
    Utils::FftRanges *fftRanges, 
    double sr);
  void setDuration(float duration) { mDuration = duration; }
  void setRate(float rate) { mRate = rate; }
  void setDiversity(float diversity) { mDiversity = diversity; }

  void process(juce::AudioBuffer<float>* blockBuffer);
  std::vector<GrainPosition> playNote(int midiNote); // Returns vector of fft time positions where freq energy is high
  void stopNote(int midiNote);

  //==============================================================================
  void run() override;



private:
  static constexpr auto TIMESTRETCH_RATIO = 1.0594f;
  static constexpr auto MAX_DURATION = 0.6f;
  static constexpr auto MIN_DURATION = 0.05f;
  static constexpr auto MIN_DIVERSITY = 1.f;
  static constexpr auto MAX_DIVERSITY = 5.f;
  static constexpr auto MIN_RATE = 1.f; // Grains per second
  static constexpr auto MAX_RATE = 20.f;

  typedef struct GrainNote
  {
    int midiNote;
    int curPos = 0;
    std::vector<GrainPosition> positions;
    GrainNote(int midiNote, std::vector<GrainPosition> positions)
      : midiNote(midiNote), positions(positions) {}
  } GrainNote;

  juce::AudioBuffer<float> *mFileBuffer = nullptr;
  juce::MidiKeyboardState& mMidiState;
  std::vector<std::vector<float>>* mFftData = nullptr;
  Utils::FftRanges* mFftRanges = nullptr;
  std::vector<GrainNote> mActiveGrains;
  std::array<float, 512> mGaussianEnv;

  /* Grain control */
  juce::Array<Grain> mGrains;
  long mTotalSamps;
  juce::Array<GrainNote> mActiveNotes;
  double mSampleRate;

  /* Grain parameters */
  float mDuration = 0.1; // Grain duration normalized to 0-1
  float mDiversity = 0.0; // Extracts number of positions to find for freq match
  float mRate = 0.1; // Grain rate normalized to 0-1
  float mNextGrainTs; // Timestamp when next grain should be generated

  // Generate gaussian envelope to be used for each grain
  void generateGaussianEnvelope();
};