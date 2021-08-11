/*
  ==============================================================================

    GeneratorBox.cpp
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#include "GeneratorBox.h"
#include <JuceHeader.h>

//==============================================================================
GeneratorBox::GeneratorBox(NoteParams& noteParams) : mNoteParams(noteParams) {
  mPositionChanger.onPositionChanged = [this](bool isRight) {
    if (onPositionChanged != nullptr) {
      onPositionChanged(mCurSelectedTab, isRight);
    }
  };
  mPositionChanger.onSoloChanged = [this](bool isSolo) {
    mNoteParams.notes[mCurPitchClass]
        ->generators[mCurSelectedTab]
        ->solo->setValueNotifyingHost(isSolo);
    refreshState();
  };
  addAndMakeVisible(mPositionChanger);

  /* Knob params */
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi; 
  rotaryParams.stopAtEnd = true; 

  bool enabled = mNoteParams.notes[mCurPitchClass]
                     ->generators[mCurSelectedTab]
                     ->enable->get();
  mLabelShape.setEnabled(enabled);
  mLabelTilt.setEnabled(enabled);
  mLabelRate.setEnabled(enabled);
  mLabelDuration.setEnabled(enabled);
  mLabelGain.setEnabled(enabled);

  /* Adjust pitch */
  mSliderPitch.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderPitch.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderPitch.setRotaryParameters(rotaryParams);
  mSliderPitch.setRange(0.0, 1.0, 0.01);
  mSliderPitch.onValueChange = [this] {
    // TODO: set param
  };
  addAndMakeVisible(mSliderPitch);

  mLabelPitch.setText("Pitch Adjust", juce::dontSendNotification);
  mLabelPitch.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPitch);

  /* Adjust position */
  mSliderPosition.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderPosition.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderPosition.setRotaryParameters(rotaryParams);
  mSliderPosition.setRange(0.0, 1.0, 0.01);
  mSliderPosition.onValueChange = [this] {
    // TODO: set param
  };
  addAndMakeVisible(mSliderPosition);

  mLabelPosition.setText("Position Adjust", juce::dontSendNotification);
  mLabelPosition.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPosition);

  /* Shape */
  mSliderShape.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderShape.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderShape.setRotaryParameters(rotaryParams);
  mSliderShape.setRange(0.0, 1.0, 0.01);
  mSliderShape.onValueChange = [this] {
    mEnvelopeGrain.setShape(mSliderShape.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderShape);

  mLabelShape.setText("Shape", juce::dontSendNotification);
  mLabelShape.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelShape);

  /* Tilt */
  mSliderTilt.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderTilt.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderTilt.setRotaryParameters(rotaryParams);
  mSliderTilt.setRange(0.0, 1.0, 0.01);
  mSliderTilt.onValueChange = [this] {
    mEnvelopeGrain.setTilt(mSliderTilt.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderTilt);

  mLabelTilt.setText("Tilt", juce::dontSendNotification);
  mLabelTilt.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelTilt);

  /* Rate */
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRate.setRotaryParameters(rotaryParams);
  mSliderRate.setRange(0.0, 1.0, 0.01);
  mSliderRate.onValueChange = [this] {
    mEnvelopeGrain.setRate(mSliderRate.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderRate);

  mLabelRate.setText("Rate", juce::dontSendNotification);
  mLabelRate.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRate);

  /* Duration */
  mSliderDuration.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDuration.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDuration.setRotaryParameters(rotaryParams);
  mSliderDuration.setRange(0.0, 1.0, 0.01);
  mSliderDuration.onValueChange = [this] {
    mEnvelopeGrain.setDuration(mSliderDuration.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderDuration);

  mLabelDuration.setText("Duration", juce::dontSendNotification);
  mLabelDuration.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDuration);

  /* Gain */
  mSliderGain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderGain.setRotaryParameters(rotaryParams);
  mSliderGain.setRange(0.0, 1.0, 0.01);
  mSliderGain.onValueChange = [this] {
    mEnvelopeGrain.setGain(mSliderGain.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderGain);

  mLabelGain.setText("Gain", juce::dontSendNotification);
  mLabelGain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelGain);

  /* Grain envelope viz */
  addAndMakeVisible(mEnvelopeGrain);

  /* Amp envelope viz */
  addAndMakeVisible(mEnvelopeAmp);

  /* Attack */
  mSliderAttack.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderAttack.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderAttack.setRotaryParameters(rotaryParams);
  mSliderAttack.setRange(0.0, 1.0, 0.01);
  mSliderAttack.onValueChange = [this] {
    mEnvelopeAmp.setAttack(mSliderAttack.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderAttack);

  mLabelAttack.setText("Attack", juce::dontSendNotification);
  mLabelAttack.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelAttack);

  /* Decay */
  mSliderDecay.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDecay.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDecay.setRotaryParameters(rotaryParams);
  mSliderDecay.setRange(0.0, 1.0, 0.01);
  mSliderDecay.onValueChange = [this] {
    mEnvelopeAmp.setDecay(mSliderDecay.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderDecay);

  mLabelDecay.setText("Decay", juce::dontSendNotification);
  mLabelDecay.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDecay);

  /* Sustain */
  mSliderSustain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderSustain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderSustain.setRotaryParameters(rotaryParams);
  mSliderSustain.setRange(0.0, 1.0, 0.01);
  mSliderSustain.onValueChange = [this] {
    mEnvelopeAmp.setSustain(mSliderSustain.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderSustain);

  mLabelSustain.setText("Sustain", juce::dontSendNotification);
  mLabelSustain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelSustain);

  /* Release */
  mSliderRelease.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRelease.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRelease.setRotaryParameters(rotaryParams);
  mSliderRelease.setRange(0.0, 1.0, 0.01);
  mSliderRelease.onValueChange = [this] {
    mEnvelopeAmp.setRelease(mSliderRelease.getValue());
    // TODO: set param
  };
  addAndMakeVisible(mSliderRelease);

  mLabelRelease.setText("Release", juce::dontSendNotification);
  mLabelRelease.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRelease);
}

GeneratorBox::~GeneratorBox() {}

void GeneratorBox::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  GeneratorParams* gen =
      mNoteParams.notes[mCurPitchClass]->generators[mCurSelectedTab].get();
  bool borderLit = Utils::shouldPlay(gen->enable->get(), gen->solo->get(), gen->waiting->get());
  juce::Colour fillCol = borderLit
                             ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                             : juce::Colours::darkgrey;
  g.setColour(fillCol);

  // Lines to connect to tab
  float tabWidth = getWidth() / Utils::GeneratorColour::NUM_GEN;
  if (mColour > 0) {
    g.drawLine(1.0f, 0.0f, mColour * tabWidth + 2.0f, 0.0f,
               3.0f);
  }
  if (mColour < Utils::GeneratorColour::NUM_GEN - 1) {
    g.drawLine((mColour + 1) * tabWidth - 2.0f, 0.0f, getWidth() - 1.0f, 0.0f,
               3.0f);
  }

  // Adjustments section title
  juce::Rectangle<int> adjustTitleRect = juce::Rectangle<int>(
      0, PADDING_SIZE / 2, getWidth(), SECTION_TITLE_HEIGHT).reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(adjustTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_ADJUST_TITLE), adjustTitleRect,
             juce::Justification::centred);

  // Amp env section title
  juce::Rectangle<int> ampEnvTitleRect = juce::Rectangle<int>(
      0, mEnvelopeAmp.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f),
          getWidth(), SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(ampEnvTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_AMP_ENV_TITLE), ampEnvTitleRect,
             juce::Justification::centred);

  // Grain env section title
  juce::Rectangle<int> grainEnvTitleRect = juce::Rectangle<int>(
      0, mEnvelopeGrain.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f),
          getWidth(), SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(grainEnvTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_GRAIN_ENV_TITLE), grainEnvTitleRect,
             juce::Justification::centred);

  g.setColour(fillCol);
  g.drawRoundedRectangle(getLocalBounds().withHeight(getHeight() + 10).translated(0, -11).toFloat().reduced(1.0f), 10.0f, 2.0f);
}

void GeneratorBox::resized() {
  auto r = getLocalBounds();
  // Add insets
  r.removeFromTop(PADDING_SIZE);
  r.removeFromLeft(PADDING_SIZE);
  r.removeFromRight(PADDING_SIZE);
  r.removeFromBottom(PADDING_SIZE);

  // Enable and solo buttons
  r.removeFromTop(SECTION_TITLE_HEIGHT);
  auto adjustmentsPanel = r.removeFromTop(ADJUSTMENT_HEIGHT + LABEL_HEIGHT);
  int adjustKnobWidth = adjustmentsPanel.getWidth() / 3;
  auto pitchKnobPanel = adjustmentsPanel.removeFromLeft(adjustKnobWidth);
  mLabelPitch.setBounds(pitchKnobPanel.removeFromBottom(LABEL_HEIGHT));
  mSliderPitch.setBounds(pitchKnobPanel.withSizeKeepingCentre(
      pitchKnobPanel.getHeight() * 2, pitchKnobPanel.getHeight()));
  auto posKnobPanel = adjustmentsPanel.removeFromRight(adjustKnobWidth);
  mLabelPosition.setBounds(posKnobPanel.removeFromBottom(LABEL_HEIGHT));
  mSliderPosition.setBounds(posKnobPanel.withSizeKeepingCentre(
      posKnobPanel.getHeight() * 2, posKnobPanel.getHeight()));
  mPositionChanger.setBounds(adjustmentsPanel.withSizeKeepingCentre(
      adjustmentsPanel.getWidth(), adjustmentsPanel.getHeight() / 1.5f));

  r.removeFromTop(PADDING_SIZE);

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

  r.removeFromTop(PADDING_SIZE);

  // Grain envelopes
  r.removeFromTop(SECTION_TITLE_HEIGHT);
  mEnvelopeGrain.setBounds(r.removeFromTop(ENVELOPE_HEIGHT));
  r.removeFromTop(PADDING_SIZE);

  // Grain env knobs
  knobWidth = r.getWidth() / NUM_GRAIN_ENV_PARAMS;
  knobPanel = r.removeFromTop(knobWidth / 2);
  mSliderShape.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderTilt.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDuration.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderGain.setBounds(knobPanel.removeFromLeft(knobWidth));

  labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelShape.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelTilt.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelRate.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDuration.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelGain.setBounds(labelPanel.removeFromLeft(knobWidth));
}

void GeneratorBox::refreshState() {
  GeneratorParams* gen =
      mNoteParams.notes[mCurPitchClass]->generators[mCurSelectedTab].get();
  mPositionChanger.setSolo(gen->solo->get());

  mColour = mCurSelectedTab;
  juce::Colour newColour =
      juce::Colour(Utils::POSITION_COLOURS[mCurSelectedTab]);
  mPositionChanger.setColour(newColour);
  mEnvelopeGrain.setColour(newColour);
  mEnvelopeAmp.setColour(newColour);

  bool componentsLit = Utils::shouldPlay(gen->enable->get(), gen->solo->get(), gen->waiting->get());
  juce::Colour knobColour = componentsLit
                                ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                                : juce::Colours::darkgrey;
  mSliderPitch.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                          knobColour);
  mSliderPitch.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mLabelPitch.setEnabled(componentsLit);
  mSliderPosition.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                          knobColour);
  mSliderPosition.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mLabelPosition.setEnabled(componentsLit);
  mPositionChanger.setActive(componentsLit);

  mEnvelopeAmp.setActive(componentsLit);
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                          knobColour);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         knobColour);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                           knobColour);
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                           knobColour);
  mSliderAttack.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         componentsLit ? knobColour.brighter().brighter()
                                       : juce::Colours::darkgrey);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                           componentsLit
                               ? knobColour.brighter().brighter().brighter()
                               : juce::Colours::darkgrey);
  mSliderRelease.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit ? knobColour.brighter().brighter().brighter().brighter()
                    : juce::Colours::darkgrey);
  mLabelAttack.setEnabled(componentsLit);
  mLabelDecay.setEnabled(componentsLit);
  mLabelSustain.setEnabled(componentsLit);
  mLabelRelease.setEnabled(componentsLit);

  mEnvelopeGrain.setActive(componentsLit);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         knobColour);
  mSliderTilt.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         knobColour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                            knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         knobColour);
  mSliderTilt.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         knobColour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mSliderDuration.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mLabelShape.setEnabled(componentsLit);
  mLabelTilt.setEnabled(componentsLit);
  mLabelRate.setEnabled(componentsLit);
  mLabelDuration.setEnabled(componentsLit);
  mLabelGain.setEnabled(componentsLit);
  repaint();
}