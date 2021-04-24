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

  void loadBuffer(juce::AudioBuffer<float>& fileBuffer, double sampleRate);
  std::vector<Pitch>& getPitches() { return mPitches; }

 private:
  static constexpr auto FFT_ORDER = 10;
  static constexpr auto FFT_SIZE = 1 << FFT_ORDER;
  static constexpr auto NUM_HPS_HARMONICS = 2;
  static constexpr auto DETECTION_THRESHOLD = 0.1f;
  static constexpr auto DETECTION_SPREAD = 0.02f;

  juce::dsp::FFT mForwardFFT;
  std::array<float, FFT_SIZE * 2> mFftFrame;
  std::vector<std::vector<float>> mFftData;  // FFT data normalized from 0.0-1.0
  std::vector<Pitch> mPitches;

  void updateFft(juce::AudioBuffer<float>& fileBuffer);
  void updateHps(double sampleRate);
};