/*
  ==============================================================================

    LFOPanel.h
    Created: 15 Dec 2023 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "Components/Sliders.h"
#include "Components/Buttons.h"

//==============================================================================
/*
 */
class LFOPanel : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  LFOPanel(int modIdx, Parameters& parameters);
  ~LFOPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}
  void visibilityChanged() override;

  void timerCallback() override;

 private:
  static constexpr int NUM_LFO_SAMPLES = 250; // # samples in the LFO buffer
  static constexpr double WINDOW_SECONDS = Utils::UI_REFRESH_INTERVAL * 0.001 * NUM_LFO_SAMPLES; // Width in seconds of the display window

  void updateLfoPath();

  // Bookkeeping
  Parameters& mParameters;
  LFOModSource& mModLFO;
  std::atomic<bool> mParamHasChanged;
  juce::Path mLfoPath;
  int mBufDepthWrPos = 0;
  std::vector<float> mBufDepth; // LFO depth buffer

  // Components
  juce::ComboBox mChoiceShape;
  QuantizedGlobalSlider mSliderRate;
  ParamSlider mSliderPhase;
  juce::Label mLabelShape;
  juce::Label mLabelRate;
  juce::Label mLabelPhase;
  juce::TextButton mBtnSync;
  juce::TextButton mBtnBipolar;
  juce::TextButton mBtnRetrigger;
  MapButton mBtnMap;

  // UI values saved on resize
  juce::Rectangle<float> mVizRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOPanel)
};
