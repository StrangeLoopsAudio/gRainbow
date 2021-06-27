/*
  ==============================================================================

    PositionBox.cpp
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#include "PositionBox.h"

#include <JuceHeader.h>

//==============================================================================
PositionBox::PositionBox() {
  addAndMakeVisible(mBtnEnabled);
  addAndMakeVisible(mBtnSolo);

  /* Rate */
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
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

  g.setColour(juce::Colour(POSITION_COLOURS[mColour]));
  g.drawRoundedRectangle(getLocalBounds().toFloat(), 10.0f, 2.0f);
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
  btnPanel.removeFromLeft(PADDING_SIZE);
  mBtnSolo.setBounds(btnPanel.removeFromLeft(TOGGLE_SIZE));

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

  /* Envelopes */
  mGrainEnvelopes.setBounds(r.removeFromTop(ENVELOPE_HEIGHT));
}

void PositionBox::setColour(GranularSynth::PositionColour colour) {
  mColour = colour;
  mGrainEnvelopes.setColour(juce::Colour(POSITION_COLOURS[colour]));
  repaint();
}

void PositionBox::parameterChanged(GranularSynth::ParameterType type,
                                   float value) {
  if (onParameterChanged != nullptr) {
    onParameterChanged(mColour, type, value);
  }
}