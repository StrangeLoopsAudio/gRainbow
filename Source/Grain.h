/*
  ==============================================================================

    Grain.h
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Utils.h"

class Grain {
 public:
  Grain()
      : generator((Utils::GeneratorColour)0),
        duration(0),
        pbRate(1.0),
        startPos(0),
        trigTs(0),
        gain(0.0) {}
  Grain(Utils::GeneratorColour generator, std::vector<float> env, int duration,
        float pbRate, int startPos, int trigTs, float gain)
      : generator(generator),
        mEnv(env),
        duration(duration),
        pbRate(pbRate),
        startPos(startPos),
        trigTs(trigTs),
        gain(gain) {}
  ~Grain() {}

  void process(juce::AudioBuffer<float>& fileBuffer,
               juce::AudioBuffer<float>& blockBuffer, float gain, int time);

  float getAmplitude(float timePerc);

  const int duration;  // Grain duration in samples
  const int startPos;  // Start position in file to play from in samples
  const int trigTs;    // Timestamp when grain was triggered in samples
  const float pbRate;  // Playback rate (1.0 being regular speed)
  const float gain;    // Grain gain
  const Utils::GeneratorColour generator;

 private:
  std::vector<float> mEnv;
};