/*
  ==============================================================================

    Grain.cpp
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#include "Grain.h"

float Grain::process(float chanPerc, const juce::AudioBuffer<float>& audioBuffer, float envelopeGain, int time) {
  const float timePerc = static_cast<float>((time - trigTs)) / duration;

  // Panning gain
  const float panGain = computeChannelPanningGain(chanPerc);

  const float totalGain = envelopeGain * panGain * getAmplitude(timePerc);
  const float* fileBuf = audioBuffer.getReadPointer(0);

  const float sampleIdx = duration * pbRate * timePerc;
  int lowSample = std::floor(sampleIdx);
  int highSample = std::ceil(sampleIdx);
  float rem = sampleIdx - lowSample;
  if (pbRate < 0.0f) {
    std::swap(lowSample, highSample);
    rem = fabsf(lowSample - sampleIdx);
    lowSample += audioBuffer.getNumSamples();
    highSample += audioBuffer.getNumSamples();
  }

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
