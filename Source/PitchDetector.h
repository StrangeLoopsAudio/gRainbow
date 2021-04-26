/*
  ==============================================================================

    PitchDetector.h
    Created: 24 Apr 2021 2:07:10pm
    Author:  brady

    Pitch detection algorithm using Harmonic Product Spectrum (HPS)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Fft.h"

class PitchDetector {
 public:
  static constexpr auto MIN_MIDINOTE = 43;
  static constexpr auto MAX_MIDINOTE = 91;
  
  PitchDetector();
  ~PitchDetector() {}

  typedef struct Pitch {
    float freq;
    float posRatio;
    float gain;
    Pitch(float freq, float posRatio, float gain)
        : freq(freq), posRatio(posRatio), gain(gain) {}
  } Pitch;

  void processBuffer(juce::AudioBuffer<float>& fileBuffer, double sampleRate);
  std::vector<Pitch>& getPitches() { return mPitches; }

 private:
  static constexpr auto FFT_SIZE = 1024;
  static constexpr auto HOP_SIZE = 512;
  static constexpr auto NUM_HPS_HARMONICS = 2;
  static constexpr auto DETECTION_THRESHOLD = 0.1f;
  static constexpr auto DETECTION_SPREAD = 0.02f;

  Fft mFft;
  std::vector<Pitch> mPitches;

  void updateHps(double sampleRate);
};