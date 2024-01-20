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
  setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Utils::Colour::GLOBAL);
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::Colour::GLOBAL);
  if (!parameter) return;
  setSkewFactor(parameter->getNormalisableRange().skew);
  onValueChange = [this] {
    if (parameters.getMappingModSource()) {
      int idx = parameter->getParameterIndex();
      if (!parameters.modulations.contains(idx)) {
        // Add modulator if it doesn't exist
        parameters.modulations.set(idx, Modulation(parameters.getMappingModSource(), 0.0f));
      } else {
        // Increment/decrement its depth
        double diff = parameter->convertTo0to1(getValue()) - parameter->convertTo0to1(dragStartValue);
        double scale = getRange().getLength() / (getRange().getEnd() - dragStartValue);
        float depth = juce::jlimit(-1.0, 1.0, diff * scale);
        parameters.modulations.set(idx, Modulation(parameters.getMappingModSource(), depth));
      }
      // Reset actual slider value
      setValue(dragStartValue, juce::dontSendNotification);
    } else {
      if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(parameter)) ParamHelper::setParam(floatParam, (float)getValue());
      else if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(parameter)) ParamHelper::setParam(intParam, (int)getValue());
      else jassert(false); // if you get here you should add an if-case for the new param type, it must not be a float or int
    }
  };
}

CommonSlider::CommonSlider(Parameters& _parameters, ParamCommon::Type type)
: ParamSlider(_parameters, _parameters.global.common[type]), mType(type) {
  parameters.addListener(this);

  // Knob params
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;
  setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
  setRotaryParameters(rotaryParams);
  setSkewFactor(parameters.global.common[mType]->getNormalisableRange().skew);
  setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Utils::Colour::GLOBAL);
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::Colour::GLOBAL);
  onValueChange = [this] {
    if (parameters.getMappingModSource()) {
      int idx = parameter->getParameterIndex();
      if (!parameters.modulations.contains(idx)) {
        // Add modulator if it doesn't exist
        parameters.modulations.set(idx, Modulation(parameters.getMappingModSource(), 0.0f));
      } else {
        // Increment/decrement its depth
        Modulation& mod = parameters.modulations.getReference(idx);
        double diff = parameter->convertTo0to1(getValue()) - parameter->convertTo0to1(dragStartValue);
        double scale = getRange().getLength() / (getRange().getEnd() - dragStartValue);
        mod.depth = juce::jlimit(-1.0, 1.0, diff * scale);
      }
      // Reset actual slider value
      setValue(dragStartValue, juce::dontSendNotification);
    } else {
      ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, getValue());
      setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, getUsedColour());
      // Update active parameter pointer
      parameter = parameters.getUsedParam(parameters.getSelectedParams(), mType);
    }
  };
}

CommonSlider::~CommonSlider() {
  parameters.removeListener(this);
}

void CommonSlider::mouseDoubleClick(const juce::MouseEvent&) {
  const float defaultVal = COMMON_DEFAULTS[mType];
  float globalVal;
  if (auto* pFloat = P_FLOAT(parameters.global.common[mType])) globalVal = pFloat->get();
  if (auto* pInt = P_INT(parameters.global.common[mType])) globalVal = pInt->get();
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
        if (auto* pFloat = P_FLOAT(parameters.global.common[mType])) {
          ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, pFloat->get());
        } else if (auto* pInt = P_INT(parameters.global.common[mType])) {
          ParamHelper::setCommonParam(parameters.getSelectedParams(), mType, pInt->get());
        }
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

// Update slider colours for new selected group
void CommonSlider::selectedCommonParamsChanged(ParamCommon* newParams) {
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, getUsedColour());
  // Update active parameter pointer
  parameter = parameters.getUsedParam(newParams, mType);
}

// Get the colour of the parameter at the level that's used (global, note)
juce::Colour CommonSlider::getUsedColour() {
  // Is generator used?
  if (parameters.getSelectedParams()->isUsed[mType]) return parameters.getSelectedParamColour();
  else if (auto* gen = dynamic_cast<ParamGenerator*>(parameters.getSelectedParams())) {
    if (parameters.note.notes[gen->noteIdx]->isUsed[mType]) return parameters.getSelectedParamColour();
  }
  return Utils::Colour::GLOBAL;
}
