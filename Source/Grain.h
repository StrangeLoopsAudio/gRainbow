/*
  ==============================================================================

    Grain.h
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Grain
{
public:
  Grain() : 
    mEnv(std::array<float, 512>()),
    duration(0),
    pbRate(1.0),
    startPos(0),
    trigTs(0) {}
  Grain(
    std::array<float, 512> &env,
    int duration,
    float pbRate,
    int startPos,
    int trigTs
  ) :
    mEnv(env),
    duration(duration), 
    pbRate(pbRate),
    startPos(startPos),
    trigTs(trigTs) {}
  ~Grain() {}

  void process(
    juce::AudioBuffer<float>& fileBuffer,
    juce::AudioBuffer<float>& blockBuffer,
    int time);

  float getGain(float timePerc);

  const int duration; // Grain duration in samples
  const int startPos; // Start position in file to play from in samples
  const int trigTs;   // Timestamp when grain was triggered in samples
  const float pbRate;   // Playback rate (1.0 being regular speed)

private:
  std::array<float, 512> &mEnv;
};