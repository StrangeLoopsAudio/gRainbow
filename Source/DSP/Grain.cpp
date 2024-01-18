/*
  ==============================================================================

    Grain.cpp
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#include "Grain.h"

float Grain::process(float chanPerc, const juce::AudioBuffer<float>& audioBuffer, float envelopeGain, int time) {
  // Time percentage over duration
  mTimePerc = juce::jlimit(0.0f, 1.0f, static_cast<float>((time - trigTs)) / duration);

  // Panning gain
  mPanGain = computeChannelPanningGain(chanPerc);

  mTotalGain = envelopeGain * mPanGain * getAmplitude();

  mSampleIdx = duration * pbRate * mTimePerc;
  mLowSample = std::floor(mSampleIdx);
  mHighSample = std::ceil(mSampleIdx);
  mSampleRem = mSampleIdx - mLowSample;
  if (pbRate < 0.0f) {
    std::swap(mLowSample, mHighSample);
    mSampleRem = fabsf(mLowSample - mSampleIdx);
    mLowSample += audioBuffer.getNumSamples();
    mHighSample += audioBuffer.getNumSamples();
  }

  // Some quick interpolation between sample values
  const float* fileBuf = audioBuffer.getReadPointer(0);
  mSampleValue = juce::jmap(mSampleRem, fileBuf[(startPos + mLowSample) % audioBuffer.getNumSamples()],
                            fileBuf[(startPos + mHighSample) % audioBuffer.getNumSamples()]);

  mSampleValue *= mTotalGain;
  return mSampleValue;
}

float Grain::getAmplitude() {
  if (mEnv.size() == 0) return 0.0f;
  return mEnv[mTimePerc * (mEnv.size() - 1)];
}

float Grain::computeChannelPanningGain(float chanPerc) {
  // Calculate angle based on panning value and channel index
  mAngle = (pan + 1.0f) * juce::MathConstants<float>::pi / 4.0f + chanPerc * juce::MathConstants<float>::halfPi;

  // Compute gain for the specific channel
  return std::abs(std::cos(mAngle));
}
