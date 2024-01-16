/*
  ==============================================================================

    Utils.h
    Created: 10 Apr 2021 3:27:51pm
    Author:  brady

  ==============================================================================
*/

#pragma once

namespace Utils {

static constexpr int ENV_LUT_SIZE = 64;  // grain env lookup table size
typedef std::array<float, ENV_LUT_SIZE> GrainEnv;

/* 
 Makes a simple grain envelope LUT divided into 3 parts
 
    1.0   -----
 rampUp  /     \  rampDown
        /       \
 
 @arg shape: (0.0, 1.0), triangle env at 0, square env at 1.0
 @arg tilt: (-1.0, 1.0), left edge triangle at -1.0, right edge triangle at 1.0
 */
[[maybe_unused]] static void fillGrainEnvelopeLUT(GrainEnv& lut, const float shape, const float tilt) {
  float scaledShape = (shape * ENV_LUT_SIZE) / 2.0f;
  float scaledTilt = (ENV_LUT_SIZE * 0.5f) + (tilt * ENV_LUT_SIZE * 0.5f);
  int rampUpEndSample = juce::jmax(0.0f, scaledTilt - scaledShape);
  int rampDownStartSample = juce::jmin((float)ENV_LUT_SIZE, scaledTilt + scaledShape);
  for (int i = 0; i < ENV_LUT_SIZE; i++) {
    if (i < rampUpEndSample) {
      // Ramping up
      lut[i] = (float)i / rampUpEndSample;
    } else if (i > rampDownStartSample) {
      // Ramping down
      lut[i] = 1.0f - (float)(i - rampDownStartSample) / (ENV_LUT_SIZE - rampDownStartSample);
    } else {
      // At the peak
      lut[i] = 1.0f;
    }
  }
  //juce::FloatVectorOperations::clip(lut.data(), lut.data(), 0.0f, 1.0f, lut.size());
}

enum ADSRState { ATTACK, DECAY, SUSTAIN, RELEASE };

typedef struct EnvelopeADSR {
  // All adsr params are in samples (except for sustain amp)
  ADSRState state = ADSRState::ATTACK;
  float amplitude = 0.0f;
  float noteOffAmplitude = 0.0f;
  int noteOnTs = -1;
  int noteOffTs = -1;
  EnvelopeADSR() {}
  void noteOn(int ts) {
    noteOnTs = ts;
    noteOffTs = -1;
    state = ADSRState::ATTACK;
    amplitude = 0.0f;
    noteOffAmplitude = 0.0f;
  }
  void noteOff(int ts) {
    noteOffTs = ts;
    noteOffAmplitude = amplitude;
    state = ADSRState::RELEASE;
  }
  /* ADSR params (except sustain) should be in samples */
  float getAmplitude(int curTs, float attack, float decay, float sustain, float release) {
    const float refTs = (state == Utils::ADSRState::RELEASE) ? noteOffTs : noteOnTs;
    float tsDiff = static_cast<float>(curTs - refTs);
    switch (state) {
      case Utils::ADSRState::ATTACK: {
        if (noteOnTs < 0) return 0.0f;
        amplitude = tsDiff / attack;
        if (tsDiff >= attack) {
          state = Utils::ADSRState::DECAY;
        }
        break;
      }
      case Utils::ADSRState::DECAY: {
        if (noteOnTs < 0) return 0.0f;
        amplitude = 1.0f - ((tsDiff - attack) / (float)decay) * (1.0f - sustain);
        if (tsDiff - attack >= decay) {
          state = Utils::ADSRState::SUSTAIN;
        }
        break;
      }
      case Utils::ADSRState::SUSTAIN: {
        amplitude = sustain;
        // Note: setting note off sets state to release, we don't need to
        // here
        break;
      }
      case Utils::ADSRState::RELEASE: {
        if (noteOffTs < 0) return 0.0f;
        amplitude = noteOffAmplitude - ((tsDiff / release) * noteOffAmplitude);
        if (tsDiff > release) {
          noteOnTs = -1;
          noteOffTs = -1;
        }
        break;
      }
    }
    return amplitude;
  }
} EnvelopeADSR;

}  // namespace Utils
