/*
  ==============================================================================

    PitchDetector.h
    Created: 24 Apr 2021 2:07:10pm
    Author:  brady

    Pitch detection algorithm using HPCP algorithm from:

    TONAL DESCRIPTION OF MUSIC AUDIO SIGNALS
    Gutierrez 2006

    and

    Melody Extraction Using Chroma-Level Note Tracking and Pitch Mapping
    Weiwei Zhang , Zhe Chen and Fuliang Yin 2018

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Fft.h"

class PitchDetector : juce::Thread {
 public:
  static constexpr auto MIN_MIDINOTE = 43;
  static constexpr auto MAX_MIDINOTE = 91;

  PitchDetector();
  ~PitchDetector();

  static enum PitchClass { NONE = -1, C, Cs, D, Ds, E, F, Fs, G, Gs, A, As, B };

  typedef struct Pitch {
    PitchClass pitchClass;
    float posRatio;
    float gain;
    Pitch(PitchClass pitchClass, float posRatio, float gain)
        : pitchClass(pitchClass), posRatio(posRatio), gain(gain) {}
  } Pitch;

  std::function<void(std::vector<std::vector<float>>&)> onPitchesUpdated =
      nullptr;

  void processBuffer(juce::AudioBuffer<float>* fileBuffer, double sampleRate);
  std::vector<Pitch>& getPitches() { return mPitches; }

  void run() override;

 private:
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 512;
  static constexpr auto DB_THRESH = -80;
  static constexpr auto REF_FREQ = 440;
  static constexpr auto MAX_PEAKS = 60;
  static constexpr auto NUM_PITCH_CLASSES = 12;
  static constexpr auto HPCP_WINDOW_LEN = 1.34f;
  static constexpr auto NUM_HARMONIC_WEIGHTS = 3;
  static constexpr auto HARMONIC_PRECISION = 0.00001;
  static constexpr auto MAGNITUDE_THRESHOLD = 0.00001;
  static constexpr auto MIN_FREQ = 100;
  static constexpr auto MAX_FREQ = 3500;

  typedef struct Peak {
    float binPos; // Bin pos from 0-1
    float gain;
    Peak(float binPos, float gain) : binPos(binPos), gain(gain) {}
  } Peak;

  typedef struct HarmonicWeight {
    float semitone;
    float gain;
    HarmonicWeight(int semitone, float harmonicStrength)
        : semitone(semitone), gain(harmonicStrength) {}
  } HarmonicWeight;

  juce::AudioBuffer<float>* mFileBuffer = nullptr;
  Fft mFft;
  double mSampleRate;
  std::vector<std::vector<float>> mHPCP;  // harmonic pitch class profile
  std::vector<std::vector<float>> mPitchesTest;
  std::vector<HarmonicWeight> mHarmonicWeights;
  std::vector<Pitch> mPitches;

  void computeHPCP();
  void estimatePitches();
  Peak interpolatePeak(int frame, int bin);
  void interpolatePeak(const float leftVal, const float middleVal,
                       const float rightVal, int currentBin, float& resultVal,
                       float& resultBin) const;
  std::vector<Peak> getPeaks(int frame);
  void initHarmonicWeights();
};