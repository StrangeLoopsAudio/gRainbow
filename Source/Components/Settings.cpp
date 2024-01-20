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
    mSynth->getParams().resetParams();
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
  mBtnResetParameters.onClick = [] { PowerUserSettings::get().resetParameters(); };
  addAndMakeVisible(mBtnResetParameters);

  mBtnResourceUsage.setButtonText("Resource Usage");
  mBtnResourceUsage.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
  mBtnResourceUsage.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
  mBtnResourceUsage.setToggleState(PowerUserSettings::get().getResourceUsage(), juce::NotificationType::dontSendNotification);
  mBtnResourceUsage.setClickingTogglesState(true);
  mBtnResourceUsage.setToggleState(false, juce::NotificationType::dontSendNotification);
  mBtnResourceUsage.onClick = [this] { PowerUserSettings::get().setResourceUsage(mBtnResourceUsage.getToggleState()); };
  addAndMakeVisible(mBtnResourceUsage);
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
  mBtnResourceUsage.setBounds(r.removeFromTop(buttonHeight).withWidth(buttonWidth));
}
