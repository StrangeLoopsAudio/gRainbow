/*
  ==============================================================================

    Utils.h
    Created: 10 Apr 2021 3:27:51pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace Utils {
typedef std::vector<std::vector<float>> SpecBuffer;

// UI spacing and colours
static constexpr int EDITOR_WIDTH = 1000;
static constexpr int EDITOR_HEIGHT = 550;
static constexpr int PANEL_WIDTH = 270;
static constexpr int KEYBOARD_HEIGHT = 200;
static constexpr int PADDING = 6;
static constexpr float PADDING_F = 6.0f;
static constexpr int TITLE_HEIGHT = 17;
static constexpr int LABEL_HEIGHT = TITLE_HEIGHT;
static constexpr int KNOB_WIDTH = (PANEL_WIDTH - (PADDING * 2)) / 4;
static constexpr int KNOB_HEIGHT = KNOB_WIDTH / 2;
static constexpr float ROUNDED_AMOUNT = 6.0f;

// Grain envelopes and limits
static constexpr int MAX_GRAINS = 20;  // Max grains active at once
static constexpr auto ENV_LUT_SIZE = 64;  // grain env lookup table size
typedef std::array<float, ENV_LUT_SIZE> GrainEnv;

typedef struct Result {
  bool success;
  juce::String message;
} Result;

// Constant used for pitch shifting by semitones
static constexpr auto TIMESTRETCH_RATIO = 1.0594f;

static juce::Array<juce::String> SpecTypeNames{"Spectrogram", "Harmonic Profile", "Detected Pitches", "Audio Waveform"};

enum EnvelopeState { ATTACK, DECAY, SUSTAIN, RELEASE };
enum FilterType { NO_FILTER, LOWPASS, HIGHPASS, BANDPASS };

typedef struct EnvelopeADSR {
  // All adsr params are in samples (except for sustain amp)
  EnvelopeState state = EnvelopeState::ATTACK;
  float amplitude = 0.0f;
  float noteOffAmplitude = 0.0f;
  int noteOnTs = -1;
  int noteOffTs = -1;
  EnvelopeADSR() {}
  EnvelopeADSR(int ts) { noteOn(ts); }
  void noteOn(int ts) {
    noteOnTs = ts;
    noteOffTs = -1;
    state = EnvelopeState::ATTACK;
    amplitude = 0.0f;
    noteOffAmplitude = 0.0f;
  }
  void noteOff(int ts) {
    noteOnTs = -1;
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
          noteOffTs = -1;
        }
        break;
      }
    }
    amplitude = juce::jlimit(0.0f, 1.0f, newAmp);
    return amplitude;
  }
} EnvelopeADSR;

[[maybe_unused]] static void fillGrainEnvelopeLUT(GrainEnv& lut, const float shape, const float tilt) {
  /* LUT divided into 3 parts

               1.0
              -----
     rampUp  /     \  rampDown
            /       \
  */
  float scaledShape = (shape * ENV_LUT_SIZE) / 2.0f;
  float scaledTilt = tilt * ENV_LUT_SIZE;
  int rampUpEndSample = juce::jmax(0.0f, scaledTilt - scaledShape);
  int rampDownStartSample = juce::jmin((float)ENV_LUT_SIZE, scaledTilt + scaledShape);
  for (int i = 0; i < ENV_LUT_SIZE; i++) {
    if (i < rampUpEndSample) {
      lut[i] = static_cast<float>(i / rampUpEndSample);
    } else if (i > rampDownStartSample) {
      lut[i] = 1.0f - (float)(i - rampDownStartSample) / (ENV_LUT_SIZE - rampDownStartSample);
    } else {
      lut[i] = 1.0f;
    }
  }
  juce::FloatVectorOperations::clip(lut.data(), lut.data(), 0.0f, 1.0f, lut.size());
}

static inline float db2lin(float value) { return std::pow(10.0f, value / 10.0f); }
static inline float lin2db(float value) { return value < 1e-10 ? -100 : 10.0f * std::log10(value); }

}  // namespace Utils
