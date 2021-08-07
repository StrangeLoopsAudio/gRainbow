/*
  ==============================================================================

    Grain.cpp
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#include "Grain.h"

void Grain::process(juce::AudioBuffer<float>& fileBuffer,
                    juce::AudioBuffer<float>& blockBuffer, int time) {
  float timePerc = (time - trigTs) / (float)duration;
  float totalGain = gain * getAmplitude(timePerc);
  const float* fileBuf = fileBuffer.getReadPointer(0);

  float unStretchedDuration = duration * pbRate;
  int lowSample = std::floor(juce::jmax(0.0f, timePerc * unStretchedDuration));
  int highSample = std::ceil(juce::jmax(0.0f, timePerc * unStretchedDuration));
  float rem = (timePerc * unStretchedDuration) - lowSample;
  float sample = juce::jmap(
      rem, fileBuf[(startPos + lowSample) % fileBuffer.getNumSamples()],
      fileBuf[(startPos + highSample) % fileBuffer.getNumSamples()]);
  sample *= totalGain;
  for (int ch = 0; ch < blockBuffer.getNumChannels(); ++ch) {
    float* channelBlock = blockBuffer.getWritePointer(ch);
    channelBlock[time % blockBuffer.getNumSamples()] += sample;
  }
}

float Grain::getAmplitude(float timePerc) {
  if (mEnv.size() == 0) return 0.0f;
  timePerc = juce::jlimit(0.0f, 1.0f, timePerc);
  int i = timePerc * (mEnv.size() - 1);
  return mEnv[i];
}