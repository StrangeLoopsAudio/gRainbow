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
GlobalParamBox::GlobalParamBox(GlobalParams& globalParams) : mGlobalParams(globalParams) {

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
  mSliderAttack = std::make_unique<
      Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment> >(
      *mGlobalParams.attack, *this,
      [mainColour, rotaryParams](juce::Slider& slider) {
        slider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         mainColour);
        slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         mainColour);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setRotaryParameters(rotaryParams);
        slider.setRange(0.0, 1.0, 0.01);
      });
  mSliderAttack->component.onValueChange = [this] {
    mEnvelopeAmp.setAttack(mSliderAttack->component.getValue());
  };
  mLabelAttack.setText("Attack", juce::dontSendNotification);
  mLabelAttack.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelAttack); 

  /* Decay */
  mSliderDecay = std::make_unique<
      Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment> >(
      *mGlobalParams.decay, *this,
      [mainColour, rotaryParams](juce::Slider& slider) {
        slider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         mainColour);
        slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         mainColour);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setRotaryParameters(rotaryParams);
        slider.setRange(0.0, 1.0, 0.01);
      });
  mSliderDecay->component.onValueChange = [this] {
    mEnvelopeAmp.setDecay(mSliderDecay->component.getValue());
  };
  mLabelDecay.setText("Decay", juce::dontSendNotification);
  mLabelDecay.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDecay);

  /* Sustain */
  mSliderSustain = std::make_unique<
      Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment> >(
      *mGlobalParams.sustain, *this,
      [mainColour, rotaryParams](juce::Slider& slider) {
        slider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         mainColour);
        slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         mainColour);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setRotaryParameters(rotaryParams);
        slider.setRange(0.0, 1.0, 0.01);
      });
  mSliderSustain->component.onValueChange = [this] {
    mEnvelopeAmp.setSustain(mSliderSustain->component.getValue());
  };
  mLabelSustain.setText("Sustain", juce::dontSendNotification);
  mLabelSustain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelSustain);

  /* Release */
  mSliderRelease = std::make_unique<
      Utils::AttachedComponent<juce::Slider, juce::SliderParameterAttachment> >(
      *mGlobalParams.release, *this,
      [mainColour, rotaryParams](juce::Slider& slider) {
        slider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         mainColour);
        slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         mainColour);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setRotaryParameters(rotaryParams);
        slider.setRange(0.0, 1.0, 0.01);
      });
  mSliderRelease->component.onValueChange = [this] {
    mEnvelopeAmp.setRelease(mSliderRelease->component.getValue());
  };
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
  mSliderAttack->component.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDecay->component.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderSustain->component.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRelease->component.setBounds(knobPanel.removeFromLeft(knobWidth));

  auto labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelAttack.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDecay.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelSustain.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelRelease.setBounds(labelPanel.removeFromLeft(knobWidth));
}