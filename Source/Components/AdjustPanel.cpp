/*
  ==============================================================================

    AdjustPanel.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "AdjustPanel.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

AdjustPanel::AdjustPanel(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mParamColour(Utils::GLOBAL_COLOUR),
      mSliderPitchAdjust(parameters, ParamCommon::Type::PITCH_ADJUST),
      mSliderPitchSpray(parameters, ParamCommon::Type::PITCH_SPRAY),
      mSliderPosAdjust(parameters, ParamCommon::Type::POS_ADJUST),
      mSliderPosSpray(parameters, ParamCommon::Type::POS_SPRAY),
      mSliderPanAdjust(parameters, ParamCommon::Type::PAN_ADJUST),
mSliderPanSpray(parameters, ParamCommon::Type::PAN_SPRAY) {
  
  mCurSelectedParams->addListener(this);
  updateSelectedParams();
  
  // Default slider settings
  std::vector<std::reference_wrapper<CommonSlider>> sliders = { mSliderPanSpray, mSliderPanAdjust, mSliderPosSpray, mSliderPosAdjust, mSliderPitchSpray, mSliderPitchAdjust };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(slider.get());
  }
  
  // Default button settings
  std::vector<std::reference_wrapper<juce::ToggleButton>> buttons = { mBtnReverse, mBtnTriggerMode };
  for (auto& btn : buttons) {
    btn.get().setColour(juce::ToggleButton::ColourIds::tickColourId, Utils::GLOBAL_COLOUR);
    addAndMakeVisible(btn.get());
  }
  
  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelReverse, mLabelPanSpray, mLabelPanAdjust, mLabelPosSpray, mLabelPosAdjust, mLabelPitchSpray, mLabelPitchAdjust, mLabelTriggerMode };
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
    label.get().setJustificationType(juce::Justification::centredTop);
    label.get().setFont(juce::Font(14));
    addAndMakeVisible(label.get());
  }
  
  // Adjust pitch
  mSliderPitchAdjust.setRange(ParamRanges::PITCH_ADJUST.start, ParamRanges::PITCH_ADJUST.end, 0.01);
  mLabelPitchAdjust.setText("tune adjust", juce::dontSendNotification);
  
  // Pitch spray
  mSliderPitchSpray.setTextValueSuffix(" cents");
  mSliderPitchSpray.setNumDecimalPlacesToDisplay(3);
  mSliderPitchSpray.setRange(ParamRanges::PITCH_SPRAY.start, ParamRanges::PITCH_SPRAY.end, 0.005);
  mLabelPitchSpray.setText("tune spray", juce::dontSendNotification);
  
  // Adjust position
  mSliderPosAdjust.setRange(ParamRanges::POSITION_ADJUST.start, ParamRanges::POSITION_ADJUST.end, 0.01);
  mLabelPosAdjust.setText("pos adjust", juce::dontSendNotification);
  
  // Position spray
  mSliderPosSpray.setTextValueSuffix("s");
  mSliderPosSpray.setNumDecimalPlacesToDisplay(3);
  mSliderPosSpray.setRange(ParamRanges::POSITION_SPRAY.start, ParamRanges::POSITION_SPRAY.end, 0.005);
  mLabelPosSpray.setText("pos spray", juce::dontSendNotification);
  
  // Adjust pan
  mSliderPanAdjust.setRange(ParamRanges::PAN_ADJUST.start, ParamRanges::PAN_ADJUST.end, 0.01);
  mLabelPanAdjust.setText("pan adjust", juce::dontSendNotification);
  
  // Pan spray
  mSliderPanSpray.setTextValueSuffix("s");
  mSliderPanSpray.setNumDecimalPlacesToDisplay(3);
  mSliderPanSpray.setRange(ParamRanges::PAN_SPRAY.start, ParamRanges::PAN_SPRAY.end, 0.005);
  mLabelPanSpray.setText("pan spray", juce::dontSendNotification);
        
  // Reverse playback
  mBtnReverse.onClick = [this]() {
    ParamHelper::setParam(P_BOOL(mCurSelectedParams->common[ParamCommon::Type::REVERSE]), mBtnReverse.getToggleState());
  };
  mLabelReverse.setText("reverse", juce::dontSendNotification);
  
  // Trigger mode
  mBtnTriggerMode.onClick = [this]() {
    // TODO: set the param
  };
  mLabelTriggerMode.setText("trig seq", juce::dontSendNotification);

  startTimer(100);
}

AdjustPanel::~AdjustPanel() {
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void AdjustPanel::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void AdjustPanel::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderPitchAdjust.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::PITCH_ADJUST),
                                juce::dontSendNotification);
    mSliderPitchSpray.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::PITCH_SPRAY),
                               juce::dontSendNotification);
    mSliderPosAdjust.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::POS_ADJUST),
                              juce::dontSendNotification);
    mSliderPosSpray.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::POS_SPRAY),
                             juce::dontSendNotification);
    mSliderPanAdjust.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::PAN_ADJUST),
                              juce::dontSendNotification);
    mSliderPanSpray.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::PAN_SPRAY),
                             juce::dontSendNotification);
    // TODO: set value of reverse and trigger mode
    mBtnReverse.setToggleState(mParameters.getBoolParam(mCurSelectedParams, ParamCommon::Type::REVERSE), juce::dontSendNotification);
  }
}

void AdjustPanel::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  
  mParamColour = mParameters.getSelectedParamColour();
  mSliderPitchAdjust.updateSelectedParams();
  mSliderPitchSpray.updateSelectedParams();
  mSliderPosAdjust.updateSelectedParams();
  mSliderPosSpray.updateSelectedParams();
  mSliderPanAdjust.updateSelectedParams();
  mSliderPanSpray.updateSelectedParams();
  mBtnReverse.setColour(juce::ToggleButton::ColourIds::tickColourId, mParamColour);
  mBtnTriggerMode.setColour(juce::ToggleButton::ColourIds::tickColourId, mParamColour);

  mParamHasChanged.store(true);
  repaint();
}

void AdjustPanel::paint(juce::Graphics& g) {
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
}

void AdjustPanel::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING, Utils::PADDING);
  
  const int knobWidth = r.getWidth() / 3;
  
  // Left: Pitch spray/adjust and trigger mode
  juce::Rectangle<int> knobPanel = r.removeFromLeft(knobWidth);
  mLabelPitchSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPitchSpray.setBounds(knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  knobPanel.removeFromBottom(Utils::PADDING);
  mLabelPitchAdjust.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPitchAdjust.setBounds(
                               knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  // Trigger mode button
  knobPanel.removeFromBottom(Utils::PADDING);
  mLabelTriggerMode.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mBtnTriggerMode.setBounds(knobPanel.withSizeKeepingCentre(Utils::BUTTON_WIDTH, Utils::LABEL_HEIGHT));
  
  // Right: Position spray/adjust and ref tone
  knobPanel = r.removeFromRight(knobWidth);
  mLabelPosSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPosSpray.setBounds(knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  knobPanel.removeFromBottom(Utils::PADDING);
  mLabelPosAdjust.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPosAdjust.setBounds(
                             knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
   
  // Middle: Pan spray/adjust and reverse
  knobPanel = r;
  mLabelPanSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPanSpray.setBounds(knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  knobPanel.removeFromBottom(Utils::PADDING);
  mLabelPanAdjust.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPanAdjust.setBounds(
                             knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  // Reverse button
  knobPanel.removeFromBottom(Utils::PADDING);
  mLabelReverse.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mBtnReverse.setBounds(knobPanel.withSizeKeepingCentre(Utils::BUTTON_WIDTH, Utils::LABEL_HEIGHT));
}
