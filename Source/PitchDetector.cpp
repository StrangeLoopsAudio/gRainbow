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

PitchDetector::PitchDetector() : mForwardFFT(FFT_ORDER) {}

void PitchDetector::loadBuffer(juce::AudioBuffer<float>& fileBuffer,
                               double sampleRate) {
  updateFft(fileBuffer);
  updateHps(sampleRate);
}

void PitchDetector::updateFft(juce::AudioBuffer<float>& fileBuffer) {
  const float* pBuffer = fileBuffer.getReadPointer(0);
  int curSample = 0;

  bool hasData = fileBuffer.getNumSamples() > mFftFrame.size();
  float curMax = std::numeric_limits<float>::min();

  mFftData.clear();

  while (hasData) {
    const float* startSample = &pBuffer[curSample];
    int numSamples = mFftFrame.size();
    if (curSample + mFftFrame.size() > fileBuffer.getNumSamples()) {
      numSamples = (fileBuffer.getNumSamples() - curSample);
    }
    mFftFrame.fill(0.0f);
    memcpy(mFftFrame.data(), startSample, numSamples);

    // then render our FFT data..
    mForwardFFT.performFrequencyOnlyForwardTransform(mFftFrame.data());

    // Add fft data to our master array
    std::vector<float> newFrame =
        std::vector<float>(mFftFrame.begin(), mFftFrame.end());
    float frameMax = juce::FloatVectorOperations::findMaximum(mFftFrame.data(),
                                                              mFftFrame.size());
    if (frameMax > curMax) curMax = frameMax;
    mFftData.push_back(newFrame);

    curSample += mFftFrame.size();
    if (curSample > fileBuffer.getNumSamples()) hasData = false;
  }

  /* Normalize fft values according to max value */
  for (int i = 0; i < mFftData.size(); ++i) {
    for (int j = 0; j < mFftData[i].size(); ++j) {
      mFftData[i][j] /= curMax;
    }
  }
}

/* Performs HPS on the FFT data for pitch tracking purposes */
void PitchDetector::updateHps(double sampleRate) {
  float totalMax = std::numeric_limits<float>::min();
  std::vector<Pitch> tempPitches;

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
  for (int frame = 0; frame < mFftData.size(); ++frame) {
    int peakIndex = minIndex;
    // Perform harmonic doubling
    for (int i = minIndex; i < maxHIndex; ++i) {
      for (int j = 1; j <= NUM_HPS_HARMONICS; ++j) {
        mFftData[frame][i] *= mFftData[frame][i * j];
      }
      if (mFftData[frame][i] > mFftData[frame][peakIndex]) {
        peakIndex = i;
      }
    }
    // Correct octave errors
    int peakIndex2 = minIndex;
    int maxsearch = peakIndex * 3 / 4;
    for (int i = minIndex + 1; i < maxsearch; i++) {
      if (mFftData[frame][i] > mFftData[frame][peakIndex2]) {
        peakIndex2 = i;
      }
    }
    if (std::abs(peakIndex2 * 2 - peakIndex) < 4) {
      if (mFftData[frame][peakIndex2] / mFftData[frame][peakIndex] > 0.2) {
        peakIndex = peakIndex2;
      }
    }

    float maxVal = mFftData[frame][peakIndex];
    if (maxVal > totalMax) {
      totalMax = maxVal;
    }
    float peakFreq = (peakIndex * sampleRate) / FFT_SIZE;
    auto newPitch = Pitch(peakFreq, (float)frame / mFftData.size(), maxVal);
    tempPitches.push_back(newPitch);  // Add temporary pitch
  }
  // Normalize post-hps data
  for (int i = 0; i < mFftData.size(); ++i) {
    for (int j = 0; j < mFftData[i].size(); ++j) {
      mFftData[i][j] /= totalMax;
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
          DBG("replacing " << mPitches[foundIndex].freq << ", "
                           << mPitches[foundIndex].gain << "--- with --- "
                           << tempPitches[i].freq << ", "
                           << tempPitches[i].gain);
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