/*
  ==============================================================================

    Grain.cpp
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#include "Grain.h"

float Grain::process(juce::AudioBuffer<float>& fileBuffer, juce::AudioBuffer<float>& blockBuffer, float gain, int time) {
  float timePerc = (time - trigTs) / (float)duration;
  float totalGain = gain * getAmplitude(timePerc);
  const float** fileBuf = fileBuffer.getArrayOfReadPointers();

  float unStretchedDuration = duration * pbRate;
  int lowSample = std::floor(juce::jmax(0.0f, timePerc * unStretchedDuration));
  int highSample = std::ceil(juce::jmax(0.0f, timePerc * unStretchedDuration));
  float rem = (timePerc * unStretchedDuration) - lowSample;

  jassert(blockBuffer.getNumChannels() > 0);
  float sample = juce::jmap(rem, fileBuf[0][(startPos + lowSample) % fileBuffer.getNumSamples()],
                            fileBuf[0][(startPos + highSample) % fileBuffer.getNumSamples()]);
  sample *= totalGain;
  return sample;
}

float Grain::getAmplitude(float timePerc) {
  if (mEnv.size() == 0) return 0.0f;
  timePerc = juce::jlimit(0.0f, 1.0f, timePerc);
  int i = timePerc * (mEnv.size() - 1);
  return mEnv[i];
}