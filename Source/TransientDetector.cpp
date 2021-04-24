/*
  ==============================================================================

    TransientDetector.cpp
    Created: 21 Apr 2021 9:34:38pm
    Author:  brady

  ==============================================================================
*/

#define _USE_MATH_DEFINES

#include "TransientDetector.h"
#include <limits.h>

TransientDetector::TransientDetector() : mForwardFFT(FFT_ORDER) {}

void TransientDetector::loadBuffer(juce::AudioBuffer<float>& fileBuffer) {
  updateFft(fileBuffer);
  retrieveTransients();
  DBG("all done here-----------------------");
}

void TransientDetector::updateFft(juce::AudioBuffer<float>& fileBuffer) {
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

void TransientDetector::retrieveTransients() {
  // Perform transient detection on each frame
  mTransients.clear();
  mEnergyBuffer.fill(0.0f);
  for (int frame = 0; frame < mFftData.size(); ++frame) {
    // Shift energy frames
    for (int i = PARAM_SPREAD - 1; i >= 0; --i) {
      if (i == 0) mEnergyBuffer[i] = 0;
      else {
        mEnergyBuffer[i] = mEnergyBuffer[i - 1];
      }
    }

    // Accumulate frame energy
    for (int i = 0; i < FFT_SIZE / 2; ++i) {
      mEnergyBuffer[0] += mFftData[frame][i];
    }

    // Check energy threshold
    if (isTransient()) {
      mTransients.push_back(Transient((float)frame / mFftData.size(), 1.0f));
      mAttackFrames = PARAM_ATTACK_LOCK;
    } else if (mAttackFrames != 0) {
      mAttackFrames--;
    }
  }
}

bool TransientDetector::isTransient() {
  if (mAttackFrames != 0)
    return false;  // Too close to a previous transient, skip
  for (int i = 0; i < PARAM_SPREAD; ++i) {
    if (i == PARAM_SPREAD - 1) {
      if (mEnergyBuffer[0] <= mEnergyBuffer[i] * PARAM_THRESHOLD) return false;
    } else if (mEnergyBuffer[i] <= mEnergyBuffer[i + 1]) {
      return false;
    }
  }
  return true;
}