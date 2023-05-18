/*
  ==============================================================================

    Grain.cpp
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#include "Grain.h"

float Grain::process(float chanPerc, const juce::AudioBuffer<float>& audioBuffer, float gain, int time) {
  const float timePerc = (time - trigTs) / (float)duration;
  
  // Panning gain
  const float panGain = computeChannelPanningGain(chanPerc);

  const float totalGain = gain * panGain * getAmplitude(timePerc);
  const float* fileBuf = audioBuffer.getReadPointer(0);

  const float sampleIdx = duration * pbRate * timePerc;
  const int lowSample = std::floor(sampleIdx);
  const int highSample = std::ceil(sampleIdx);
  const float rem = sampleIdx - lowSample;

  // Some quick interpolation between sample values
  float sample = juce::jmap(rem, fileBuf[(startPos + lowSample) % audioBuffer.getNumSamples()],
                            fileBuf[(startPos + highSample) % audioBuffer.getNumSamples()]);


  sample *= totalGain;
  return sample;
}

float Grain::getAmplitude(float timePerc) {
  if (mEnv.size() == 0) return 0.0f;
  timePerc = juce::jlimit(0.0f, 1.0f, timePerc);
  int i = timePerc * (mEnv.size() - 1);
  return mEnv[i];
}

float Grain::computeChannelPanningGain(float chanPerc) {
  // Calculate angle based on panning value and channel index
  float angle = (pan + 1.0f) * juce::MathConstants<float>::pi / 4.0f + chanPerc * juce::MathConstants<float>::halfPi;

  // Compute gain for the specific channel
  return std::abs(std::cos(angle));
}