/*
  ==============================================================================

    Grain.cpp
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#include "Grain.h"

void Grain::process(
  juce::AudioBuffer<float>& fileBuffer,
  juce::AudioBuffer<float>& blockBuffer,
  RubberBand::RubberBandStretcher* timeStretcher,
  int time)
{ 
  for (int ch = 0; ch < blockBuffer.getNumChannels(); ++ch)
  {
    float gain = getGain(time);

    float* channelBlock = blockBuffer.getWritePointer(ch);

    int filePos = startPos + (time - trigTs);

    const float* fileBuf = fileBuffer.getReadPointer(0);
    float sample = fileBuf[filePos % fileBuffer.getNumSamples()];
    sample *= gain;
    channelBlock[time % blockBuffer.getNumSamples()] += sample;
  }
}

float Grain::getGain(int time)
{
  float perc = (time - trigTs) / (float)duration;
  perc = juce::jlimit(0.0f, 1.0f, perc);
  int i = perc * (mEnv.size() - 1);
  return mEnv[i];
}