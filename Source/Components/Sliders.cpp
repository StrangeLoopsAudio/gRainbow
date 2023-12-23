#include "Sliders.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/Colour.h"
#include "Utils/PitchClass.h"
#include "Parameters.h"

CommonSlider::CommonSlider(Parameters& parameters, ParamCommon::Type type)
    : juce::Slider(), mType(type), mParameters(parameters) {
  // Knob params
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;
  setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
  setRotaryParameters(rotaryParams);
  setSkewFactor(mParameters.global.common[mType]->getNormalisableRange().skew);
  setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Utils::GLOBAL_COLOUR);
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::GLOBAL_COLOUR);
  //setDoubleClickReturnValue(true, COMMON_DEFAULTS[mType]);
  onValueChange = [this] {
    ParamHelper::setCommonParam(mParameters.selectedParams, mType, (float)getValue());
    setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, getUsedColour());
    auto* param = mParameters.selectedParams->common[mType];
    if (mParameters.selectedParams->type == ParamType::GLOBAL) {
      mGlobalValue = param->getNormalisableRange().convertTo0to1(P_FLOAT(param)->get());
    } else if (mParameters.selectedParams->type == ParamType::NOTE) {
      mNoteValue = param->getNormalisableRange().convertTo0to1(P_FLOAT(param)->get());
    }
  };
}

void CommonSlider::mouseDoubleClick(const juce::MouseEvent& evt) {
  const float defaultVal = COMMON_DEFAULTS[mType];
  const float globalVal = P_FLOAT(mParameters.global.common[mType])->get();
  // Reset value to the level above it
  if (mParameters.selectedParams->type == ParamType::GLOBAL) {
    // If global, reset to the parameter's default
    ParamHelper::setCommonParam(mParameters.selectedParams, mType, defaultVal);
  } else if (mParameters.selectedParams->type == ParamType::NOTE) {
    // If note is used, reset to global value.. if not, reset to common default
    if (mParameters.selectedParams->isUsed[mType]) {
      ParamHelper::setCommonParam(mParameters.selectedParams, mType, globalVal);
    } else {
      ParamHelper::setCommonParam(mParameters.selectedParams, mType, defaultVal);
      ParamHelper::setCommonParam(&mParameters.global, mType, defaultVal);
    }
  } else if (mParameters.selectedParams->type == ParamType::GENERATOR) {
    auto* gen = dynamic_cast<ParamGenerator*>(mParameters.selectedParams);
    auto* note = mParameters.note.notes[gen->noteIdx].get();
    if (mParameters.selectedParams->isUsed[mType]) {
      // If gen is used, reset to level above that's used (note or global)
      if (note->isUsed[mType]) {
        // Note is used, reset to its value
        ParamHelper::setCommonParam(mParameters.selectedParams, mType, P_FLOAT(note->common[mType])->get());
      } else {
        // Neither gen nor note is used, reset to global
        ParamHelper::setCommonParam(mParameters.selectedParams, mType, globalVal);
      }
    } else {
      // If gen is not used, reset to either global or common default
      if (note->isUsed[mType]) {
        // Note is used, reset to global
        ParamHelper::setCommonParam(mParameters.selectedParams, mType, globalVal);
        ParamHelper::setCommonParam(note, mType, globalVal);
        note->isUsed[mType] = false;
      } else {
        // Nothing is used, reset to common default
        ParamHelper::setCommonParam(mParameters.selectedParams, mType, defaultVal);
        ParamHelper::setCommonParam(note, mType, defaultVal);
        note->isUsed[mType] = false;
        ParamHelper::setCommonParam(&mParameters.global, mType, defaultVal);
      }
    }
  }
  mParameters.selectedParams->isUsed[mType] = false;
  updateSelectedParams();
}


// Update slider colours for new selected group
void CommonSlider::updateSelectedParams() {
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, getUsedColour());
  // Set global tick value
  auto* globalParam = mParameters.global.common[mType];
  mGlobalValue = globalParam->getNormalisableRange().convertTo0to1(P_FLOAT(globalParam)->get());
  ParamNote* note = dynamic_cast<ParamNote*>(mParameters.selectedParams);
  if (auto* gen = dynamic_cast<ParamGenerator*>(mParameters.selectedParams)) {
    note = mParameters.note.notes[gen->noteIdx].get();
  }
  if (note) {
    auto* noteParam = note->common[mType];
    mNoteValue = noteParam->getNormalisableRange().convertTo0to1(P_FLOAT(noteParam)->get());
  }
}

// Get the colour of the parameter at the level that's used (global, note)
juce::Colour CommonSlider::getUsedColour() {
  // Is generator used?
  if (mParameters.selectedParams->isUsed[mType]) return mParameters.getSelectedParamColour();
  else if (auto* gen = dynamic_cast<ParamGenerator*>(mParameters.selectedParams)) {
    if (mParameters.note.notes[gen->noteIdx]->isUsed[mType]) return mParameters.getSelectedParamColour();
  }
  return Utils::GLOBAL_COLOUR;
}


GlobalSlider::GlobalSlider(juce::RangedAudioParameter* param)
: juce::Slider(), mParam(param) {
  if (!param) return;
  // Knob params
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;
  setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
  setRotaryParameters(rotaryParams);
  setSkewFactor(mParam->getNormalisableRange().skew);
  setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Utils::GLOBAL_COLOUR);
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::GLOBAL_COLOUR);
  setDoubleClickReturnValue(true, mParam->getDefaultValue());
  // TODO: set global param on value change
  onValueChange = [this] {
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(mParam)) ParamHelper::setParam(floatParam, (float)getValue());
    else if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(mParam)) ParamHelper::setParam(intParam, (int)getValue());
    else jassert(false); // if you get here you should add an if-case for the new param type, it must not be a float or int
  };
}
