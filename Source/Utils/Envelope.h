/*
  ==============================================================================

    Utils.h
    Created: 10 Apr 2021 3:27:51pm
    Author:  brady

  ==============================================================================
*/

#pragma once

namespace Utils {

enum EnvelopeState { ATTACK, DECAY, SUSTAIN, RELEASE };

typedef struct EnvelopeADSR {
  // All adsr params are in samples (except for sustain amp)
  EnvelopeState state = EnvelopeState::ATTACK;
  float amplitude = 0.0f;
  float noteOffAmplitude = 0.0f;
  int noteOnTs = -1;
  int noteOffTs = -1;
  EnvelopeADSR() {}
  void noteOn(int ts) {
    noteOnTs = ts;
    noteOffTs = -1;
    state = EnvelopeState::ATTACK;
    amplitude = 0.0f;
    noteOffAmplitude = 0.0f;
  }
  void noteOff(int ts) {
    noteOffTs = ts;
    noteOffAmplitude = amplitude;
    state = EnvelopeState::RELEASE;
  }
  /* ADSR params (except sustain) should be in samples */
  float getAmplitude(int curTs, float attack, float decay, float sustain, float release) {
    float newAmp = 0.0f;
    float tsDiff = static_cast<float>(curTs - noteOnTs);
    switch (state) {
      case Utils::EnvelopeState::ATTACK: {
        if (noteOnTs < 0) return 0.0f;
        newAmp = tsDiff / attack;
        if (tsDiff >= attack) {
          state = Utils::EnvelopeState::DECAY;
        }
        break;
      }
      case Utils::EnvelopeState::DECAY: {
        if (noteOnTs < 0) return 0.0f;
        newAmp = 1.0f - ((tsDiff - attack) / (float)decay) * (1.0f - sustain);
        if (tsDiff - attack >= decay) {
          state = Utils::EnvelopeState::SUSTAIN;
        }
        break;
      }
      case Utils::EnvelopeState::SUSTAIN: {
        newAmp = sustain;
        // Note: setting note off sets state to release, we don't need to
        // here
        break;
      }
      case Utils::EnvelopeState::RELEASE: {
        if (noteOffTs < 0) return 0.0f;
        newAmp = noteOffAmplitude - ((tsDiff / release) * noteOffAmplitude);
        if (tsDiff > release) {
          noteOnTs = -1;
          noteOffTs = -1;
        }
        break;
      }
    }
    amplitude = juce::jlimit(0.0f, 1.0f, newAmp);
    return amplitude;
  }
} EnvelopeADSR;

}  // namespace Utils
