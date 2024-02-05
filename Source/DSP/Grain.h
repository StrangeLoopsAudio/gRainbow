/*
  ==============================================================================

    Grain.h
    Created: 11 Apr 2021 8:51:46pm
    Author:  brady

  ==============================================================================
*/

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "Utils/Envelope.h"

class Grain {
 public:
  Grain() : duration(0), pbRate(1.0), startPos(0), trigTs(0), gain(0.0), pan(0.0), isActive(false) {}
  
  void set(int _duration, float _pbRate, int _startPos, int _trigTs, float _gain, float _pan, float shape, float tilt) {
    isActive = true;
    duration = _duration;
    pbRate = _pbRate;
    startPos = juce::jmax(0, _startPos);
    trigTs = _trigTs;
    gain = _gain;
    pan = _pan;
    Utils::fillGrainEnvelopeLUT(mEnv, shape, tilt);
  }
  

  float process(float chanPerc, const juce::AudioBuffer<float>& audioBuffer, float gain, int time);

  int duration;  // Grain duration in samples
  float pbRate;  // Playback rate (1.0 being regular speed, -1.0 being regular speed in reverse)
  int startPos;  // Start position in file to play from in samples
  int trigTs;   // Timestamp when grain was triggered in samples
  float gain;  // Grain gain
  float pan;
  bool isActive;

 private:
  float getAmplitude(float timePerc);
  float computeChannelPanningGain(float chanPerc);
  Utils::GrainEnv mEnv;
};
