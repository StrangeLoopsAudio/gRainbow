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
      mSliderGain(parameters, ParamCommon::Type::GAIN),
      mSliderMaxGrains(mParameters.global.maxGrains),
      mSliderMacro1(mParameters.global.macro1),
      mSliderMacro2(mParameters.global.macro2),
      mSliderMacro3(mParameters.global.macro3),
      mSliderMacro4(mParameters.global.macro4) {
  juce::Colour colour = Utils::GLOBAL_COLOUR;
        
  // Titles
  mLabelTitle.setText("master", juce::dontSendNotification);
  mLabelTitle.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelTitle.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mLabelTitle);
  mLabelMacros.setText("macros", juce::dontSendNotification);
  mLabelMacros.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelMacros.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mLabelMacros);

  // Meter
  mMeter.setMeterSource(&meterSource);
  mMeter.setLookAndFeel(&mMeterLookAndFeel);
  addAndMakeVisible(mMeter);
  
  // Default slider settings
  std::vector<std::reference_wrapper<juce::Slider>> sliders = { mSliderGain, mSliderMaxGrains, mSliderMacro1, mSliderMacro2, mSliderMacro3, mSliderMacro4 };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(slider.get());
  }

  mSliderGain.setRange(ParamRanges::GAIN.start, ParamRanges::GAIN.end, 0.01);
  mSliderMaxGrains.setRange(ParamRanges::MAX_GRAINS.start, ParamRanges::MAX_GRAINS.end, 1);
  mSliderMacro1.setRange(ParamRanges::MACRO.start, ParamRanges::MACRO.end, 0.01);
  mSliderMacro2.setRange(ParamRanges::MACRO.start, ParamRanges::MACRO.end, 0.01);
  mSliderMacro3.setRange(ParamRanges::MACRO.start, ParamRanges::MACRO.end, 0.01);
  mSliderMacro4.setRange(ParamRanges::MACRO.start, ParamRanges::MACRO.end, 0.01);
  
  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelGain, mLabelMaxGrains, mLabelMacro1, mLabelMacro2, mLabelMacro3, mLabelMacro4 };
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
    label.get().setJustificationType(juce::Justification::centredTop);
    label.get().setFont(juce::Font(14));
    addAndMakeVisible(label.get());
  }

  mLabelGain.setText("gain", juce::dontSendNotification);
  mLabelMaxGrains.setText("max grains", juce::dontSendNotification);
  mLabelMacro1.setText("macro 1", juce::dontSendNotification);
  mLabelMacro2.setText("macro 2", juce::dontSendNotification);
  mLabelMacro3.setText("macro 3", juce::dontSendNotification);
  mLabelMacro4.setText("macro 4", juce::dontSendNotification);

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

MasterPanel::~MasterPanel() {
  mCurSelectedParams->removeListener(this);
  mMeter.setLookAndFeel(nullptr);
}

void MasterPanel::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void MasterPanel::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderGain.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::GAIN), juce::dontSendNotification);
    mSliderMaxGrains.setValue(mParameters.global.maxGrains->get(), juce::dontSendNotification);
    mSliderMacro1.setValue(mParameters.global.macro1->get(), juce::dontSendNotification);
    mSliderMacro2.setValue(mParameters.global.macro2->get(), juce::dontSendNotification);
    mSliderMacro3.setValue(mParameters.global.macro3->get(), juce::dontSendNotification);
    mSliderMacro4.setValue(mParameters.global.macro4->get(), juce::dontSendNotification);
  }
}

void MasterPanel::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
    
  mSliderGain.updateSelectedParams();

  mParamHasChanged.store(true);
  repaint();
}

void MasterPanel::paint(juce::Graphics& g) {
  juce::Colour colour = mParamColour;

  // Panel rectangle
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().toFloat(), 10);
  
  // Separator line between master and macros
  g.setColour(Utils::BG_COLOUR);
  g.drawLine(getWidth() / 2, Utils::PADDING, getWidth() / 2, getHeight() - Utils::PADDING, 2);
}

void MasterPanel::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING, Utils::PADDING);
  
  auto masterPanel = r.removeFromRight(r.getWidth() / 2);
  mLabelTitle.setBounds(masterPanel.removeFromTop(Utils::TAB_HEIGHT));
  // Meter
  mMeter.setBounds(masterPanel.removeFromRight(masterPanel.getWidth() / 2).reduced(Utils::PADDING * 2, Utils::PADDING * 2));
  
  int knobWidth = masterPanel.getWidth();
  // Gain
  auto topKnob = masterPanel.removeFromTop(masterPanel.getHeight() / 2);
  mLabelGain.setBounds(topKnob.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderGain.setBounds(topKnob.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  masterPanel.removeFromTop(Utils::PADDING);
  mSliderMaxGrains.setBounds(masterPanel.removeFromTop(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mLabelMaxGrains.setBounds(masterPanel.removeFromTop(Utils::LABEL_HEIGHT));
  
  auto macroPanel = r;
  mLabelMacros.setBounds(macroPanel.removeFromTop(Utils::TAB_HEIGHT));
  // Macros 1 and 3
  auto macrosLeft = macroPanel.removeFromLeft(macroPanel.getWidth() / 2);
  topKnob = macrosLeft.removeFromTop(macrosLeft.getHeight() / 2);
  mLabelMacro1.setBounds(topKnob.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderMacro1.setBounds(topKnob.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  macrosLeft.removeFromTop(Utils::PADDING);
  mSliderMacro3.setBounds(macrosLeft.removeFromTop(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mLabelMacro3.setBounds(macrosLeft.removeFromTop(Utils::LABEL_HEIGHT));
  // Macros 2 and 4
  topKnob = macroPanel.removeFromTop(macroPanel.getHeight() / 2);
  mLabelMacro2.setBounds(topKnob.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderMacro2.setBounds(topKnob.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  macroPanel.removeFromTop(Utils::PADDING);
  mSliderMacro4.setBounds(macroPanel.removeFromTop(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mLabelMacro4.setBounds(macroPanel.removeFromTop(Utils::LABEL_HEIGHT));
}
