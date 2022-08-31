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

TransientDetector::TransientDetector() : mFft(FFT_SIZE, HOP_SIZE), juce::Thread("transient thread") {
  mFft.onProcessingComplete = [this](std::vector<std::vector<float>>& spectrum) {
    stopThread(4000);
    startThread();
  };
}

TransientDetector::~TransientDetector() { stopThread(2000); }

void TransientDetector::processAudioBuffer(juce::AudioBuffer<float>* audioBuffer) {
  mFft.processAudioBuffer(audioBuffer);
  mInputBuffer = audioBuffer;
}

void TransientDetector::run() {
  if (mInputBuffer == nullptr) return;
  retrieveTransients();
  if (onTransientsUpdated != nullptr && !threadShouldExit()) {
    onTransientsUpdated(mTransients);
  }
}

void TransientDetector::retrieveTransients() {
  // Perform transient detection on each frame
  std::vector<std::vector<float>>& spec = mFft.getSpectrum();
  mTransients.clear();
  mEnergyBuffer.fill(0.0f);
  for (int frame = 0; frame < spec.size(); ++frame) {
    if (threadShouldExit()) return;
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
      mTransients.push_back(Transient((float)frame / spec.size(), 1.0f));
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