/*
  ==============================================================================

    Grain.h
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "Utils/Utils.h"

class Grain {
 public:
  Grain() : duration(0), pbRate(1.0), startPos(0), trigTs(0), gain(0.0), pan(0.0) {}
  Grain(std::vector<float> env, int duration_, float pbRate_, int startPos_, int trigTs_, float gain_, float pan_)
      : duration(duration_),
        pbRate(pbRate_),
        startPos(juce::jmax(0, startPos_)),
        trigTs(trigTs_),
        gain(gain_),
        pan(pan_),
        mEnv(env) {}

  float process(float chanPerc, const juce::AudioBuffer<float>& audioBuffer, float gain, int time);

  const int duration;  // Grain duration in samples
  const float pbRate;  // Playback rate (1.0 being regular speed)
  const int startPos;  // Start position in file to play from in samples
  const int trigTs;   // Timestamp when grain was triggered in samples
  const float gain;  // Grain gain
  const float pan;

 private:
  float getAmplitude(float timePerc);
  float computeChannelPanningGain(float chanPerc);
  std::vector<float> mEnv;
};