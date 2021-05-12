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

PitchDetector::PitchDetector() : mFft(FFT_SIZE, HOP_SIZE), juce::Thread("pitch detector thread") {}

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
  findPeaks();
  if (threadShouldExit()) return;
  computeHPCP();
}

void PitchDetector::findPeaks() {
  float lastGain = 0.0f;
  bool isIncreasing = true;

  mPeaks.clear();

  float threshold = std::powf(10.0f, DB_THRESH / 20.0f);
  std::vector<std::vector<float>> spec = mFft.getSpectrum();
  for (int frame = 0; frame < spec.size(); ++frame) {
    mPeaks.push_back(std::vector<Peak>());
    lastGain = 0.0f;
    isIncreasing = true;
    for (int i = 0; i < spec[frame].size(); ++i) {
      float curGain = spec[frame][i];
      if (isIncreasing && 
        curGain < lastGain) {
        Peak peak = interpolatePeak(frame, i);
        if (peak.gain > threshold && peak.freq > MIN_FREQ && peak.freq < MAX_FREQ) {
          mPeaks.back().push_back(peak);
        }
      }
      isIncreasing = curGain > lastGain;
      lastGain = curGain;
    }
    if (threadShouldExit()) return;
  }
}

void PitchDetector::computeHPCP() {
  mHPCP.clear(); // clear HPCP data
  for (int frame = 0; frame < mPeaks.size(); ++frame) {
    mHPCP.push_back(std::vector<float>(NUM_PITCH_CLASSES, 0.0f));
    // Create sum for each pitch class
    float curMax = std::numeric_limits<float>::min();
    for (int pc = 1; pc <= NUM_PITCH_CLASSES; ++pc) {
      float centerFreq = REF_FREQ * 2.0f * (pc / mHPCP.size());
      for (int i = 0; i < mPeaks[frame].size(); ++i) {
        float d = std::fmod(12.0f * std::log2(mPeaks[frame][i].freq / centerFreq), 12.0f);
        if (d <= (0.5f * HPCP_WINDOW_LEN)) {
          float w = std::pow(std::cos((M_PI * d) / HPCP_WINDOW_LEN), 2.0f);
          mHPCP.back()[pc - 1] += (w * std::pow(mPeaks[frame][i].gain, 2));
          if (mHPCP.back()[pc - 1] > curMax) curMax = mHPCP.back()[pc - 1];
        }
      }
    }

    // Normalize frame
    for (int pc = 0; pc < NUM_PITCH_CLASSES; ++pc) {
      mHPCP[frame][pc] /= curMax;
      if (mHPCP[frame][pc] == 1.0f) {
         // TODO: remove
        DBG("perc: " << (float)frame / mPeaks.size() << ", pc: " << pc);
      }
    }
    
    if (threadShouldExit()) return;
  }
}

PitchDetector::Peak PitchDetector::interpolatePeak(int frame, int bin) {
  // Use quadratic interpolation to find peak freq and amplitude
  std::vector<std::vector<float>> spec = mFft.getSpectrum();
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
  return Peak(freq, std::pow(10, gainDB / 20.0f));
}