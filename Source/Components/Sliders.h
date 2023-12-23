
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
  float getGlobalValue() { return mGlobalValue; }
  float getNoteValue() { return mNoteValue; }
  ParamType getParamLevel() { return mParameters.selectedParams->type; }
  void mouseDoubleClick(const juce::MouseEvent& evt) override;

 private:
  // Get the colour of the parameter at the level that's used (global, note)
  juce::Colour getUsedColour();
  
  ParamCommon::Type mType;
  Parameters& mParameters;

  float mGlobalValue, mNoteValue;

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
