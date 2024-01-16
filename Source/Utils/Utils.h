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

typedef struct Result {
  bool success;
  juce::String message;
} Result;

// Constant used for pitch shifting by semitones
static constexpr auto TIMESTRETCH_RATIO = 1.0594f;

enum FilterType { NO_FILTER, LOWPASS, HIGHPASS, BANDPASS };

static inline float db2lin(float value) { return std::pow(10.0f, value / 10.0f); }
static inline float lin2db(float value) { return value < 1e-10 ? -100 : 10.0f * std::log10(value); }

}  // namespace Utils
