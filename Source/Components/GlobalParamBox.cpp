/*
  ==============================================================================

    GlobalParamBox.cpp
    Created: 30 Jul 2021 2:50:47pm
    Author:  brady

  ==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>

#include "GlobalParamBox.h"

//==============================================================================
GlobalParamBox::GlobalParamBox(ParamGlobal& paramGlobal) : mParamGlobal(paramGlobal) {
  juce::Colour mainColour = juce::Colours::white;

  /* Knob params */
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;

  /* Gain */
  mSliderGain = std::make_unique<Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment> >(
      *mParamGlobal.gain, *this, [mainColour, rotaryParams](juce::Slider& slider) {
        slider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, mainColour);
        slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, mainColour);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setRotaryParameters(rotaryParams);
        slider.setNumDecimalPlacesToDisplay(2);
        slider.setRange(0.0, 1.0, 0.01);
      });
  mSliderGain->component.onValueChange = [this] { 
    // TODO: re-enable this
    //mEnvelopeAmp.setGain(mSliderGain->component.getValue()); 
  };
  mLabelGain.setText("Gain", juce::dontSendNotification);
  mLabelGain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelGain);

  /* Filter Cutoff */
  mSliderCutoff = std::make_unique<Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment> >(
      *mParamGlobal.filterCutoff, *this, [mainColour, rotaryParams](juce::Slider& slider) {
        slider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, mainColour);
        slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, mainColour);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setRotaryParameters(rotaryParams);
        slider.setNumDecimalPlacesToDisplay(2);
        slider.setRange(0.0, 1.0, 0.01);
      });
  mSliderCutoff->component.setTextValueSuffix("Hz");
  mSliderCutoff->component.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderCutoff->component.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderCutoff->component.setRotaryParameters(rotaryParams);
  mSliderCutoff->component.setRange(ParamRanges::CUTOFF.start, ParamRanges::CUTOFF.end, .01f);
  mSliderCutoff->component.onValueChange = [this] {
    mFilterControl.setCutoff(ParamRanges::CUTOFF.convertTo0to1(mParamGlobal.filterCutoff->get()));
  };

  mLabelCutoff.setText("Cutoff", juce::dontSendNotification);
  mLabelCutoff.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelCutoff);

  /* Filter Resonance */
  mSliderResonance = std::make_unique<Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment> >(
      *mParamGlobal.filterResonance, *this, [mainColour, rotaryParams](juce::Slider& slider) {
        slider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, mainColour);
        slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, mainColour);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setRotaryParameters(rotaryParams);
        slider.setNumDecimalPlacesToDisplay(2);
        slider.setRange(0.0, 1.0, 0.01);
      });
  mSliderResonance->component.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderResonance->component.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderResonance->component.setRotaryParameters(rotaryParams);
  mSliderResonance->component.setRange(0.0, 1.0, 0.01);
  mSliderResonance->component.onValueChange = [this] { mFilterControl.setResonance(mParamGlobal.filterResonance->get()); };

  mLabelResonance.setText("Resonance", juce::dontSendNotification);
  mLabelResonance.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelResonance);

  /* Filter type */
  mFilterControl.setColour(mainColour);
  mFilterControl.onFilterTypeChange = [this](Utils::FilterType filterType) {
    ParamHelper::setParam(mParamGlobal.filterType, filterType);
    switch (filterType) {
      case (Utils::FilterType::LOWPASS):
        ParamHelper::setParam(mParamGlobal.filterCutoff, ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
        break;
      case (Utils::FilterType::HIGHPASS):
        ParamHelper::setParam(mParamGlobal.filterCutoff, ParamDefaults::FILTER_HP_CUTOFF_DEFAULT_HZ);
        break;
      case (Utils::FilterType::BANDPASS):
        ParamHelper::setParam(mParamGlobal.filterCutoff, ParamDefaults::FILTER_BP_CUTOFF_DEFAULT_HZ);
        break;
    }
    ParamHelper::setParam(mParamGlobal.filterResonance, ParamDefaults::FILTER_RESONANCE_DEFAULT);
  };
  // TODO: listen for filter type change and:
  //  mFilterControl.setFilterType(gen.filterType->getIndex());
  addAndMakeVisible(mFilterControl);
}

GlobalParamBox::~GlobalParamBox() {}

void GlobalParamBox::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  juce::Colour mainColour = juce::Colours::white;

  // Global param title
  g.setColour(mainColour);
  g.fillRoundedRectangle(mTitleRect, 10.0f);

  g.setColour(juce::Colours::black);
  g.drawText(juce::String(MAIN_TITLE), mTitleRect, juce::Justification::centred);

  // Filter Control env section title
  g.setColour(mainColour);
  g.fillRect(mFilterTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_FILTER_ENV_TITLE), mFilterTitleRect, juce::Justification::centred);

  // Outline rect
  g.setColour(mainColour);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
}

void GlobalParamBox::resized() {
  juce::Rectangle<int> r = getLocalBounds();

  int padding = getWidth() * PADDING_SIZE;
  // Add insets
  r.removeFromTop(padding);
  r.removeFromLeft(padding);
  r.removeFromRight(padding);
  r.removeFromBottom(padding);

  r.removeFromTop(padding);

  // Filter Control
  juce::Rectangle<int> filterPanel = r;
  mFilterTitleRect = filterPanel.removeFromTop(SECTION_TITLE_HEIGHT * getHeight());

  mFilterControl.setBounds(filterPanel);

  r.removeFromTop(padding);

  // Filter env knobs
  int knobWidth = r.getWidth() / 3;
  auto knobPanel = r.removeFromTop(PADDING_SIZE + knobWidth / 2);
  auto cutoffPanel = knobPanel.removeFromLeft(knobPanel.getWidth() / 2);
  mSliderCutoff->component.setBounds(cutoffPanel.removeFromRight(knobWidth));
  mSliderResonance->component.setBounds(knobPanel.removeFromLeft(knobWidth));

  auto labelPanel = r.removeFromTop(LABEL_HEIGHT);
  cutoffPanel = labelPanel.removeFromLeft(labelPanel.getWidth() / 2);
  mLabelCutoff.setBounds(cutoffPanel.removeFromRight(knobWidth));
  mLabelResonance.setBounds(labelPanel.removeFromLeft(knobWidth));
}