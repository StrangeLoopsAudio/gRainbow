/*
  ==============================================================================

    Settings.h
    Created: 8 Jun 2022 2:46:05pm
    Author:  fricke

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../DSP/GranularSynth.h"

/**
    This class holds the state of the settings that are known globally at all times
*/
class PowerUserSettings {
 public:
  PowerUserSettings() : mIsAnimated(true), mIsResourceUsage(true), mSynth(nullptr) {}
  ~PowerUserSettings() {}

  void setSynth(GranularSynth* synth) { mSynth = synth; }

  void setAnimated(bool value) { mIsAnimated = value; }
  bool getAnimated() { return mIsAnimated; }

  void setResourceUsage(bool value) { mIsResourceUsage = value; }
  bool getResourceUsage() { return mIsResourceUsage; }

  void resetParameters();

  // Creates a singleton
  PowerUserSettings(PowerUserSettings const&) = delete;
  void operator=(PowerUserSettings const&) = delete;
  static PowerUserSettings& get() {
    static PowerUserSettings instance;
    return instance;
  }

 private:
  bool mIsAnimated;
  bool mIsResourceUsage;

  GranularSynth* mSynth;
};

/**
    This class is the component part of the settings, it lets you edit the settings but destroyed when the editor closes
*/
class SettingsComponent : public juce::Component {
 public:
  SettingsComponent();
  ~SettingsComponent() override;

  void paint(juce::Graphics& g) override;
  void resized() override;

  // height of setting component
  int getHeight() { return 100; }

private:
  const int mDivideLineSize = 5;
  juce::TextButton mBtnAnimation;
  juce::TextButton mBtnResetParameters;
  juce::TextButton mBtnResourceUsage;
};
