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
    : mFft(FFT_SIZE, HOP_SIZE), juce::Thread("pitch detector thread") {
  initHarmonicWeights();
}

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
  estimatePitches();
  if (onPitchesUpdated != nullptr && !threadShouldExit()) {
    //onPitchesUpdated(mHPCP);
    onPitchesUpdated(mPitchesTest);
  }
}

void PitchDetector::estimatePitches() {
  mPitchesTest.clear();
  for (int frame = 0; frame < mHPCP.size(); ++frame) {
    mPitchesTest.push_back(std::vector<float>(NUM_PITCH_CLASSES, 0.0f));
    for (int i = 0; i < NUM_PITCH_CLASSES; ++i) {
      if (mHPCP[frame][i] == 1.0f) {
        mPitchesTest[frame][i] = 1.0f;
      }
    }
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
        // Add contribution from each harmonic
        for (int w = 0; w < NUM_HARMONIC_WEIGHTS; ++w) {
          float freq = mPeaks[frame][i].freq *
                       pow(2., -mHarmonicWeights[w].semitone / 12.0);
          float harmonicWeight = mHarmonicWeights[w].gain;
          float d = std::fmod(12.0f * std::log2(freq / centerFreq), 12.0f);
          if (std::abs(d) <= (0.5f * HPCP_WINDOW_LEN)) {
            float w = std::pow(std::cos((M_PI * d) / HPCP_WINDOW_LEN), 2.0f);
            mHPCP[frame][pc] += (w * std::pow(mPeaks[frame][i].gain, 2) *
                                 harmonicWeight * harmonicWeight);
            if (mHPCP[frame][pc] > curMax) curMax = mHPCP[frame][pc];
          }
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

// From essentia:
// Builds a weighting table of harmonic contribution. Higher harmonics
// contribute less and the fundamental frequency has a full harmonic
// strength of 1.0.
void PitchDetector::initHarmonicWeights() {
  mHarmonicWeights.clear();

  // Populate _harmonicPeaks with the semitonal positions of each of the
  // harmonics.
  for (int i = 0; i <= NUM_HARMONIC_WEIGHTS; i++) {
    float semitone = 12.0 * log2(i + 1.0);
    float octweight = std::max(1.0, (semitone / 12.0) * 0.5);

    // Get the semitone within the range (0-HARMONIC_PRECISION, 12.0-HARMONIC_PRECISION]
    while (semitone >= 12.0 - HARMONIC_PRECISION) {
      semitone -= 12.0;
    }

    // Check to see if the semitone has already been added to weights
    std::vector<HarmonicWeight>::iterator it;
    for (it = mHarmonicWeights.begin(); it != mHarmonicWeights.end(); it++) {
      if ((*it).semitone > semitone - HARMONIC_PRECISION &&
          (*it).semitone < semitone + HARMONIC_PRECISION)
        break;
    }

    if (it == mHarmonicWeights.end()) {
      // no harmonic peak found for this frequency; add it
      mHarmonicWeights.push_back(HarmonicWeight(semitone, (1.0 / octweight)));
    } else {
      // else, add the weight
      (*it).gain += (1.0 / octweight);
    }
  }
}