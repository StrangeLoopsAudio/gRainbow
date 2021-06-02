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
  GranularSynth();
  ~GranularSynth();

  void setFileBuffer(juce::AudioBuffer<float>* buffer, double sr);
  void setDuration(float duration) { mDuration = duration; }
  void setRate(float rate) { mRate = rate; }
  void setDiversity(float diversity) { mDiversity = diversity; }

  void process(juce::AudioBuffer<float>* blockBuffer);
  void setPositions(int midiNote,
                    std::vector<GrainPositionFinder::GrainPosition> gPositions);
  void stopNote(int midiNote);

  //==============================================================================
  void run() override;

 private:
  static constexpr auto MAX_DURATION = 0.6f;
  static constexpr auto MIN_DURATION = 0.05f;
  static constexpr auto MIN_RATE = 10.f;  // Grains per second
  static constexpr auto MAX_RATE = 40.f;

  typedef struct GrainNote {
    int midiNote;
    int curPos = 0;
    std::vector<GrainPositionFinder::GrainPosition> positions;
    GrainNote(int midiNote, int startPos,
              std::vector<GrainPositionFinder::GrainPosition> positions)
        : midiNote(midiNote), curPos(startPos), positions(positions) {}
  } GrainNote;

  juce::AudioBuffer<float>* mFileBuffer = nullptr;
  std::vector<GrainNote> mActiveGrains;
  std::array<float, 512> mGaussianEnv;

  /* Grain control */
  juce::Array<Grain> mGrains;
  long mTotalSamps;
  juce::Array<GrainNote> mActiveNotes;
  double mSampleRate;

  /* Grain parameters */
  float mDuration = 0.1;  // Grain duration normalized to 0-1
  float mDiversity =
      0.0;             // Extracts number of positions to find for freq match
  float mRate = 0.1;   // Grain rate normalized to 0-1
  float mNextGrainTs;  // Timestamp when next grain should be generated

  // Generate gaussian envelope to be used for each grain
  void generateGaussianEnvelope();
  int getNextPosition(GrainNote& gNote);
  int getStartPosition(
      std::vector<GrainPositionFinder::GrainPosition>& gPositions);
};