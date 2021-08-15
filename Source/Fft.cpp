/*
  ==============================================================================

    Fft.cpp
    Created: 26 Apr 2021 9:01:42pm
    Author:  brady

  ==============================================================================
*/

#include "Fft.h"

// Once a buffer is loaded, will run to produce mFftData and then will async
// notify when done
void Fft::run() {
  if (mFileBuffer == nullptr) return;
  // Runs with first channel
  const float* pBuffer = mFileBuffer->getReadPointer(0);
  mFftFrame.clear();
  mFftFrame.resize(mWindowSize * 2, 0.0f);
  int curSample = 0;
  bool hasData = mFileBuffer->getNumSamples() > mFftFrame.size();
  float curMax = std::numeric_limits<float>::min();
  mFftData.clear();

  while (hasData && !threadShouldExit()) {
    const float* startSample = &pBuffer[curSample];
    int numSamples = mFftFrame.size();
    if (curSample + mFftFrame.size() > mFileBuffer->getNumSamples()) {
      numSamples = (mFileBuffer->getNumSamples() - curSample);
    }
    mFftFrame.clear();
    mFftFrame.resize(mWindowSize * 2, 0.0f);
    memcpy(mFftFrame.data(), startSample, numSamples);
    mWindowEnvelope.multiplyWithWindowingTable(mFftFrame.data(),
                                               mFftFrame.size());

    // then render our FFT data..
    mForwardFFT.performFrequencyOnlyForwardTransform(mFftFrame.data());

    // Add fft data to our master array
    std::vector<float> newFrame = std::vector<float>(
        mFftFrame.begin(), mFftFrame.begin() + (mWindowSize / 2));
    float frameMax = juce::FloatVectorOperations::findMaximum(mFftFrame.data(),
                                                              mFftFrame.size());
    if (frameMax > curMax) curMax = frameMax;
    mFftData.push_back(newFrame);
    // Normalize fft values according to max frame value
    for (int i = 0; i < mFftData.back().size(); ++i) {
      mFftData.back()[i] /= curMax;
    }

    curSample += mHopSize;
    if (curSample > mFileBuffer->getNumSamples()) hasData = false;
  }

  if (onProcessingComplete != nullptr) {
    onProcessingComplete(mFftData);
  }
}

void Fft::processBuffer(juce::AudioBuffer<float>* fileBuffer) {
  stopThread(4000);
  mFileBuffer = fileBuffer;
  startThread();
}