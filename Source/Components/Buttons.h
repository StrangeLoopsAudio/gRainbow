/*
 ==============================================================================

 Sliders.h
 Created: 23 Dec 2023 8:34:54pm
 Author:  brady

 ==============================================================================
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"

class MapButton : public juce::TextButton,
public Parameters::Listener,
public juce::Timer {
public:
  MapButton(Parameters& parameters, ModSource& modSource): mParameters(parameters), mModSource(modSource) {
    setColour(juce::TextButton::ColourIds::buttonColourId, mModSource.colour);
    setClickingTogglesState(true);
    setButtonText("map");
    setTooltip("Once enabled, drag sliders to create modulations");
    startTimer(300);
    onClick = [this]() {
      mParameters.setMappingModSource(getToggleState() ? &mModSource : nullptr);
    };
    mParameters.addListener(this);
  }
  ~MapButton() {
    mParameters.removeListener(this);
    stopTimer();
  }

  void timerCallback() override {
    if (getToggleState()) {
      mIsBright = !mIsBright;
      setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour.brighter(mIsBright ? 0.3f : 0.0f));
      repaint();
    }
  }

  void mappingSourceChanged(ModSource* mod) override {
    if (mod == &mModSource) {
      mIsBright = true;
      setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour);
    } else {
      setToggleState(false, juce::dontSendNotification);
      mIsBright = true;
      setColour(juce::TextButton::ColourIds::buttonOnColourId, mModSource.colour);
    }
  }

private:
  Parameters& mParameters;
  ModSource& mModSource;
  bool mIsBright = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MapButton)
};

// Enables a common parameter to be displayed at different levels
class CommonButton : public juce::TextButton, public Parameters::Listener {
public:
  CommonButton(Parameters& _parameters, ParamCommon::Type type) : parameters(_parameters), parameter(_parameters.global.common[type]), mType(type) {
    setToggleable(true);
    setColour(juce::TextButton::textColourOffId, Utils::Colour::GLOBAL);
    setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    setColour(juce::TextButton::buttonColourId, Utils::Colour::GLOBAL);
    setColour(juce::TextButton::buttonOnColourId, Utils::Colour::GLOBAL);

    onClick = [this]() {
      ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, !getToggleState());
      juce::Colour usedColour = getUsedColour();
      setColour(juce::TextButton::textColourOffId, usedColour);
      setColour(juce::TextButton::buttonColourId, usedColour);
      setColour(juce::TextButton::buttonOnColourId, usedColour);
      // Update active parameter pointer
      parameter = parameters.getUsedParam(parameters.getSelectedParams(), mType);
    };
    parameters.addListener(this);
  }
  ~CommonButton() { parameters.removeListener(this); }

  ParamCommon* getParam() { return parameters.getSelectedParams(); }
  bool getIsUsed() { return parameters.getSelectedParams()->isUsed[mType]; }
  void mouseDoubleClick(const juce::MouseEvent&) override {
    const bool defaultVal = COMMON_DEFAULTS[mType];
    const bool globalVal = P_BOOL(parameters.global.common[mType])->get();
    // Reset value to the level above it
    if (parameters.getSelectedParams()->type == ParamType::GLOBAL) {
      // If global, reset to the parameter's default
      ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, defaultVal);
    } else if (parameters.getSelectedParams()->type == ParamType::NOTE) {
      // If note is used, reset to global value.. if not, reset to common default
      if (parameters.getSelectedParams()->isUsed[mType]) {
        ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, globalVal);
      } else {
        ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, defaultVal);
        ParamHelper::setCommonParam(&parameters.global, mType, defaultVal);
      }
    } else if (parameters.getSelectedParams()->type == ParamType::GENERATOR) {
      auto* gen = dynamic_cast<ParamGenerator*>(parameters.getSelectedParams());
      auto* note = parameters.note.notes[gen->noteIdx].get();
      if (parameters.getSelectedParams()->isUsed[mType]) {
        // If gen is used, reset to level above that's used (note or global)
        if (note->isUsed[mType]) {
          // Note is used, reset to its value
          ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, P_BOOL(note->common[mType])->get());
        } else {
          // Neither gen nor note is used, reset to global
          ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, globalVal);
        }
      } else {
        // If gen is not used, reset to either global or common default
        if (note->isUsed[mType]) {
          // Note is used, reset to global
          ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, globalVal);
          ParamHelper::setCommonParam(note, mType, globalVal);
          note->isUsed[mType] = false;
        } else {
          // Nothing is used, reset to common default
          ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, defaultVal);
          ParamHelper::setCommonParam(note, mType, defaultVal);
          note->isUsed[mType] = false;
          ParamHelper::setCommonParam(&parameters.global, mType, defaultVal);
        }
      }
    }
    parameters.getSelectedParams()->isUsed[mType] = false;
    selectedCommonParamsChanged(parameters.getSelectedParams());
  }

  // Update button colours for new selected group
  void selectedCommonParamsChanged(ParamCommon* newParams) override {
    juce::Colour usedColour = getUsedColour();
    setColour(juce::TextButton::textColourOffId, usedColour);
    setColour(juce::TextButton::buttonColourId, usedColour);
    setColour(juce::TextButton::buttonOnColourId, usedColour);    // Update active parameter pointer
    parameter = parameters.getUsedParam(newParams, mType);
  }
  juce::RangedAudioParameter* getParameter() { return parameter; }

private:
  // Get the colour of the parameter at the level that's used (global, note)
  juce::Colour getUsedColour() {
    // Is generator used?
    if (parameters.getSelectedParams()->isUsed[mType]) return parameters.getSelectedParamColour();
    else if (auto* gen = dynamic_cast<ParamGenerator*>(parameters.getSelectedParams())) {
      if (parameters.note.notes[gen->noteIdx]->isUsed[mType]) return parameters.getSelectedParamColour();
    }
    return Utils::Colour::GLOBAL;
  }

  Parameters& parameters;
  juce::RangedAudioParameter* parameter;
  ParamCommon::Type mType;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CommonButton)
};
