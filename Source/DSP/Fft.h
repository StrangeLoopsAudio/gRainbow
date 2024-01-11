/*
  ==============================================================================

    Fft.h
    Created: 26 Apr 2021 9:01:42pm
    Author:  brady

    FFT processing and utilities with overlapping windows and normalized values

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "Utils/DSP.h"

class Fft {
 public:
  Fft(int windowSize, int hopSize);
  ~Fft();

  void clear(bool clearData);

  Utils::SpecBuffer* process(const juce::AudioBuffer<float>* audioBuffer);
  Utils::SpecBuffer* getSpectrum() { return &mFftData; }

 private:
  // pointer to buffer to read from
  const juce::AudioBuffer<float>* mInputBuffer = nullptr;

  // values passed in at creation time
  int mWindowSize;
  int mHopSize;
  juce::dsp::FFT mForwardFFT;
  juce::dsp::WindowingFunction<float> mWindowEnvelope;

  // processed data
  std::vector<float> mFftFrame;
  Utils::SpecBuffer mFftData;  // FFT data normalized from 0.0-1.0
};
