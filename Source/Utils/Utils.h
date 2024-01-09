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

// UI refresh timer interval (for updating sliders/displays when host changes their value)
static constexpr int UI_REFRESH_INTERVAL = 20; // Interval in ms

// UI spacing and colours
static constexpr int PADDING = 3;
static constexpr int EDITOR_WIDTH = 762;
static constexpr int EDITOR_HEIGHT = 400;
static constexpr int PRESET_PANEL_HEIGHT = 47;
static constexpr int PANEL_WIDTH = EDITOR_WIDTH / 3;
static constexpr int BUTTON_WIDTH = PANEL_WIDTH / 6;
static constexpr int PANEL_HEIGHT = (EDITOR_HEIGHT - PRESET_PANEL_HEIGHT - (PADDING * 3)) / 2;
static constexpr int TAB_HEIGHT = 30;
static constexpr int KEYBOARD_HEIGHT = 200;
static constexpr float PADDING_F = 6.0f;
static constexpr int TITLE_HEIGHT = 17;
static constexpr int LABEL_HEIGHT = TITLE_HEIGHT;
static constexpr int KNOB_WIDTH = 25;
static constexpr int KNOB_HEIGHT = 25;
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

enum FilterType { NO_FILTER, LOWPASS, HIGHPASS, BANDPASS };

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
