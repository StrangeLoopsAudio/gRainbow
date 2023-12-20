
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/PitchClass.h"
#include "Parameters.h"
#include <optional>

// Enables a common parameter to be displayed at different levels
class CommonSlider : public juce::Slider {
 public:
  CommonSlider(Parameters& parameters, ParamCommon::Type type);

  // Update slider colours for new selected group
  void updateSelectedParams();

  std::optional<float> getArcValue(Utils::PitchClass pitchClass);

 private:
  ParamCommon::Type mType;
  Parameters& mParameters;

  juce::HashMap<Utils::PitchClass, float> mArcs;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CommonSlider)
};

// Enables a common parameter to be displayed at different levels
class GlobalSlider : public juce::Slider {
public:
  GlobalSlider(juce::RangedAudioParameter* param);

private:
  juce::RangedAudioParameter* mParam;
    
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalSlider)
};
