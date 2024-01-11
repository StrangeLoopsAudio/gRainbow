/*
  ==============================================================================

    HPCP.h
    Created: 11 Jan 2024 9:01:42pm
    Author:  brady

    FFT processing and utilities with overlapping windows and normalized values

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "Utils/DSP.h"

class HPCP {
 public:
  HPCP() { initHarmonicWeights(); }

  void clear();

  Utils::SpecBuffer& process(Utils::SpecBuffer& fft, double sampleRate);
  const Utils::SpecBuffer& getHPCP() { return mHPCP; }

 private:
  // Spectral Whitening
  static constexpr double MIN_AVG_FRAME_ENERGY = 0.0001;
  // HPCP
  static constexpr int REF_FREQ = 440;
  static constexpr int MAX_SPEC_PEAKS = 60;
  static constexpr int NUM_HPCP_BINS = 120;
  static constexpr float HPCP_WINDOW_LEN = 1.0f;
  static constexpr int NUM_HARMONIC_WEIGHTS = 3;
  static constexpr int MIN_FREQ = 40;
  static constexpr int MAX_FREQ = 5000;
  static constexpr double HARMONIC_PRECISION = 0.00001;
  static constexpr double MAGNITUDE_THRESHOLD = 0.00001;
  static constexpr int PITCH_CLASS_OFFSET = 9;  // Offset from reference freq A to lowest class C
  static constexpr int PITCH_CLASS_OFFSET_BINS = (NUM_HPCP_BINS / 12) * PITCH_CLASS_OFFSET;
  
  typedef struct Peak {
    float binNum;  // Bin number in frame
    float gain;
    Peak() : binNum(-1), gain(0) {}
    Peak(float binNum_, float gain_) : binNum(binNum_), gain(gain_) {}
  } Peak;
  
  typedef struct HarmonicWeight {
    float semitone;
    float gain;
    HarmonicWeight(float semitone_, float gain_) : semitone(semitone_), gain(gain_) {}
  } HarmonicWeight;
  
  void initHarmonicWeights();
  std::vector<Peak> getPeaks(int numPeaks, const std::vector<float>& frame);
  Peak interpolatePeak(Utils::SpecBuffer& spec, int frame, int bin);
  void interpolatePeak(const float leftVal, const float middleVal, const float rightVal, int currentBin, float& resultVal,
                       float& resultBin) const;
  
  // pointer to buffer to read from
  const juce::AudioBuffer<float>* mInputBuffer = nullptr;

  // HPCP fields
  double mSampleRate;
  std::vector<HarmonicWeight> mHarmonicWeights;
  Utils::SpecBuffer mHPCP;  // harmonic pitch class profile
};
