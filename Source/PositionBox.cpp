/*
  ==============================================================================

    PositionBox.cpp
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#include "PositionBox.h"
#include "Utils.h"
#include <JuceHeader.h>

//==============================================================================
PositionBox::PositionBox() {

  mPositionChanger.onPositionChanged = [this](bool isRight) {
    if (onPositionChanged != nullptr) {
      onPositionChanged(isRight);
    }
  };
  addAndMakeVisible(mPositionChanger);

  mBtnEnabled.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::darkgrey);
  mBtnEnabled.onClick = [this] {
    mIsActive = !mIsActive;
    setState(mState);
    parameterChanged(GranularSynth::ParameterType::ENABLED, mBtnEnabled.getToggleState());
  };
  addAndMakeVisible(mBtnEnabled);

  mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId,
                        juce::Colours::blue);
  mBtnSolo.onClick = [this] {
    setState(mBtnSolo.getToggleState() ? BoxState::SOLO : BoxState::READY);
    parameterChanged(GranularSynth::ParameterType::SOLO,
                     mBtnSolo.getToggleState());
  };
  addAndMakeVisible(mBtnSolo);

  /* Knob params */
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi; 
  rotaryParams.stopAtEnd = true; 

  mLabelRate.setEnabled(mState == BoxState::READY);
  mLabelDuration.setEnabled(mState == BoxState::READY);
  mLabelGain.setEnabled(mState == BoxState::READY);

  /* Rate */
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRate.setRotaryParameters(rotaryParams);
  mSliderRate.setRange(0.0, 1.0, 0.01);
  mSliderRate.onValueChange = [this] {
    mGrainEnvelopes.setRate(mSliderRate.getValue());
    parameterChanged(GranularSynth::ParameterType::RATE,
                     mSliderRate.getValue());
  };
  mSliderRate.setValue(PARAM_RATE_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderRate);

  mLabelRate.setText("Rate", juce::dontSendNotification);
  mLabelRate.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRate);

  /* Duration */
  mSliderDuration.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDuration.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDuration.setRotaryParameters(rotaryParams);
  mSliderDuration.setRange(0.0, 1.0, 0.01);
  mSliderDuration.onValueChange = [this] {
    mGrainEnvelopes.setDuration(mSliderDuration.getValue());
    parameterChanged(GranularSynth::ParameterType::DURATION,
                     mSliderDuration.getValue());
  };
  mSliderDuration.setValue(PARAM_DURATION_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderDuration);

  mLabelDuration.setText("Duration", juce::dontSendNotification);
  mLabelDuration.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDuration);

  /* Gain */
  mSliderGain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderGain.setRotaryParameters(rotaryParams);
  mSliderGain.setRange(0.0, 1.0, 0.01);
  mSliderGain.onValueChange = [this] {
    mGrainEnvelopes.setGain(mSliderGain.getValue());
    parameterChanged(GranularSynth::ParameterType::GAIN,
                     mSliderGain.getValue());
  };
  mSliderGain.setValue(PARAM_GAIN_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderGain);

  mLabelGain.setText("Gain", juce::dontSendNotification);
  mLabelGain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelGain);

  /* Envelope viz */
  addAndMakeVisible(mGrainEnvelopes);
}

PositionBox::~PositionBox() {}

void PositionBox::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  bool borderLit = (mIsActive || mState == BoxState::SOLO);
  g.setColour(borderLit ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                        : juce::Colours::darkgrey);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 10.0f, 2.0f);
}

void PositionBox::resized() {
  auto r = getLocalBounds();
  // Add insets
  r.removeFromTop(PADDING_SIZE);
  r.removeFromLeft(PADDING_SIZE);
  r.removeFromRight(PADDING_SIZE);
  r.removeFromBottom(PADDING_SIZE);

  // Enable and solo buttons
  auto btnPanel = r.removeFromTop(TOGGLE_SIZE);
  mBtnEnabled.setBounds(btnPanel.removeFromLeft(TOGGLE_SIZE));
  mBtnSolo.setBounds(btnPanel.removeFromRight(TOGGLE_SIZE));
  mPositionChanger.setBounds(btnPanel.withSizeKeepingCentre(
      btnPanel.getWidth() * 0.7, btnPanel.getHeight()));
  
  /* Envelopes */
  mGrainEnvelopes.setBounds(r.removeFromTop(ENVELOPE_HEIGHT));
  r.removeFromTop(PADDING_SIZE);

  // Knobs and labels
  auto knobWidth = r.getWidth() / NUM_PARAMS;
  auto knobPanel = r.removeFromTop(knobWidth / 2);
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDuration.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderGain.setBounds(knobPanel.removeFromLeft(knobWidth));

  auto labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelRate.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDuration.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelGain.setBounds(labelPanel.removeFromLeft(knobWidth));
}

void PositionBox::setActive(bool isActive) {
  mIsActive = isActive;
  mBtnEnabled.setToggleState(isActive, juce::dontSendNotification);
  setState(mState);
}

void PositionBox::setPositions(std::vector<int> positions, int numPositions) {
  mPositionChanger.setGlobalPositions(positions, numPositions);
}

void PositionBox::setState(BoxState state) {
  mState = state;

  if (state == BoxState::SOLO_WAIT) {
    mBtnSolo.setToggleState(false, juce::dontSendNotification);
  }

  juce::Colour enabledColour = (state != BoxState::SOLO_WAIT)
          ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                                     : juce::Colours::darkgrey;
  mBtnEnabled.setColour(juce::ToggleButton::ColourIds::tickColourId,
                        enabledColour);
  juce::Colour soloColour = (state != BoxState::SOLO_WAIT)
                                   ? juce::Colours::blue
                                   : juce::Colours::darkgrey;
  mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId, soloColour);

  bool componentsLit = (mIsActive && state == BoxState::READY || state == BoxState::SOLO);
  juce::Colour knobColour = componentsLit
                                ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                                : juce::Colours::darkgrey;
  mPositionChanger.setActive(componentsLit);
  mGrainEnvelopes.setActive(componentsLit);
  
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                            knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mLabelRate.setEnabled(componentsLit);
  mLabelDuration.setEnabled(componentsLit);
  mLabelGain.setEnabled(componentsLit);
  repaint();
}

GranularSynth::PositionParams PositionBox::getParams() {
  return GranularSynth::PositionParams(
      mBtnEnabled.getToggleState(), mBtnSolo.getToggleState(),
      mSliderRate.getValue(), mSliderDuration.getValue(),
      mSliderGain.getValue());
}

void PositionBox::setColour(GranularSynth::PositionColour colour) {
  mColour = colour;
  if (mState == BoxState::READY) {
    mBtnEnabled.setColour(juce::ToggleButton::ColourIds::tickColourId,
                          juce::Colour(Utils::POSITION_COLOURS[colour]));
    mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId,
                       juce::Colours::blue);
  }
  mPositionChanger.setColour(colour,
                             juce::Colour(Utils::POSITION_COLOURS[colour]));
  mGrainEnvelopes.setColour(juce::Colour(Utils::POSITION_COLOURS[colour]));
  repaint();
}

void PositionBox::parameterChanged(GranularSynth::ParameterType type,
                                   float value) {
  if (onParameterChanged != nullptr) {
    onParameterChanged(mColour, type, value);
  }
}