/*
  ==============================================================================

    Fft.cpp
    Created: 26 Apr 2021 9:01:42pm
    Author:  brady

  ==============================================================================
*/

#include "Fft.h"

void Fft::processBuffer(juce::AudioBuffer<float>& fileBuffer) {
  mFileLength = fileBuffer.getNumSamples();

  const float* pBuffer = fileBuffer.getReadPointer(0);
  mFftFrame.clear();
  mFftFrame.resize(mWindowSize * 2, 0.0f);
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
    mFftFrame.clear();
    mFftFrame.resize(mWindowSize * 2, 0.0f);
    memcpy(mFftFrame.data(), startSample, numSamples);
    mWindowEnvelope.multiplyWithWindowingTable(mFftFrame.data(),
                                               mFftFrame.size());

    // then render our FFT data..
    mForwardFFT.performFrequencyOnlyForwardTransform(mFftFrame.data());

    // Add fft data to our master array
    std::vector<float> newFrame =
        std::vector<float>(mFftFrame.begin(), mFftFrame.begin() + (mWindowSize / 2));
    float frameMax = juce::FloatVectorOperations::findMaximum(mFftFrame.data(),
                                                              mFftFrame.size());
    if (frameMax > curMax) curMax = frameMax;
    mFftData.push_back(newFrame);
    /* Normalize fft values according to max frame value */
    for (int i = 0; i < mFftData.back().size(); ++i) {
      mFftData.back()[i] /= curMax;
    }

    curSample += mHopSize;
    if (curSample > fileBuffer.getNumSamples()) hasData = false;
  }

  /* Normalize fft values according to max value */
 /* for (int i = 0; i < mFftData.size(); ++i) {
    for (int j = 0; j < mFftData[i].size(); ++j) {
      mFftData[i][j] /= curMax;
    }
  } */
}