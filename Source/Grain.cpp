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
  int time)
{
  float** channelBlocks = blockBuffer.getArrayOfWritePointers();
 
  for (int ch = 0; ch < blockBuffer.getNumChannels(); ++ch)
  {
    float gain = getGain(time);

    float* channelBlock = channelBlocks[ch];

    int filePos = startPos + (time - trigTs);

    const float* fileBuf = fileBuffer.getReadPointer(ch);
    float sample = fileBuf[filePos % fileBuffer.getNumSamples()];
    sample *= gain;
    channelBlock[time % blockBuffer.getNumSamples()] += sample;
  }
}

float Grain::getGain(int time)
{
  return 1.0f;
}