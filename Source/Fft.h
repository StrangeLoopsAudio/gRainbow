/*
  ==============================================================================

    Fft.h
    Created: 26 Apr 2021 9:01:42pm
    Author:  brady

    FFT processing and utilities with overlapping windows and normalized values

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Fft {
 public:
  Fft(int windowSize, int hopSize)
      : mWindowSize(windowSize),
        mHopSize(hopSize),
        mForwardFFT(std::log2(windowSize)) {}
  ~Fft() {}

  void processBuffer(juce::AudioBuffer<float>& fileBuffer);
  std::vector<std::vector<float>>& getSpectrum() { return mFftData; }

 private:
  int mWindowSize;
  int mHopSize;
  juce::dsp::FFT mForwardFFT;
  int mFileLength = 0;  // Input buffer length in samples
  std::vector<float> mFftFrame;
  std::vector<std::vector<float>> mFftData;  // FFT data normalized from 0.0-1.0
};