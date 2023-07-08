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

TransientDetector::TransientDetector(double startProgress, double endProgress)
    : juce::Thread("transient thread"),
      mStartProgress(startProgress / 2.0),
      mEndProgress(endProgress),
      mDiffProgress(mEndProgress - mStartProgress),
      mFft(FFT_SIZE, HOP_SIZE, startProgress, endProgress / 2.0) {
  mFft.onProcessingComplete = [this](Utils::SpecBuffer&) {
    stopThread(4000);
    startThread();
  };
}

TransientDetector::~TransientDetector() { stopThread(2000); }

void TransientDetector::process(const juce::AudioBuffer<float>* audioBuffer) { mFft.process(audioBuffer); }

void TransientDetector::run() {
  retrieveTransients();
  if (onTransientsUpdated != nullptr && !threadShouldExit()) {
    onTransientsUpdated(mTransients);
  }
}

void TransientDetector::updateProgress(double progress) {
  if (onProgressUpdated != nullptr) {
    onProgressUpdated(progress);
  }
}

void TransientDetector::retrieveTransients() {
  // Perform transient detection on each frame
  const Utils::SpecBuffer& spec = mFft.getSpectrum();
  mTransients.clear();
  mEnergyBuffer.fill(0.0f);
  for (size_t frame = 0; frame < spec.size(); ++frame) {
    if (threadShouldExit()) return;
    updateProgress(mStartProgress + (mDiffProgress * static_cast<double>(frame / spec.size())));
    // Shift energy frames
    for (int i = PARAM_SPREAD - 1; i >= 0; --i) {
      if (i == 0)
        mEnergyBuffer[i] = 0;
      else {
        mEnergyBuffer[i] = mEnergyBuffer[i - 1];
      }
    }

    // Accumulate frame energy
    for (int i = 0; i < FFT_SIZE / 2; ++i) {
      mEnergyBuffer[0] += spec[frame][i];
    }

    // Check energy threshold
    if (isTransient()) {
      mTransients.emplace_back(Transient((float)frame / spec.size(), 1.0f));
      mAttackFrames = PARAM_ATTACK_LOCK;
    } else if (mAttackFrames != 0) {
      mAttackFrames--;
    }
  }
}

bool TransientDetector::isTransient() {
  if (mAttackFrames != 0) return false;  // Too close to a previous transient, skip
  for (int i = 0; i < PARAM_SPREAD; ++i) {
    if (i == PARAM_SPREAD - 1) {
      if (mEnergyBuffer[0] <= mEnergyBuffer[i] * PARAM_THRESHOLD) return false;
    } else if (mEnergyBuffer[i] <= mEnergyBuffer[i + 1]) {
      return false;
    }
  }
  return true;
}