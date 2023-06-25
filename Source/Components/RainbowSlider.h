
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/PitchClass.h"
#include "Parameters.h"
#include <optional>

// Enables more than one slider value to be displayed at once in a rainbow fashion
class RainbowSlider : public juce::Slider {
 public:
  RainbowSlider(Parameters& parameters, ParamCommon::Type type);

  // Update slider colours for new selected group
  void updateSelectedParams();

  std::optional<float> getArcValue(Utils::PitchClass pitchClass);

 private:
  ParamCommon::Type mType;
  Parameters& mParameters;

  juce::HashMap<Utils::PitchClass, float> mArcs;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowSlider)
};
