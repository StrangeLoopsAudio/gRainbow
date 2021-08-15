/*
  ==============================================================================

    GeneratorsBox.h
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GranularSynth.h"
#include "EnvelopeGrain.h"
#include "EnvelopeADSR.h"
#include "PositionChanger.h"
#include "Utils.h"

//==============================================================================
/*
 */
class GeneratorsBox : public juce::Component,
                      juce::AudioProcessorParameter::Listener,
                      juce::Timer {
 public:
  GeneratorsBox(ParamsNote& paramsNote, ParamUI& paramUI);
  ~GeneratorsBox() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void mouseMove(const juce::MouseEvent& event) override;
  void mouseExit(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event) override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void setPitchClass(Utils::PitchClass pitchClass);

  std::function<void(int gen, bool isRight)> onPositionChanged = nullptr;

 private:
  // Params
  static constexpr auto NUM_AMP_ENV_PARAMS = 4;
  static constexpr auto NUM_GRAIN_ENV_PARAMS = 4;

  // UI Layout
  static constexpr juce::int64 GRAIN_SYNC_COLOURS_HEX[2] = {0xFF20FFD4,
                                                            0xFFFFD420};

  static constexpr auto TABS_HEIGHT = 30;
  static constexpr auto PADDING_SIZE = 6;
  static constexpr auto ADJUSTMENT_HEIGHT = 40;
  static constexpr auto TOGGLE_SIZE = 16;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto ENVELOPE_HEIGHT = 60;
  static constexpr auto SECTION_TITLE_HEIGHT = 20;
  static constexpr auto SECTION_AMP_ENV_TITLE = "amplitude envelope";
  static constexpr auto SECTION_GRAIN_ENV_TITLE = "grain envelope";
  static constexpr auto SECTION_ADJUST_TITLE = "generator adjustments";

  // Parameters
  ParamsNote& mParamsNote;
  ParamUI& mParamUI;

  // Bookkeeping
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::C;
  Utils::GeneratorColour mCurSelectedGenerator = Utils::GeneratorColour::BLUE;
  int mCurHoverGenerator = -1;
  std::atomic<bool> mParamHasChanged;

  class QuantizedSlider : public juce::Slider {
   public:
    QuantizedSlider() : juce::Slider() {}
    QuantizedSlider(juce::NormalisableRange<float> range) : mRange(range), juce::Slider() {}
    void setSync(bool sync) { mSync = sync; }

    juce::String getTextFromValue(double value) override {
      if (mSync) {
        float prog = mRange.convertTo0to1(getValue());
        return juce::String("1/") +
               juce::String(std::pow(2, (int)(MAX_SYNC_LOG * prog)));
      } else {
        return juce::String(getValue());
      }
    }

   private:
    static constexpr auto MAX_SYNC_LOG = 5;
    bool mSync;
    juce::NormalisableRange<float> mRange;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizedSlider)
  };

  // UI Components
  // -- Generator Tabs
  std::array<juce::ToggleButton, Utils::GeneratorColour::NUM_GEN> mBtnsEnabled;
  // -- Generator Adjustments
  PositionChanger mPositionChanger;
  juce::Slider mSliderPitch;
  juce::Label mLabelPitch;
  juce::Slider mSliderPosition;
  juce::Label mLabelPosition;
  // -- Grain Env
  juce::Slider mSliderShape;
  juce::Label mLabelShape;
  juce::Slider mSliderTilt;
  juce::Label mLabelTilt;
  QuantizedSlider mSliderRate;
  juce::Label mLabelRate;
  QuantizedSlider mSliderDuration;
  juce::Label mLabelDuration;
  juce::TextButton mBtnSync;
  EnvelopeGrain mEnvelopeGrain;
  // -- ADSR Env
  juce::Slider mSliderAttack;
  juce::Label mLabelAttack;
  juce::Slider mSliderDecay;
  juce::Label mLabelDecay;
  juce::Slider mSliderSustain;
  juce::Label mLabelSustain;
  juce::Slider mSliderRelease;
  juce::Label mLabelRelease;
  EnvelopeADSR mEnvelopeAmp;
  juce::Slider mSliderGain;
  juce::Label mLabelGain;

  void changeGenerator(Utils::GeneratorColour newGenerator);
  void refreshState();
  inline ParamGenerator* getCurrentGenerator() {
    return mParamsNote.notes[mCurPitchClass]
        ->generators[mCurSelectedGenerator]
        .get();
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratorsBox)
};
