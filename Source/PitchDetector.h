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

  typedef struct Pitch {
    float freq;
    float posRatio;
    float gain;
    Pitch(float freq, float posRatio, float gain)
        : freq(freq), posRatio(posRatio), gain(gain) {}
  } Pitch;

  std::function<void(std::vector<std::vector<float>>&)> onPitchesUpdated =
      nullptr;

  void processBuffer(juce::AudioBuffer<float>* fileBuffer, double sampleRate);
  std::vector<Pitch>& getPitches() { return mPitches; }

  void run() override;

 private:
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 512;
  static constexpr auto DB_THRESH = -100;
  static constexpr auto REF_FREQ = 440;
  static constexpr auto NUM_PITCH_CLASSES = 12;
  static constexpr auto HPCP_WINDOW_LEN = 1.34f;
  static constexpr auto NUM_HPS_HARMONICS = 2;
  static constexpr auto DETECTION_THRESHOLD = 0.05f;
  static constexpr auto DETECTION_SPREAD = 0.02f;
  static constexpr auto MIN_FREQ = 100;
  static constexpr auto MAX_FREQ = 5000;
  
  typedef struct Peak {
    float freq;
    float gain;
    Peak(float freq, float gain) : freq(freq), gain(gain) {}
  } Peak;

  juce::AudioBuffer<float>* mFileBuffer = nullptr;
  Fft mFft;
  double mSampleRate;
  std::vector<std::vector<Peak>> mPeaks;
  std::vector<std::vector<float>> mHPCP;  // harmonic pitch class profile
  std::vector<Pitch> mPitches;

  void computeHPCP();
  Peak interpolatePeak(int frame, int bin);
};