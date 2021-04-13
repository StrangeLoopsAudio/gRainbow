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
  void setIsPlaying(bool isPlaying) { mShouldPlayTest = isPlaying; }
  void setSampleRate(double sr) { mSampleRate = sr; }
  void setDuration(int duration) { mGrainDuration = duration; }

  void process(juce::AudioBuffer<float>* blockBuffer);
  std::vector<int> playNote(int midiNote, int k); // Returns vector of fft time positions where freq energy is high

  //==============================================================================
  void run() override;

private:
  typedef struct GrainNote
  {
    int midiNote;
    int voiceNum;
    GrainNote(int midiNote, int voiceNum) : midiNote(midiNote), voiceNum(voiceNum) {}
  } GrainNote;

  juce::AudioBuffer<float> *mFileBuffer = nullptr;
  juce::MidiKeyboardState& mMidiState;
  std::vector<std::vector<float>>* mFftData = nullptr;

  /* Grain control */
  juce::Array<Grain> mGrains;
  long mTotalSamps;
  juce::Array<GrainNote> mActiveNotes;
  bool mShouldPlayTest = false;
  double mSampleRate;

  /* Grain parameters */
  int mGrainDuration; // Grain duration in samples
};