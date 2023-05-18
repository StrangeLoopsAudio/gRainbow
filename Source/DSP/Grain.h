/*
  ==============================================================================

    Grain.h
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Utils.h"

class Grain {
 public:
  Grain() : duration(0), pbRate(1.0), startPos(0), trigTs(0), gain(0.0), pan(0.0) {}
  Grain(std::vector<float> env, int duration, float pbRate, int startPos, long trigTs, float gain, float pan)
      : mEnv(env), duration(duration), pbRate(pbRate), startPos(juce::jmax(0, startPos)), trigTs(trigTs), gain(gain), pan(pan) {
  
  }
  ~Grain() {}

  float process(float chanPerc, const juce::AudioBuffer<float>& audioBuffer, float gain, int time);

  const int duration;  // Grain duration in samples
  const int startPos;  // Start position in file to play from in samples
  const long trigTs;   // Timestamp when grain was triggered in samples
  const float pbRate;  // Playback rate (1.0 being regular speed)
  const float pan;
  const float gain;  // Grain gain

 private:
  float getAmplitude(float timePerc);
  float computeChannelPanningGain(float chanPerc);
  std::vector<float> mEnv;
};