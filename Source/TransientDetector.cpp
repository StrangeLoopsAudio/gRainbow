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
  for (int i = 0; i < PARAM_M; ++i) {
    iterateFunctions();
  }
  retrieveTransients();
}

void TransientDetector::updateFft(juce::AudioBuffer<float>& fileBuffer) {
  const float* pBuffer = fileBuffer.getReadPointer(0);
  int curSample = 0;

  bool hasData = fileBuffer.getNumSamples() > mFftFrame.size();
  float curMax = std::numeric_limits<float>::min();

  mX.clear();
  mF.clear();
  mLambda.clear();
  mSigmaGamma.clear();
  mP.clear();

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
    mX.push_back(newFrame);
    mF.push_back(std::vector<float>(mFftFrame.size(), 0.0f));
    mLambda.push_back(std::vector<float>(mFftFrame.size(), 0.0f));
    mP.push_back(std::vector<float>(mFftFrame.size(), 0.0f));

    curSample += mFftFrame.size();
    if (curSample > fileBuffer.getNumSamples()) hasData = false;
  }

  mSigmaGamma.insert(mSigmaGamma.begin(), mX.size(), 0.0f);

  /* Normalize fft values according to max value */
  for (int i = 0; i < mX.size(); ++i) {
    for (int j = 0; j < mX[i].size(); ++j) {
      mX[i][j] /= curMax;
    }
  }
}

void TransientDetector::iterateFunctions() {
  // Update F
  for (int i = 0; i < mF.size(); ++i) {
    for (int j = 0; j < FFT_SIZE; ++j) {
      mF[i][j] = F(i, j);
    }
  }
  // Update Lambda
  for (int i = 0; i < mF.size(); ++i) {
    for (int j = 0; j < FFT_SIZE; ++j) {
      mLambda[i][j] = LAMBDA(i, j);
    }
  }
  // Update Sigma Gamma
  for (int i = 0; i < mSigmaGamma.size(); ++i) {
    float sum = 0.0f;
    for (int j = 0; j < FFT_SIZE; ++j) {
      sum += GAMMA(i, j);
    }
    mSigmaGamma[i] = sum;
  }
  // Update P
  for (int i = 0; i < mP.size(); ++i) {
    for (int j = 0; j < FFT_SIZE; ++j) {
      mP[i][j] = P(i, j);
    }
  }
  // Update X
  for (int i = 0; i < mX.size(); ++i) {
    for (int j = 0; j < FFT_SIZE; ++j) {
      mX[i][j] = XMod(i, j);
    }
  }
}

void TransientDetector::retrieveTransients() {
  // TODO:
}

float TransientDetector::F(int frame, int bin) {
  float sum = 0.0f;
  for (int i = bin - PARAM_V; i <= bin + PARAM_V; ++i) {
    float tOnset = mX[i][bin] - mX[i - 1][bin];
    float tOffset = mX[i][bin] - mX[i + 1][bin];
    sum += (std::signbit(tOnset) ? tOnset : 0) +
            (std::signbit(tOffset) ? tOffset : 0);
  }
  return sum;
}

float TransientDetector::LAMBDA(int frame, int bin) {
  float fSum = 0.0f;
  for (int i = frame - PARAM_TAU; i <= frame + PARAM_TAU; ++i) {
    fSum += mF[i][bin];
  }
  return fSum / (2 * PARAM_TAU + 1);
}

int TransientDetector::GAMMA(int frame, int bin) { 
  return (mF[frame][bin] > mLambda[frame][bin]) ? 1 : 0;
}

float TransientDetector::P(int frame, int bin) {
  return (mSigmaGamma[frame] >= PARAM_THRESH) ? (PARAM_DELTA * mX[frame][bin])
                                             : mP[frame][bin];
}

float TransientDetector::XMod(int frame, int bin) {
  return (mSigmaGamma[frame] >= PARAM_THRESH) ? ((1.0f - PARAM_DELTA) * mX[frame][bin])
                                             : mX[frame][bin];
}