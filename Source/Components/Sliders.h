/*
 ==============================================================================
 
 Sliders.h
 Created: 23 Dec 2023 8:34:54pm
 Author:  brady
 
 ==============================================================================
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/PitchClass.h"
#include "Parameters.h"

class ParamSlider : public juce::Slider {
public:
  ParamSlider(Parameters& parameters, juce::RangedAudioParameter* parameter);
  
  void mouseDown(const juce::MouseEvent &evt) override {
    if (evt.mods.isPopupMenu()) {
      // If modulations exist on this slider, let the user choose to remove them
      int idx = parameter->getParameterIndex();
      if (parameters.modulations.contains(idx)) {
        juce::PopupMenu menu;
        menu.addItem("Remove modulation", [this, idx]() {
          parameters.modulations.remove(idx);
        });
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
      }
    } else {
      juce::Slider::mouseDown(evt);
    }
  }
  void startedDragging() override {
    // Set drag start so that we can reset the value if mapping
    dragStartValue = getValue();
  }
  
  juce::RangedAudioParameter* getParameter() { return parameter; }
  
  Parameters& parameters;
  
protected:
  juce::RangedAudioParameter* parameter;
  float dragStartValue;
private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamSlider)
};

// Enables a common parameter to be displayed at different levels
class CommonSlider : public ParamSlider, public Parameters::Listener {
 public:
  CommonSlider(Parameters& parameters, ParamCommon::Type type);
  ~CommonSlider();

  ParamCommon* getParam() { return parameters.getSelectedParams(); }
  bool getIsUsed() { return parameters.getSelectedParams()->isUsed[mType]; }
  void mouseDoubleClick(const juce::MouseEvent& evt) override;
  
  void selectedCommonParamsChanged(ParamCommon* newParams) override;

 private:
  // Get the colour of the parameter at the level that's used (global, note)
  juce::Colour getUsedColour();
  
  ParamCommon::Type mType;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CommonSlider)
};

class QuantizedCommonSlider : public CommonSlider {
public:
  QuantizedCommonSlider(Parameters& _parameters, ParamCommon::Type type, bool reverse) : CommonSlider(_parameters, type), mSync(false), mRange(COMMON_RANGES[type]), mReverse(reverse) {}
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
      if (mReverse) prog = 1.0f - prog;
      return juce::String("1/") + juce::String(std::pow(2, juce::roundToInt(ParamRanges::SYNC_DIV_MAX * prog)));
    } else {
      return juce::String(getValue()) + " " + getTextValueSuffix();
    }
  }
  
private:
  bool mSync;
  bool mReverse;
  juce::String suffix;
  juce::NormalisableRange<float> mRange;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizedCommonSlider)
};

class QuantizedGlobalSlider : public ParamSlider {
public:
  QuantizedGlobalSlider(Parameters& parameters, juce::RangedAudioParameter* param, juce::NormalisableRange<float> range, bool reverse) : ParamSlider(parameters, param), mSync(false), mRange(range), mReverse(reverse) {}
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
      if (mReverse) prog = 1.0f - prog;
      return juce::String("1/") + juce::String(std::pow(2, juce::roundToInt(ParamRanges::SYNC_DIV_MAX * prog)));
    } else {
      return juce::String(getValue());
    }
  }
  
private:
  bool mSync;
  bool mReverse;
  juce::String suffix;
  juce::NormalisableRange<float> mRange;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizedGlobalSlider)
};
