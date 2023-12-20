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
  setDoubleClickReturnValue(true, COMMON_DEFAULTS[mType]);
  onValueChange = [this] {
    ParamHelper::setCommonParam(mParameters.selectedParams, mType, (float)getValue());
    if (mParameters.selectedParams->type == ParamType::NOTE) {
      Utils::PitchClass pitchClass = (Utils::PitchClass) dynamic_cast<ParamNote*>(mParameters.selectedParams)->noteIdx;
      float posNorm = mParameters.selectedParams->common[mType]->getNormalisableRange().convertTo0to1((float)getValue());
      mArcs.set(pitchClass, posNorm);
    }
  };
}

// Update slider colours for new selected group
void CommonSlider::updateSelectedParams() {
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, mParameters.getSelectedParamColour());
}

std::optional<float> CommonSlider::getArcValue(Utils::PitchClass pitchClass) {
  return mArcs.contains(pitchClass) ? std::optional<float>{mArcs[pitchClass]} : std::nullopt;
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
