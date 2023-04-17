/*
  ==============================================================================

    EnvelopeGrain.h
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Parameters.h"
#include "../RainbowLookAndFeel.h"

//==============================================================================
/*
 */
class EnvelopeGrain : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  EnvelopeGrain(Parameters& parameters);
  ~EnvelopeGrain();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();

 private:
  static constexpr const char* SECTION_TITLE = "grain envelope";
  static constexpr float MIN_RATE_RATIO = .25f;
  static constexpr float MAX_RATE_RATIO = 1.0f;
  static constexpr float GAIN_HEIGHT = 0.8f;
  static constexpr int MAX_NUM_ENVS = 6;
  static constexpr juce::int64 GRAIN_SYNC_COLOURS_HEX[2] = {0xFF20FFD4, 0xFFFFD420};

  class QuantizedSlider : public RainbowSlider {
   public:
    QuantizedSlider(Parameters& parameters, ParamCommon::Type type) : mSync(false), RainbowSlider(parameters, type) {}
    QuantizedSlider(Parameters& parameters, ParamCommon::Type type, juce::NormalisableRange<float> range)
        : mRange(range), RainbowSlider(parameters, type) {}
    void setSync(bool sync) { mSync = sync; }

    juce::String getTextFromValue(double value) override {
      if (mSync) {
        float prog = mRange.convertTo0to1(getValue());
        return juce::String("1/") + juce::String(std::pow(2, (int)(ParamRanges::SYNC_DIV_MAX * prog)));
      } else {
        return juce::String(getValue());
      }
    }

   private:
    bool mSync;
    juce::NormalisableRange<float> mRange;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizedSlider)
  };

  juce::PathStrokeType mPathStroke;

  // Components
  RainbowSlider mSliderShape;
  RainbowSlider mSliderTilt;
  QuantizedSlider mSliderRate;
  QuantizedSlider mSliderDuration;
  juce::TextButton mBtnSync;
  juce::Label mLabelShape;
  juce::Label mLabelTilt;
  juce::Label mLabelRate;
  juce::Label mLabelDuration;

  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour = Utils::GLOBAL_COLOUR;

  // UI values saved on resize
  juce::Rectangle<float> mTitleRect;
  juce::Rectangle<float> mVizRect;

  float limitEnvX(float x);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeGrain)
};
