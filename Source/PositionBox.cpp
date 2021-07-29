/*
  ==============================================================================

    PositionBox.cpp
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#include "PositionBox.h"
#include "Utils.h"
#include <JuceHeader.h>

//==============================================================================
PositionBox::PositionBox() {

  mPositionChanger.onPositionChanged = [this](bool isRight) {
    if (onPositionChanged != nullptr) {
      onPositionChanged(isRight);
    }
  };
  addAndMakeVisible(mPositionChanger);

  mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId,
                        juce::Colours::blue);
  mBtnSolo.onClick = [this] {
    setState(mBtnSolo.getToggleState() ? BoxState::SOLO : BoxState::READY);
    parameterChanged(GranularSynth::ParameterType::SOLO,
                     mBtnSolo.getToggleState());
  };
  addAndMakeVisible(mBtnSolo);

  /* Knob params */
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi; 
  rotaryParams.stopAtEnd = true; 

  mLabelShape.setEnabled(mState == BoxState::READY);
  mLabelRate.setEnabled(mState == BoxState::READY);
  mLabelDuration.setEnabled(mState == BoxState::READY);
  mLabelGain.setEnabled(mState == BoxState::READY);

  /* Adjust pitch */
  mSliderPitch.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderPitch.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderPitch.setRotaryParameters(rotaryParams);
  mSliderPitch.setRange(0.0, 1.0, 0.01);
  mSliderPitch.onValueChange = [this] {
    parameterChanged(GranularSynth::ParameterType::PITCH_ADJUST,
                     mSliderPitch.getValue());
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
    parameterChanged(GranularSynth::ParameterType::POSITION_ADJUST,
                     mSliderPosition.getValue());
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
    parameterChanged(GranularSynth::ParameterType::SHAPE,
                     mSliderShape.getValue());
  };
  addAndMakeVisible(mSliderShape);

  mLabelShape.setText("Shape", juce::dontSendNotification);
  mLabelShape.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelShape);

  /* Rate */
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRate.setRotaryParameters(rotaryParams);
  mSliderRate.setRange(0.0, 1.0, 0.01);
  mSliderRate.onValueChange = [this] {
    mEnvelopeGrain.setRate(mSliderRate.getValue());
    parameterChanged(GranularSynth::ParameterType::RATE,
                     mSliderRate.getValue());
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
    parameterChanged(GranularSynth::ParameterType::DURATION,
                     mSliderDuration.getValue());
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
    parameterChanged(GranularSynth::ParameterType::GAIN,
                     mSliderGain.getValue());
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
    parameterChanged(GranularSynth::ParameterType::ATTACK,
                     mSliderAttack.getValue());
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
    parameterChanged(GranularSynth::ParameterType::DECAY,
                     mSliderDecay.getValue());
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
    parameterChanged(GranularSynth::ParameterType::SUSTAIN,
                     mSliderSustain.getValue());
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
    parameterChanged(GranularSynth::ParameterType::RELEASE,
                     mSliderRelease.getValue());
  };
  addAndMakeVisible(mSliderRelease);

  mLabelRelease.setText("Release", juce::dontSendNotification);
  mLabelRelease.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRelease);
}

PositionBox::~PositionBox() {}

void PositionBox::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  bool borderLit = (mIsActive || mState == BoxState::SOLO);
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

void PositionBox::resized() {
  auto r = getLocalBounds();
  // Add insets
  r.removeFromTop(PADDING_SIZE);
  r.removeFromLeft(PADDING_SIZE);
  r.removeFromRight(PADDING_SIZE);
  r.removeFromBottom(PADDING_SIZE);

  // Enable and solo buttons
  r.removeFromTop(SECTION_TITLE_HEIGHT);
  auto adjustmentsPanel = r.removeFromTop(ADJUSTMENT_HEIGHT + LABEL_HEIGHT);
  mBtnSolo.setBounds(
      adjustmentsPanel.withLeft(adjustmentsPanel.getRight() - TOGGLE_SIZE)
          .withHeight(TOGGLE_SIZE));
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
      adjustmentsPanel.getWidth(), adjustmentsPanel.getHeight() / 2));

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
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDuration.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderGain.setBounds(knobPanel.removeFromLeft(knobWidth));

  labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelShape.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelRate.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDuration.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelGain.setBounds(labelPanel.removeFromLeft(knobWidth));
}

void PositionBox::setActive(bool isActive) {
  mIsActive = isActive;
  setState(mState);
}

void PositionBox::setPositionNumber(int positionNumber) {
  mPositionChanger.setPositionNumber(positionNumber);
}

void PositionBox::setParams(Utils::GeneratorParams params) {
  setActive(params.isActive);
  mPositionChanger.setPositionNumber(params.position);
  mSliderPitch.setValue(params.pitchAdjust, juce::dontSendNotification);
  mSliderPosition.setValue(params.posAdjust, juce::dontSendNotification);
  mSliderShape.setValue(params.shape, juce::dontSendNotification);
  mEnvelopeGrain.setShape(params.shape);
  mSliderRate.setValue(params.rate, juce::dontSendNotification);
  mEnvelopeGrain.setRate(params.rate);
  mSliderDuration.setValue(params.duration, juce::dontSendNotification);
  mEnvelopeGrain.setDuration(params.duration);
  mSliderGain.setValue(params.gain, juce::dontSendNotification);
  mEnvelopeGrain.setGain(params.gain);
  mSliderAttack.setValue(params.attack, juce::dontSendNotification);
  mEnvelopeAmp.setAttack(params.attack);
  mSliderDecay.setValue(params.decay, juce::dontSendNotification);
  mEnvelopeAmp.setDecay(params.decay);
  mSliderSustain.setValue(params.sustain, juce::dontSendNotification);
  mEnvelopeAmp.setSustain(params.sustain);
  mSliderRelease.setValue(params.release, juce::dontSendNotification);
  mEnvelopeAmp.setRelease(params.release);
}

void PositionBox::setNumPositions(int numPositions) {
  mPositionChanger.setNumPositions(numPositions);
}

void PositionBox::setState(BoxState state) {
  mState = state;

  if (state == BoxState::SOLO_WAIT) {
    mBtnSolo.setToggleState(false, juce::dontSendNotification);
  }

  juce::Colour soloColour = (state != BoxState::SOLO_WAIT)
                                   ? juce::Colours::blue
                                   : juce::Colours::darkgrey;
  mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId, soloColour);

  bool componentsLit = (mIsActive && state == BoxState::READY ||
                        state == BoxState::SOLO);
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
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                            knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         knobColour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mSliderDuration.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mLabelRate.setEnabled(componentsLit);
  mLabelDuration.setEnabled(componentsLit);
  mLabelGain.setEnabled(componentsLit);
  repaint();
}

Utils::GeneratorParams PositionBox::getParams() {
  bool canPlay =
      mState != PositionBox::BoxState::SOLO_WAIT;
  bool shouldPlay = mIsActive ||
      mState == PositionBox::BoxState::SOLO;
  return Utils::GeneratorParams(
      canPlay && shouldPlay,
      mPositionChanger.getPositionNumber(),
      mSliderPitch.getValue(),
      mSliderPosition.getValue(), mSliderShape.getValue(),
      mSliderRate.getValue(), mSliderDuration.getValue(),
      mSliderGain.getValue(), mSliderAttack.getValue(), mSliderDecay.getValue(),
      mSliderSustain.getValue(), mSliderRelease.getValue());
}

void PositionBox::setColour(Utils::GeneratorColour colour) {
  mColour = colour;
  juce::Colour newColour = juce::Colour(Utils::POSITION_COLOURS[colour]);
  if (mState == BoxState::READY) {
    mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId,
                       juce::Colours::blue);
  }
  mPositionChanger.setColour(newColour);
  mEnvelopeGrain.setColour(newColour);
  mEnvelopeAmp.setColour(colour);
  repaint();
}

void PositionBox::parameterChanged(GranularSynth::ParameterType type,
                                   float value) {
  if (onParameterChanged != nullptr) {
    onParameterChanged(mColour, type, value);
  }
}