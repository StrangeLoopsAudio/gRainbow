/*
  ==============================================================================

    Settings.cpp
    Created: 8 Jun 2022 2:46:05pm
    Author:  fricke

  ==============================================================================
*/

#include "Settings.h"

void PowerUserSettings::resetParameters() {
  if (mSynth != nullptr) {
    mSynth->resetParameters(false);
  }
}

SettingsComponent::SettingsComponent() {
  mBtnAnimation.setButtonText("Run animation");
  mBtnAnimation.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
  mBtnAnimation.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
  mBtnAnimation.setToggleState(PowerUserSettings::get().getAnimated(), juce::NotificationType::dontSendNotification);
  mBtnAnimation.setClickingTogglesState(true);
  mBtnAnimation.onClick = [this] { PowerUserSettings::get().setAnimated(mBtnAnimation.getToggleState()); };
  addAndMakeVisible(mBtnAnimation);

  mBtnResetParameters.setButtonText("Reset Parameters");
  mBtnResetParameters.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
  mBtnResetParameters.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
  mBtnResetParameters.onClick = [this] { PowerUserSettings::get().resetParameters(); };
  addAndMakeVisible(mBtnResetParameters);
}

SettingsComponent::~SettingsComponent() {}

void SettingsComponent::paint(juce::Graphics& g) {
  g.drawLine(0.0f, 0.0f, static_cast<float>(getWidth()), 0.0f, static_cast<float>(mDivideLineSize));
}

void SettingsComponent::resized() {
  auto r = getLocalBounds();
  r.removeFromTop(mDivideLineSize);

  const int buttonHeight = 30;
  const int buttonWidth = 100;
  mBtnAnimation.setBounds(r.removeFromTop(buttonHeight).withWidth(buttonWidth));
  mBtnResetParameters.setBounds(r.removeFromTop(buttonHeight).withWidth(buttonWidth));
}