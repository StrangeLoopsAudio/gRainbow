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


class GranularSynth: juce::Thread
{
public:

  GranularSynth(juce::MidiKeyboardState& midiState);
  ~GranularSynth();

  void setFileBuffer(juce::AudioBuffer<float>* buffer, std::vector<std::vector<float>>* fftData);
  void setSampleRate(double sr) { mSampleRate = sr; }
  void setDuration(float duration) { mDuration = duration; }
  void setRate(float rate) { mRate = rate; }
  void setDiversity(float diversity) { mDiversity = diversity; }

  void process(juce::AudioBuffer<float>* blockBuffer);
  std::vector<float> playNote(int midiNote); // Returns vector of fft time positions where freq energy is high
  void stopNote(int midiNote);

  //==============================================================================
  void run() override;

private:

  static constexpr auto MAX_DURATION = 0.4;
  static constexpr auto MAX_DIVERSITY = 5;
  static constexpr auto MAX_RATE = 1000;

  typedef struct GrainNote
  {
    int midiNote;
    int curPos = 0;
    std::vector<float> positionRatios;
    GrainNote(int midiNote, std::vector<float> positionRatios)
      : midiNote(midiNote), positionRatios(positionRatios) {}
  } GrainNote;

  juce::AudioBuffer<float> *mFileBuffer = nullptr;
  juce::MidiKeyboardState& mMidiState;
  std::vector<std::vector<float>>* mFftData = nullptr;
  std::vector<GrainNote> mActiveGrains;

  /* Grain control */
  juce::Array<Grain> mGrains;
  long mTotalSamps;
  juce::Array<GrainNote> mActiveNotes;
  double mSampleRate;

  /* Grain parameters */
  float mDuration = 0.1; // Grain duration normalized to 0-1
  float mDiversity = 0.0; // Extracts number of positions to find for freq match
  float mRate = 0.1; // Grain rate normalized to 0-1
};