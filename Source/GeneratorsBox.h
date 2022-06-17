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
#include "FilterControl.h"

//==============================================================================
/*
 */
class GeneratorsBox : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
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

  void setMidiNotes(const juce::Array<Utils::MidiNote>& midiNotes);
  int getSelectedGenerator() { return mCurSelectedGenerator; }
  std::function<void(int gen, bool isRight)> onPositionChanged = nullptr;

 private:
  // Params
  static constexpr auto NUM_AMP_ENV_PARAMS = 4;
  static constexpr auto NUM_GRAIN_ENV_PARAMS = 4;

  // UI Layout
  static constexpr juce::int64 GRAIN_SYNC_COLOURS_HEX[2] = {0xFF20FFD4, 0xFFFFD420};

  static constexpr auto TABS_HEIGHT = 30;
  static constexpr auto PADDING_SIZE = 6;
  static constexpr auto ADJUSTMENT_HEIGHT = 40;
  static constexpr auto TOGGLE_SIZE = 16;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto HORIZONTAL_SLIDER_HEIGHT = 15;
  static constexpr auto ENVELOPE_HEIGHT = 60;
  static constexpr auto SECTION_TITLE_HEIGHT = 20;
  static constexpr auto FILTER_CONTROL_HEIGHT = 100;
  static constexpr auto SECTION_AMP_ENV_TITLE = "amplitude envelope";
  static constexpr auto SECTION_GRAIN_ENV_TITLE = "grain envelope";
  static constexpr auto SECTION_ADJUST_TITLE = "generator adjustments";
  static constexpr auto SECTION_FILTER_ENV_TITLE = "filter control";
  // Parameters
  ParamsNote& mParamsNote;
  ParamUI& mParamUI;

  // Bookkeeping
  Utils::PitchClass mCurPitchClass;
  int mCurSelectedGenerator = 0;
  int mCurHoverGenerator = -1;
  std::atomic<bool> mParamHasChanged;

  class QuantizedSlider : public juce::Slider {
   public:
    QuantizedSlider() : mSync(false), juce::Slider() {}
    QuantizedSlider(juce::NormalisableRange<float> range) : mRange(range), juce::Slider() {}
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

  // UI Components
  // -- Generator Tabs
  std::array<juce::ToggleButton, NUM_GENERATORS> mBtnsEnabled;
  // -- Generator Adjustments
  PositionChanger mPositionChanger;
  juce::Slider mSliderPitchAdjust;
  juce::Label mLabelPitchAdjust;
  juce::Slider mSliderPitchSpray;
  juce::Label mLabelPitchSpray;
  juce::Slider mSliderPosAdjust;
  juce::Label mLabelPosAdjust;
  juce::Slider mSliderPosSpray;
  juce::Label mLabelPosSpray;
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
  /* -- Filter Control*/
  FilterControl mFilterControl;
  juce::Slider mSliderCutoff;
  juce::Label mLabelCutoff;
  juce::Slider mSliderResonance;
  juce::Label mLabelResonance;

  void changeGenerator(int newGenerator);
  void refreshState();
  inline ParamGenerator* getCurrentGenerator() {
    return mParamsNote.notes[mCurPitchClass]->generators[mCurSelectedGenerator].get();
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratorsBox)
};
