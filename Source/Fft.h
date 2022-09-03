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
#include "Utils.h"

class Fft : public juce::Thread {
 public:
  Fft(int windowSize, int hopSize);
  ~Fft();

  void run() override;

  void process(const juce::AudioBuffer<float>* audioBuffer);
  const Utils::SpecBuffer& getSpectrum() { return mFftData; }

  std::function<void(Utils::SpecBuffer& spectrum)> onProcessingComplete = nullptr;

 private:
  // values passed in at creation time
  int mWindowSize;
  int mHopSize;
  juce::dsp::FFT mForwardFFT;
  juce::dsp::WindowingFunction<float> mWindowEnvelope;

  // pointer to buffer to read from
  const juce::AudioBuffer<float>* mInputBuffer = nullptr;

  // processed data
  std::vector<float> mFftFrame;
  Utils::SpecBuffer mFftData;  // FFT data normalized from 0.0-1.0
};