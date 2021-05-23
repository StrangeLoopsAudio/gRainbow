/*
  ==============================================================================

    PitchDetector.cpp
    Created: 24 Apr 2021 2:07:10pm
    Author:  brady

  ==============================================================================
*/

#define _USE_MATH_DEFINES

#include "PitchDetector.h"

#include <limits.h>

PitchDetector::PitchDetector()
    : mFft(FFT_SIZE, HOP_SIZE), juce::Thread("pitch detector thread") {}

PitchDetector::~PitchDetector() { stopThread(2000); }

void PitchDetector::processBuffer(juce::AudioBuffer<float>* fileBuffer,
                                  double sampleRate) {
  stopThread(2000);
  mFileBuffer = fileBuffer;
  mSampleRate = sampleRate;
  startThread();
}

void PitchDetector::run() {
  if (mFileBuffer == nullptr) return;
  mFft.processBuffer(*mFileBuffer);
  if (threadShouldExit()) return;
  computeHPCP();
  if (onPitchesUpdated != nullptr && !threadShouldExit()) {
    onPitchesUpdated(mHPCP);
  }
}

void PitchDetector::computeHPCP() {
  float lastGain = 0.0f;
  bool isIncreasing = true;

  mPeaks.clear();
  mHPCP.clear();

  // Find local peaks to compute HPCP with
  float threshold = std::powf(10.0f, DB_THRESH / 20.0f);
  std::vector<std::vector<float>>& spec = mFft.getSpectrum();
  for (int frame = 0; frame < spec.size(); ++frame) {
    mPeaks.push_back(std::vector<Peak>());
    mHPCP.push_back(std::vector<float>(NUM_PITCH_CLASSES, 0.0f));
    lastGain = 0.0f;
    isIncreasing = true;

    for (int i = 0; i < spec[frame].size(); ++i) {
      float curGain = spec[frame][i];
      if (isIncreasing && curGain < lastGain) {
        Peak peak = interpolatePeak(frame, i);
        if (peak.gain > threshold && peak.freq > MIN_FREQ &&
            peak.freq < MAX_FREQ) {
          mPeaks[frame].push_back(peak);
        }
      }
      isIncreasing = curGain > lastGain;
      lastGain = curGain;
    }

    if (threadShouldExit()) return;

    // Create sum for each pitch class
    float curMax = 0.0;
    for (int i = 0; i < mPeaks[frame].size(); ++i) {
      for (int pc = 0; pc < NUM_PITCH_CLASSES; ++pc) {
      float centerFreq =
          REF_FREQ * std::pow(2.0f, (pc + 1) / (float)NUM_PITCH_CLASSES);
        float d = std::fmod(
            12.0f * std::log2(mPeaks[frame][i].freq / centerFreq), 12.0f);
        if (std::abs(d) <= (0.5f * HPCP_WINDOW_LEN)) {
          float w = std::pow(std::cos((M_PI * d) / HPCP_WINDOW_LEN), 2.0f);
          mHPCP[frame][pc] += (w * std::pow(mPeaks[frame][i].gain, 2));
          if (mHPCP[frame][pc] > curMax) curMax = mHPCP[frame][pc];
        }
      }
    }

    // Normalize HPCP frame
    if (curMax > 0.0f) {
      for (int pc = 0; pc < NUM_PITCH_CLASSES; ++pc) {
        mHPCP[frame][pc] /= curMax;
      }
    }
    
    if (threadShouldExit()) return;
  }
}

PitchDetector::Peak PitchDetector::interpolatePeak(int frame, int bin) {
  // Use quadratic interpolation to find peak freq and amplitude
  std::vector<std::vector<float>>& spec = mFft.getSpectrum();
  if (bin == 0 || bin == spec[frame].size() - 1) {
    return Peak((bin * mSampleRate) / FFT_SIZE, spec[frame][bin]);
  }
  float a = 20 * std::log10(spec[frame][bin - 1]);
  float b = 20 * std::log10(spec[frame][bin]);
  float c = 20 * std::log10(spec[frame][bin + 1]);

  float p = 0.5f * (a - c) / (a - (2.0f * b) + c);
  float interpBin = bin + p;
  float freq = (interpBin * mSampleRate) / FFT_SIZE;
  float gainDB = b - (0.25 * (a - c) * p);
  return Peak(freq, juce::jlimit(0.0, 1.0, std::pow(10, gainDB / 20.0f)));
}