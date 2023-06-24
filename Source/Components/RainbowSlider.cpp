#include "RainbowSlider.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/Colour.h"
#include "Utils/PitchClass.h"
#include "Parameters.h"

RainbowSlider::RainbowSlider(Parameters& parameters, ParamCommon::Type type)
    : juce::Slider(), mType(type), mParameters(parameters) {
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
  setDoubleClickReturnValue(true, COMMON_DEFAULTS[type]);
  onValueChange = [this] {
    ParamHelper::setCommonParam(mParameters.selectedParams, mType, (float)getValue());
    if (mParameters.selectedParams->type == ParamType::NOTE) {
      Utils::PitchClass pitchClass = (Utils::PitchClass) dynamic_cast<ParamNote*>(mParameters.selectedParams)->noteIdx;
      float posNorm = juce::jmap(getValue(), getMinimum(), getMaximum(), 0.0, 1.0);
      mArcs.set(pitchClass, posNorm);
    }
  };
}

// Update slider colours for new selected group
void RainbowSlider::updateSelectedParams() {
  setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, mParameters.getSelectedParamColour());
}

std::optional<float> RainbowSlider::getArcValue(Utils::PitchClass pitchClass) {
  return mArcs.contains(pitchClass) ? std::optional<float>{mArcs[pitchClass]} : std::nullopt;
}