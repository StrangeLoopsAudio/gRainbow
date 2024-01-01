
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
  ParamCommon* getParam() { return mParameters.selectedParams; }
  bool getIsUsed() { return mParameters.selectedParams->isUsed[mType]; }
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

class QuantizedSlider : public CommonSlider {
public:
  QuantizedSlider(Parameters& parameters, ParamCommon::Type type) : CommonSlider(parameters, type), mSync(false), mRange(COMMON_RANGES[type]) {}
  void setSync(bool sync) {
    mSync = sync;
    setTextValueSuffix(sync ? "" : suffix);
  }
  
  void setSuffix(juce::String _suffix) {
    suffix = _suffix;
    setTextValueSuffix(mSync ? "" : suffix);
  }
  
  juce::String getTextFromValue(double) override {
    if (mSync) {
      float prog = mRange.convertTo0to1(getValue());
      return juce::String("1/") + juce::String(std::pow(2, (int)(ParamRanges::SYNC_DIV_MAX * prog)));
    } else {
      return juce::String(getValue());
    }
  }
  
private:
  bool mSync;
  juce::String suffix;
  juce::NormalisableRange<float> mRange;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizedSlider)
};
