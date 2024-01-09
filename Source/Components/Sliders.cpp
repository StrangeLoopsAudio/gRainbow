#include "Sliders.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/Colour.h"
#include "Utils/PitchClass.h"
#include "Parameters.h"

ParamSlider::ParamSlider(Parameters& _parameters, juce::RangedAudioParameter* _parameter) : parameters(_parameters), parameter(_parameter) {
  // Knob params
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;
  setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
  setRotaryParameters(rotaryParams);
  setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Utils::GLOBAL_COLOUR);
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::GLOBAL_COLOUR);
  if (!parameter) return;
  setSkewFactor(parameter->getNormalisableRange().skew);
  onValueChange = [this] {
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(parameter)) ParamHelper::setParam(floatParam, (float)getValue());
    else if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(parameter)) ParamHelper::setParam(intParam, (int)getValue());
    else jassert(false); // if you get here you should add an if-case for the new param type, it must not be a float or int
  };
}

CommonSlider::CommonSlider(Parameters& _parameters, ParamCommon::Type type)
    : ParamSlider(_parameters, _parameters.global.common[type]), mType(type) {
  // Knob params
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;
  setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
  setRotaryParameters(rotaryParams);
  setSkewFactor(parameters.global.common[mType]->getNormalisableRange().skew);
  setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Utils::GLOBAL_COLOUR);
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::GLOBAL_COLOUR);
  onValueChange = [this] {
    ParamHelper::setCommonParam(parameters.selectedParams, mType, (float)getValue());
    setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, getUsedColour());
    auto* param = parameters.selectedParams->common[mType];
    if (parameters.selectedParams->type == ParamType::GLOBAL) {
      mGlobalValue = param->getNormalisableRange().convertTo0to1(P_FLOAT(param)->get());
    } else if (parameters.selectedParams->type == ParamType::NOTE) {
      mNoteValue = param->getNormalisableRange().convertTo0to1(P_FLOAT(param)->get());
    }
  };
}

void CommonSlider::mouseDoubleClick(const juce::MouseEvent&) {
  const float defaultVal = COMMON_DEFAULTS[mType];
  const float globalVal = P_FLOAT(parameters.global.common[mType])->get();
  // Reset value to the level above it
  if (parameters.selectedParams->type == ParamType::GLOBAL) {
    // If global, reset to the parameter's default
    ParamHelper::setCommonParam(parameters.selectedParams, mType, defaultVal);
  } else if (parameters.selectedParams->type == ParamType::NOTE) {
    // If note is used, reset to global value.. if not, reset to common default
    if (parameters.selectedParams->isUsed[mType]) {
      ParamHelper::setCommonParam(parameters.selectedParams, mType, globalVal);
    } else {
      ParamHelper::setCommonParam(parameters.selectedParams, mType, defaultVal);
      ParamHelper::setCommonParam(&parameters.global, mType, defaultVal);
    }
  } else if (parameters.selectedParams->type == ParamType::GENERATOR) {
    auto* gen = dynamic_cast<ParamGenerator*>(parameters.selectedParams);
    auto* note = parameters.note.notes[gen->noteIdx].get();
    if (parameters.selectedParams->isUsed[mType]) {
      // If gen is used, reset to level above that's used (note or global)
      if (note->isUsed[mType]) {
        // Note is used, reset to its value
        ParamHelper::setCommonParam(parameters.selectedParams, mType, P_FLOAT(note->common[mType])->get());
      } else {
        // Neither gen nor note is used, reset to global
        ParamHelper::setCommonParam(parameters.selectedParams, mType, globalVal);
      }
    } else {
      // If gen is not used, reset to either global or common default
      if (note->isUsed[mType]) {
        // Note is used, reset to global
        ParamHelper::setCommonParam(parameters.selectedParams, mType, globalVal);
        ParamHelper::setCommonParam(note, mType, globalVal);
        note->isUsed[mType] = false;
      } else {
        // Nothing is used, reset to common default
        ParamHelper::setCommonParam(parameters.selectedParams, mType, defaultVal);
        ParamHelper::setCommonParam(note, mType, defaultVal);
        note->isUsed[mType] = false;
        ParamHelper::setCommonParam(&parameters.global, mType, defaultVal);
      }
    }
  }
  parameters.selectedParams->isUsed[mType] = false;
  updateSelectedParams();
}

// Update slider colours for new selected group
void CommonSlider::updateSelectedParams() {
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, getUsedColour());
  // Set global tick value
  auto* globalParam = parameters.global.common[mType];
  mGlobalValue = globalParam->getNormalisableRange().convertTo0to1(P_FLOAT(globalParam)->get());
  ParamNote* note = dynamic_cast<ParamNote*>(parameters.selectedParams);
  if (auto* gen = dynamic_cast<ParamGenerator*>(parameters.selectedParams)) {
    note = parameters.note.notes[gen->noteIdx].get();
  }
  if (note) {
    auto* noteParam = note->common[mType];
    mNoteValue = noteParam->getNormalisableRange().convertTo0to1(P_FLOAT(noteParam)->get());
  }
  // Update active parameter pointer
  parameter = parameters.getUsedParam(parameters.selectedParams, mType);
}

// Get the colour of the parameter at the level that's used (global, note)
juce::Colour CommonSlider::getUsedColour() {
  // Is generator used?
  if (parameters.selectedParams->isUsed[mType]) return parameters.getSelectedParamColour();
  else if (auto* gen = dynamic_cast<ParamGenerator*>(parameters.selectedParams)) {
    if (parameters.note.notes[gen->noteIdx]->isUsed[mType]) return parameters.getSelectedParamColour();
  }
  return Utils::GLOBAL_COLOUR;
}
