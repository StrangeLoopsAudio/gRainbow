#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ff_meters/ff_meters.h"

class MeterLookAndFeel : public foleys::LevelMeterLookAndFeel {
 public:
  MeterLookAndFeel() {}

 private:
  // Disables meter tick marks
  juce::Rectangle<float> getMeterTickmarksBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Disables meter clip light
  juce::Rectangle<float> getMeterClipIndicatorBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Disable meter max number
  juce::Rectangle<float> getMeterMaxNumberBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Just use the regular bounds, no margins
  juce::Rectangle<float> getMeterBarBounds(juce::Rectangle<float> bounds, foleys::LevelMeter::MeterFlags) const override {
    return bounds;
  }
};