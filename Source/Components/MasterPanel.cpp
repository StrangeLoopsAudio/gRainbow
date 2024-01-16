/*
  ==============================================================================

    MasterPanel.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "MasterPanel.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

MasterPanel::MasterPanel(Parameters& parameters, foleys::LevelMeterSource& meterSource)
: mParameters(parameters),
mCurSelectedParams(parameters.selectedParams),
mParamColour(Utils::GLOBAL_COLOUR),
mSliderGain(mParameters, ParamCommon::Type::GAIN),
mSliderMacro1(mParameters, mParameters.global.macros[0].macro),
mSliderMacro2(mParameters, mParameters.global.macros[1].macro),
mSliderMacro3(mParameters, mParameters.global.macros[2].macro),
mSliderMacro4(mParameters, mParameters.global.macros[3].macro),
mBtnMapMacro1(mParameters, mParameters.global.macros[0]),
mBtnMapMacro2(mParameters, mParameters.global.macros[1]),
mBtnMapMacro3(mParameters, mParameters.global.macros[2]),
mBtnMapMacro4(mParameters, mParameters.global.macros[3]) {
  juce::Colour colour = Utils::GLOBAL_COLOUR;

  // Titles
  mLabelTitle.setText("master", juce::dontSendNotification);
  mLabelTitle.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelTitle.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mLabelTitle);
  mLabelMacros.setText("macros", juce::dontSendNotification);
  mLabelMacros.setColour(juce::Label::ColourIds::textColourId, mParameters.global.macros[0].colour);
  mLabelMacros.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mLabelMacros);

  // Meter
  mMeter.setMeterSource(&meterSource);
  mMeter.setLookAndFeel(&mMeterLookAndFeel);
  addAndMakeVisible(mMeter);

  // Default slider settings
  std::vector<std::reference_wrapper<juce::Slider>> sliders = { mSliderGain, mSliderMacro1, mSliderMacro2, mSliderMacro3, mSliderMacro4 };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(slider.get());
  }
        
  // Macro slider settings
  std::vector<std::reference_wrapper<ParamSlider>> macroSliders = {mSliderMacro1, mSliderMacro2, mSliderMacro3, mSliderMacro4};
  int i = 0;
  for (auto& slider : macroSliders) {
    slider.get().setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, mParameters.global.macros[i].colour);
    i++;
  }
  mSliderGain.setRange(ParamRanges::GAIN.start, ParamRanges::GAIN.end, 0.01);
  mSliderGain.setDoubleClickReturnValue(true, ParamDefaults::GAIN_DEFAULT);
  mSliderMacro1.setRange(ParamRanges::MACRO.start, ParamRanges::MACRO.end, 0.01);
  mSliderMacro1.setDoubleClickReturnValue(true, ParamDefaults::MACRO_DEFAULT);
  mSliderMacro2.setRange(ParamRanges::MACRO.start, ParamRanges::MACRO.end, 0.01);
  mSliderMacro2.setDoubleClickReturnValue(true, ParamDefaults::MACRO_DEFAULT);
  mSliderMacro3.setRange(ParamRanges::MACRO.start, ParamRanges::MACRO.end, 0.01);
  mSliderMacro3.setDoubleClickReturnValue(true, ParamDefaults::MACRO_DEFAULT);
  mSliderMacro4.setRange(ParamRanges::MACRO.start, ParamRanges::MACRO.end, 0.01);
  mSliderMacro4.setDoubleClickReturnValue(true, ParamDefaults::MACRO_DEFAULT);

  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> macroLabels = { mLabelGain, mLabelRefTone, mLabelMacro1, mLabelMacro2, mLabelMacro3, mLabelMacro4 };
  for (auto& label : macroLabels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
    label.get().setJustificationType(juce::Justification::centredTop);
    label.get().setFont(juce::Font(14));
    addAndMakeVisible(label.get());
  }
  
  // Macro label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelMacro1, mLabelMacro2, mLabelMacro3, mLabelMacro4 };
  i = 0;
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, mParameters.global.macros[i].colour);
    i++;
  }

  // Default button settings
  std::vector<std::reference_wrapper<juce::TextButton>> buttons = { mBtnMapMacro1, mBtnMapMacro2, mBtnMapMacro3, mBtnMapMacro4 };
  i = 0;
  for (auto& button : buttons) {
    button.get().setToggleable(true);
    button.get().setColour(juce::TextButton::textColourOffId, mParameters.global.macros[i].colour);
    button.get().setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    button.get().setColour(juce::TextButton::buttonColourId, mParameters.global.macros[i].colour);
    button.get().setColour(juce::TextButton::buttonOnColourId, mParameters.global.macros[i].colour);
    addAndMakeVisible(button.get());
    i++;
  }
  
  mBtnRefTone.setColour(juce::ToggleButton::ColourIds::tickColourId, Utils::GLOBAL_COLOUR);
  addAndMakeVisible(mBtnRefTone);
  // Reference tone
  mBtnRefTone.onClick = [this]() {
    if (mBtnRefTone.getToggleState() && onRefToneOn != nullptr) {
      onRefToneOn();
    }
    else if (!mBtnRefTone.getToggleState() && onRefToneOff != nullptr) onRefToneOff();
  };
  mLabelRefTone.setText("ref tone", juce::dontSendNotification);

  mLabelGain.setText("gain", juce::dontSendNotification);
  mLabelMacro1.setText("macro 1", juce::dontSendNotification);
  mLabelMacro2.setText("macro 2", juce::dontSendNotification);
  mLabelMacro3.setText("macro 3", juce::dontSendNotification);
  mLabelMacro4.setText("macro 4", juce::dontSendNotification);

  mCurSelectedParams->addListener(this);
  mParameters.global.macros[0].macro->addListener(this);
  mParameters.global.macros[1].macro->addListener(this);
  mParameters.global.macros[2].macro->addListener(this);
  mParameters.global.macros[3].macro->addListener(this);
  updateSelectedParams();

  startTimer(Utils::UI_REFRESH_INTERVAL);
}

MasterPanel::~MasterPanel() {
  mCurSelectedParams->removeListener(this);
  mParameters.global.macros[0].macro->removeListener(this);
  mParameters.global.macros[1].macro->removeListener(this);
  mParameters.global.macros[2].macro->removeListener(this);
  mParameters.global.macros[3].macro->removeListener(this);
  mMeter.setLookAndFeel(nullptr);
}

void MasterPanel::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void MasterPanel::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderGain.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::GAIN), juce::dontSendNotification);
    mSliderMacro1.setValue(mParameters.global.macros[0].macro->get(), juce::dontSendNotification);
    mSliderMacro2.setValue(mParameters.global.macros[1].macro->get(), juce::dontSendNotification);
    mSliderMacro3.setValue(mParameters.global.macros[2].macro->get(), juce::dontSendNotification);
    mSliderMacro4.setValue(mParameters.global.macros[3].macro->get(), juce::dontSendNotification);
  }
}

void MasterPanel::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();

  Utils::PitchClass selectedPitch = mParameters.getSelectedPitchClass();
  // Turn ref tone off if global parameters
  if (selectedPitch == Utils::PitchClass::NONE && mBtnRefTone.getToggleState() && onRefToneOff != nullptr) {
    mBtnRefTone.setToggleState(false, juce::dontSendNotification);
    onRefToneOff();
  }
  // Change ref tone frequency if already active
  if (mBtnRefTone.getToggleState() && onRefToneOn != nullptr) onRefToneOn();
  // Disable ref tone button if global parameters
  mBtnRefTone.setEnabled(selectedPitch != Utils::PitchClass::NONE);

  mSliderGain.updateSelectedParams();
  mBtnRefTone.setColour(juce::ToggleButton::ColourIds::tickColourId, mParamColour);

  mParamHasChanged.store(true);
  repaint();
}

void MasterPanel::paint(juce::Graphics& g) {
  // Panel rectangle
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().toFloat(), 10);

  // Separator line between master and macros
  g.setColour(Utils::BG_COLOUR);
  g.drawLine(getWidth() / 2, Utils::PADDING, getWidth() / 2, getHeight() - Utils::PADDING, 2);
}

void MasterPanel::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING);

  auto masterPanel = r.removeFromRight(r.getWidth() / 2);
  mLabelTitle.setBounds(masterPanel.removeFromTop(Utils::TAB_HEIGHT));
  // Meter
  mMeter.setBounds(masterPanel.removeFromRight(masterPanel.getWidth() / 2).reduced(Utils::PADDING * 2, Utils::PADDING * 2));

  // Gain
  auto topKnob = masterPanel.removeFromTop(masterPanel.getHeight() / 2);
  mLabelGain.setBounds(topKnob.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderGain.setBounds(topKnob.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  masterPanel.removeFromTop(Utils::PADDING);
  // Reference tone button
  mBtnRefTone.setBounds(masterPanel.removeFromTop(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::BUTTON_WIDTH, Utils::LABEL_HEIGHT));
  mLabelRefTone.setBounds(masterPanel.removeFromTop(Utils::LABEL_HEIGHT));

  auto macroPanel = r;
  mLabelMacros.setBounds(macroPanel.removeFromTop(Utils::TAB_HEIGHT));
  // Macros 1 and 3
  auto macrosLeft = macroPanel.removeFromLeft(macroPanel.getWidth() / 2);
  topKnob = macrosLeft.removeFromTop(macrosLeft.getHeight() / 2);
  mBtnMapMacro1.setBounds(topKnob.removeFromBottom(Utils::LABEL_HEIGHT));
  mLabelMacro1.setBounds(topKnob.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderMacro1.setBounds(topKnob.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  macrosLeft.removeFromTop(Utils::PADDING);
  mSliderMacro3.setBounds(macrosLeft.removeFromTop(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mLabelMacro3.setBounds(macrosLeft.removeFromTop(Utils::LABEL_HEIGHT));
  mBtnMapMacro3.setBounds(macrosLeft.removeFromTop(Utils::LABEL_HEIGHT));
  // Macros 2 and 4
  topKnob = macroPanel.removeFromTop(macroPanel.getHeight() / 2);
  mBtnMapMacro2.setBounds(topKnob.removeFromBottom(Utils::LABEL_HEIGHT));
  mLabelMacro2.setBounds(topKnob.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderMacro2.setBounds(topKnob.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  macroPanel.removeFromTop(Utils::PADDING);
  mSliderMacro4.setBounds(macroPanel.removeFromTop(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mLabelMacro4.setBounds(macroPanel.removeFromTop(Utils::LABEL_HEIGHT));
  mBtnMapMacro4.setBounds(macroPanel.removeFromTop(Utils::LABEL_HEIGHT));
}
