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

PitchDetector::PitchDetector() : mFft(FFT_SIZE, HOP_SIZE) {}

void PitchDetector::processBuffer(juce::AudioBuffer<float>& fileBuffer,
                               double sampleRate) {
  mFft.processBuffer(fileBuffer);
  updateHps(sampleRate);
}

/* Performs HPS on the FFT data for pitch tracking purposes */
void PitchDetector::updateHps(double sampleRate) {
  float totalMax = std::numeric_limits<float>::min();
  std::vector<Pitch> tempPitches;
  std::vector<std::vector<float>> spec = mFft.getSpectrum();

  int minIndex = juce::roundToInt(
      (juce::MidiMessage::getMidiNoteInHertz(MIN_MIDINOTE) * FFT_SIZE) /
      sampleRate);
  int maxIndex = juce::roundToInt(
      (juce::MidiMessage::getMidiNoteInHertz(MAX_MIDINOTE) * FFT_SIZE) /
      sampleRate);
  minIndex = juce::jlimit(0, FFT_SIZE, minIndex);
  maxIndex = juce::jlimit(0, FFT_SIZE, maxIndex);
  int maxHIndex = FFT_SIZE / NUM_HPS_HARMONICS;
  if (maxIndex < maxHIndex) maxHIndex = maxIndex;
  for (int frame = 0; frame < spec.size(); ++frame) {
    int peakIndex = minIndex;
    // Perform harmonic doubling
    for (int i = minIndex; i < maxHIndex; ++i) {
      for (int j = 1; j <= NUM_HPS_HARMONICS; ++j) {
        spec[frame][i] *= spec[frame][i * j];
      }
      if (spec[frame][i] > spec[frame][peakIndex]) {
        peakIndex = i;
      }
    }
    // Correct octave errors
    int peakIndex2 = minIndex;
    int maxsearch = peakIndex * 3 / 4;
    for (int i = minIndex + 1; i < maxsearch; i++) {
      if (spec[frame][i] > spec[frame][peakIndex2]) {
        peakIndex2 = i;
      }
    }
    if (std::abs(peakIndex2 * 2 - peakIndex) < 4) {
      if (spec[frame][peakIndex2] / spec[frame][peakIndex] > 0.2) {
        peakIndex = peakIndex2;
      }
    }

    float maxVal = spec[frame][peakIndex];
    if (maxVal > totalMax) {
      totalMax = maxVal;
    }
    float peakFreq = (peakIndex * sampleRate) / FFT_SIZE;
    auto newPitch = Pitch(peakFreq, (float)frame / spec.size(), maxVal);
    tempPitches.push_back(newPitch);  // Add temporary pitch
  }
  // Normalize post-hps data
  for (int i = 0; i < spec.size(); ++i) {
    for (int j = 0; j < spec[i].size(); ++j) {
      spec[i][j] /= totalMax;
    }
  }

  // Normalize temporary pitch amplitudes and add valid pitches
  mPitches.clear();
  for (int i = 0; i < tempPitches.size(); i++) {
    // Make sure within freq range
    if (tempPitches[i].freq >=
            juce::MidiMessage::getMidiNoteInHertz(MIN_MIDINOTE) &&
        tempPitches[i].freq <=
            juce::MidiMessage::getMidiNoteInHertz(MAX_MIDINOTE) &&
        tempPitches[i].gain > totalMax * DETECTION_THRESHOLD) {
      tempPitches[i].gain /= totalMax;
      // Check if already detected at same pos with greater gain
      int foundIndex = -1;
      for (int j = 0; j < mPitches.size(); ++j) {
        if (tempPitches[i].freq == mPitches[j].freq &&
            std::abs(tempPitches[i].posRatio - mPitches[j].posRatio) <
                DETECTION_SPREAD) {
          foundIndex = j;
        }
      }
      // If already added, make sure the higher gain is chosen
      if (foundIndex != -1) {
        if (mPitches[foundIndex].gain < tempPitches[i].gain) {
          mPitches[foundIndex] = tempPitches[i];
        }
      } else {
        mPitches.push_back(tempPitches[i]);
      }
    }
  }

  // REMOVE WHEN FINISHED TESTING
  for (int i = 0; i < mPitches.size(); ++i) {
    DBG("freq: " << mPitches[i].freq << ", gain: " << mPitches[i].gain
                 << ", pos: " << mPitches[i].posRatio);
  }
  DBG("found " << mPitches.size() << " pitches");
}