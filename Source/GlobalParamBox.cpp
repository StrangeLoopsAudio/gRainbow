/*
  ==============================================================================

    GlobalParamBox.cpp
    Created: 30 Jul 2021 2:50:47pm
    Author:  brady

  ==============================================================================
*/

#include "GlobalParamBox.h"

#include <JuceHeader.h>

//==============================================================================
GlobalParamBox::GlobalParamBox() {

  juce::Colour mainColour = juce::Colours::white;
  /* Amp envelope viz */
  mEnvelopeAmp.setActive(true);
  mEnvelopeAmp.setColour(mainColour);
  addAndMakeVisible(mEnvelopeAmp);

  /* Knob params */
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true; 

  /* Attack */
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                          mainColour);
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         mainColour);
  mSliderAttack.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderAttack.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderAttack.setRotaryParameters(rotaryParams);
  mSliderAttack.setRange(0.0, 1.0, 0.01);
  mSliderAttack.onValueChange = [this] {
    mEnvelopeAmp.setAttack(mSliderAttack.getValue());
    parameterChanged(GranularSynth::ParameterType::ATTACK,
                     mSliderAttack.getValue());
  };
  addAndMakeVisible(mSliderAttack);

  mLabelAttack.setText("Attack", juce::dontSendNotification);
  mLabelAttack.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelAttack);

  /* Decay */
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                          mainColour);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                           mainColour);
  mSliderDecay.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDecay.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDecay.setRotaryParameters(rotaryParams);
  mSliderDecay.setRange(0.0, 1.0, 0.01);
  mSliderDecay.onValueChange = [this] {
    mEnvelopeAmp.setDecay(mSliderDecay.getValue());
    parameterChanged(GranularSynth::ParameterType::DECAY,
                     mSliderDecay.getValue());
  };
  addAndMakeVisible(mSliderDecay);

  mLabelDecay.setText("Decay", juce::dontSendNotification);
  mLabelDecay.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDecay);

  /* Sustain */
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         mainColour);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                           mainColour);
  mSliderSustain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderSustain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderSustain.setRotaryParameters(rotaryParams);
  mSliderSustain.setRange(0.0, 1.0, 0.01);
  mSliderSustain.onValueChange = [this] {
    mEnvelopeAmp.setSustain(mSliderSustain.getValue());
    parameterChanged(GranularSynth::ParameterType::SUSTAIN,
                     mSliderSustain.getValue());
  };
  addAndMakeVisible(mSliderSustain);

  mLabelSustain.setText("Sustain", juce::dontSendNotification);
  mLabelSustain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelSustain);

  /* Release */
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                           mainColour);
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                           mainColour);
  mSliderRelease.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRelease.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRelease.setRotaryParameters(rotaryParams);
  mSliderRelease.setRange(0.0, 1.0, 0.01);
  mSliderRelease.onValueChange = [this] {
    mEnvelopeAmp.setRelease(mSliderRelease.getValue());
    parameterChanged(GranularSynth::ParameterType::RELEASE,
                     mSliderRelease.getValue());
  };
  addAndMakeVisible(mSliderRelease);

  mLabelRelease.setText("Release", juce::dontSendNotification);
  mLabelRelease.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRelease);
}

GlobalParamBox::~GlobalParamBox() {}

void GlobalParamBox::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  juce::Colour mainColour = juce::Colours::white;
  
  // Global param title
  juce::Rectangle<float> titleRect =
      juce::Rectangle<float>(0.0f, 0.0f, getWidth(), MAIN_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE);
  g.setColour(mainColour);
  g.fillRoundedRectangle(titleRect, 10.0f);

  g.setColour(juce::Colours::black);
  g.drawText(juce::String(MAIN_TITLE), titleRect, juce::Justification::centred);

  // Amp env section title
  juce::Rectangle<float> ampEnvTitleRect =
      juce::Rectangle<float>(
          0.0f,
          mEnvelopeAmp.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f),
          getWidth(), SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2.0f);
  g.setColour(mainColour);
  g.fillRoundedRectangle(ampEnvTitleRect, 10.0f);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_AMP_ENV_TITLE), ampEnvTitleRect,
             juce::Justification::centred);

  // Outline rect
  g.setColour(mainColour);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
}

void GlobalParamBox::resized() {
  auto r = getLocalBounds();
  // Add insets
  r.removeFromTop(PADDING_SIZE);
  r.removeFromLeft(PADDING_SIZE);
  r.removeFromRight(PADDING_SIZE);
  r.removeFromBottom(PADDING_SIZE);

  r.removeFromTop(MAIN_TITLE_HEIGHT);

  // Amp envelope
  r.removeFromTop(SECTION_TITLE_HEIGHT);
  mEnvelopeAmp.setBounds(r.removeFromTop(ENVELOPE_HEIGHT));
  r.removeFromTop(PADDING_SIZE);

  // Amp env knobs
  auto knobWidth = r.getWidth() / NUM_AMP_ENV_PARAMS;
  auto knobPanel = r.removeFromTop(knobWidth / 2);
  mSliderAttack.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDecay.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderSustain.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRelease.setBounds(knobPanel.removeFromLeft(knobWidth));

  auto labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelAttack.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDecay.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelSustain.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelRelease.setBounds(labelPanel.removeFromLeft(knobWidth));
}

void GlobalParamBox::setParams(Utils::GlobalParams params) {
  mSliderAttack.setValue(params.attack, juce::dontSendNotification);
  mEnvelopeAmp.setAttack(params.attack);
  mSliderDecay.setValue(params.decay, juce::dontSendNotification);
  mEnvelopeAmp.setDecay(params.decay);
  mSliderSustain.setValue(params.sustain, juce::dontSendNotification);
  mEnvelopeAmp.setSustain(params.sustain);
  mSliderRelease.setValue(params.release, juce::dontSendNotification);
  mEnvelopeAmp.setRelease(params.release);
}

void GlobalParamBox::parameterChanged(GranularSynth::ParameterType type,
                                    float value) {
  if (onParameterChanged != nullptr) {
    onParameterChanged(type, value);
  }
}