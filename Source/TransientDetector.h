/*
  ==============================================================================

    TransientDetector.h
    Created: 21 Apr 2021 9:34:38pm
    Author:  brady
    Juce implementation of method from:
    A TRANSIENT DETECTION ALGORITHM FOR AUDIO USING ITERATIVE
    ANALYSIS OF STFT
    Thoshkahna, Nsabimana, Ramakrishnan
    https://ismir2011.ismir.net/papers/PS2-6.pdf

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TransientDetector {
 public:
  TransientDetector();
  ~TransientDetector() {}

  typedef struct Transient {
    float posRatio;
    float confidence;
  } Transient;

  void loadBuffer(juce::AudioBuffer<float>& fileBuffer);
  std::vector<std::vector<float>>& getTransients() { return mP; }

 private:
  static constexpr auto FFT_ORDER = 9;
  static constexpr auto FFT_SIZE = 1 << FFT_ORDER;
  static constexpr auto PARAM_V = 3;    // Vertical neighbor smoothing
  static constexpr auto PARAM_B = 2;    // Transient sensitivity
  static constexpr auto PARAM_TAU = 3;  // Transient frame spread
  static constexpr auto PARAM_DELTA = 0.1f;  // Percentage to chip away each iteration
  static constexpr auto PARAM_THRESH = FFT_SIZE / 6.0f;  // Threshold value
  static constexpr auto PARAM_M = 10;  // Number of iterations

  juce::dsp::FFT mForwardFFT;
  std::array<float, FFT_SIZE * 2> mFftFrame;
  std::vector<std::vector<float>> mX; // FFT data normalized from 0.0-1.0
  std::vector<std::vector<float>> mF; // Transient edges
  std::vector<std::vector<float>> mLambda; // Threshold values
  std::vector<float>              mSigmaGamma; // Thresh hit array
  std::vector<std::vector<float>> mP; // Transient spectrum
  std::vector<std::vector<float>> mTransientMarkers;  // Transient spectrum booleans
  std::vector<Transient> mTransients; 

  void updateFft(juce::AudioBuffer<float>& fileBuffer);
  void iterateFunctions();
  void retrieveTransients();
  float F(int frame, int bin);
  float LAMBDA(int frame, int bin);
  int GAMMA(int frame, int bin);
  float P(int frame, int bin);
  float XMod(int frame, int bin);
};