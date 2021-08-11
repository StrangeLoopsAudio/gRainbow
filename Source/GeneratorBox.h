/*
  ==============================================================================

    GeneratorBox.h
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
class GeneratorBox : public juce::Component {
 public:
  GeneratorBox(NoteParams& noteParams);
  ~GeneratorBox() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  std::function<void(int gen, bool isRight)> onPositionChanged = nullptr;

 private:
  /* Params */
  static constexpr auto NUM_AMP_ENV_PARAMS = 4;
  static constexpr auto NUM_GRAIN_ENV_PARAMS = 5;

  /* UI Layout */
  static constexpr auto PADDING_SIZE = 6;
  static constexpr auto ADJUSTMENT_HEIGHT = 40;
  static constexpr auto TOGGLE_SIZE = 16;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto ENVELOPE_HEIGHT = 60;
  static constexpr auto SECTION_TITLE_HEIGHT = 20;
  static constexpr auto SECTION_AMP_ENV_TITLE = "amplitude envelope";
  static constexpr auto SECTION_GRAIN_ENV_TITLE = "grain envelope";
  static constexpr auto SECTION_ADJUST_TITLE = "generator adjustments";

  /* Parameters */
  NoteParams& mNoteParams;

  /* Bookkeeping */
  Utils::GeneratorColour mColour;
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::C;
  Utils::GeneratorColour mCurSelectedTab = Utils::GeneratorColour::BLUE;

  /* UI Components */
  /* -- Generator Adjustments */
  PositionChanger mPositionChanger;
  juce::Slider mSliderPitch;
  juce::Label mLabelPitch;
  juce::Slider mSliderPosition;
  juce::Label mLabelPosition;
  /* -- Grain Env */
  juce::Slider mSliderShape;
  juce::Label mLabelShape;
  juce::Slider mSliderTilt;
  juce::Label mLabelTilt;
  juce::Slider mSliderRate;
  juce::Label mLabelRate;
  juce::Slider mSliderDuration;
  juce::Label mLabelDuration;
  juce::Slider mSliderGain;
  juce::Label mLabelGain;
  EnvelopeGrain mEnvelopeGrain;
  /* -- ADSR Env */
  juce::Slider mSliderAttack;
  juce::Label mLabelAttack;
  juce::Slider mSliderDecay;
  juce::Label mLabelDecay;
  juce::Slider mSliderSustain;
  juce::Label mLabelSustain;
  juce::Slider mSliderRelease;
  juce::Label mLabelRelease;
  EnvelopeADSR mEnvelopeAmp;

  void refreshState();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratorBox)
};
