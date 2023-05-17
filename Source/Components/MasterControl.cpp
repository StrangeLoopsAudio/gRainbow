/*
  ==============================================================================

    MasterControl.cpp
    Created: 16 May 2023 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "MasterControl.h"
#include "../Utils.h"

MasterControl::MasterControl(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mSliderGain(parameters, ParamCommon::Type::GAIN)
{

  juce::Colour colour = Utils::GLOBAL_COLOUR;

  // Adjust pitch
  mSliderGain.setNumDecimalPlacesToDisplay(2);
  mSliderGain.setRange(ParamRanges::GAIN.start, ParamRanges::GAIN.end, 0.01);
  addAndMakeVisible(mSliderGain);

  mLabelGain.setText("Gain", juce::dontSendNotification);
  mLabelGain.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelGain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelGain);

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

void MasterControl::parameterValueChanged(int idx, float value) { mParamHasChanged.store(true); }

void MasterControl::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderGain.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::GAIN),
                                juce::dontSendNotification);
  }
}

void MasterControl::updateSelectedParams() { 
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);

  mParamColour = mParameters.getSelectedParamColour();
  mSliderGain.updateSelectedParams();
  
  mParamHasChanged.store(true);
  repaint();
}

void MasterControl::paint(juce::Graphics& g) {
  juce::Colour colour = mParamColour;

  // Section title
  g.setColour(Utils::GLOBAL_COLOUR);
  g.fillRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT);
  g.setColour(colour);
  g.drawRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(juce::String(SECTION_TITLE), mTitleRect, juce::Justification::centred);

  g.setColour(mParamColour);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), Utils::ROUNDED_AMOUNT, 2.0f);
}

void MasterControl::resized() {
  juce::Rectangle<int> r = getLocalBounds();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING).withCentre(r.getCentre());

  // Make title rect
  mTitleRect = r.removeFromTop(Utils::TITLE_HEIGHT).toFloat();

  r.removeFromTop(Utils::PADDING);

  mLabelGain.setBounds(r.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderGain.setBounds(r.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
}
