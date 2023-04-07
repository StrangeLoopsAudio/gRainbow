/*
  ==============================================================================

    EnvelopeADSR.cpp
    Created: 12 Jul 2021 12:02:12am
    Author:  brady

  ==============================================================================
*/

#include "EnvelopeADSR.h"
#include "../Utils.h"

//==============================================================================
EnvelopeADSR::EnvelopeADSR(Parameters& parameters): mParameters(parameters) {
  juce::Colour knobColour = juce::Colours::white;
  // Knob params
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;
  // Attack
  mSliderAttack.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderAttack.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderAttack.setRotaryParameters(rotaryParams);
  mSliderAttack.setNumDecimalPlacesToDisplay(2);
  mSliderAttack.setRange(ParamRanges::ATTACK.start, ParamRanges::ATTACK.end, 0.01);
  mSliderAttack.setTextValueSuffix("s");
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour.brighter());
  //mSliderAttack.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->attack, mSliderAttack.getValue()); };
  addAndMakeVisible(mSliderAttack);

  mLabelAttack.setText("Attack", juce::dontSendNotification);
  mLabelAttack.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelAttack);

  // Decay
  mSliderDecay.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDecay.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDecay.setRotaryParameters(rotaryParams);
  mSliderDecay.setNumDecimalPlacesToDisplay(2);
  mSliderDecay.setRange(ParamRanges::DECAY.start, ParamRanges::DECAY.end, 0.01);
  mSliderDecay.setTextValueSuffix("s");
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour.brighter());
  //mSliderDecay.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->decay, mSliderDecay.getValue()); };
  addAndMakeVisible(mSliderDecay);

  mLabelDecay.setText("Decay", juce::dontSendNotification);
  mLabelDecay.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDecay);

  // Sustain
  mSliderSustain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderSustain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderSustain.setRotaryParameters(rotaryParams);
  mSliderSustain.setNumDecimalPlacesToDisplay(2);
  mSliderSustain.setRange(0.0, 1.0, 0.01);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour.brighter());
  //mSliderSustain.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->sustain, mSliderSustain.getValue()); };
  addAndMakeVisible(mSliderSustain);

  mLabelSustain.setText("Sustain", juce::dontSendNotification);
  mLabelSustain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelSustain);

  // Release
  mSliderRelease.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRelease.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRelease.setRotaryParameters(rotaryParams);
  mSliderRelease.setNumDecimalPlacesToDisplay(2);
  mSliderRelease.setRange(ParamRanges::RELEASE.start, ParamRanges::RELEASE.end, 0.01);
  mSliderRelease.setTextValueSuffix("s");
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour.brighter());
  //mSliderRelease.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->release, mSliderRelease.getValue()); };
  addAndMakeVisible(mSliderRelease);

  mLabelRelease.setText("Release", juce::dontSendNotification);
  mLabelRelease.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRelease);

  mParameters.global.addListener(this);

  startTimer(500);
}

EnvelopeADSR::~EnvelopeADSR() { 
  mParameters.global.removeListener(this);
  //mParameters.note.notes[mSelPitchClass]->removeListener(mCurSelectedGenerator, this);
}

void EnvelopeADSR::parameterValueChanged(int idx, float value) { mParamHasChanged.store(true); }

void EnvelopeADSR::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    //TODO: all like this
    //mSliderAttack.setValue(mParameters.global.attack->get(), juce::dontSendNotification);
  }
}

void EnvelopeADSR::selectPitchClass(Utils::PitchClass pitchClass) { 
  // Remove listeners from old generator and note
  //mParameters.note.notes[mSelPitchClass]->removeListener(mCurSelectedGenerator, this);
  // Add listeners to new generator and note
  //mParamsNote.notes[mCurPitchClass]->addListener(mCurSelectedGenerator, this);
  
  mSelPitchClass = pitchClass; 
}

void EnvelopeADSR::paint(juce::Graphics& g) {
  juce::Colour envColour = juce::Colours::white;

  // Amp env section title
  g.setColour(envColour);
  g.fillRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_TITLE), mTitleRect, juce::Justification::centred);

  // TODO: include other non-global values as well
  float attack = ParamRanges::ATTACK.convertTo0to1(mParameters.global.attack->get());
  float decay = ParamRanges::DECAY.convertTo0to1(mParameters.global.decay->get());
  float sustain = mParameters.global.sustain->get();
  float release = ParamRanges::RELEASE.convertTo0to1(mParameters.global.release->get());

  // Draw ADSR path
  g.setFillType(juce::ColourGradient(envColour, mVizRect.getTopLeft(), envColour.withAlpha(0.4f), mVizRect.getBottomLeft(), false));

  juce::Path adsrPath;
  juce::Point<float> startPt = mVizRect.getBottomLeft().translated(1, -1);
  juce::Point<float> attackPt = startPt.translated(attack * mVizRect.getWidth() * 0.375f, -mVizRect.getHeight() + 2);
  juce::Point<float> decayPt = attackPt.translated(decay * mVizRect.getWidth() * 0.375f, (1.0f - sustain) * (mVizRect.getHeight() - 1));
  juce::Point<float> sustainPt = mVizRect.getBottomLeft().translated(mVizRect.getWidth() * 0.75f, -sustain * (mVizRect.getHeight() - 1));
  juce::Point<float> endPt = sustainPt.translated(release * mVizRect.getWidth() * 0.25f, 0.0f).withY(mVizRect.getBottom() - 1);

  adsrPath.startNewSubPath(startPt);
  adsrPath.lineTo(attackPt);
  adsrPath.lineTo(decayPt);
  adsrPath.lineTo(sustainPt);
  adsrPath.lineTo(endPt);
  adsrPath.closeSubPath();
  g.fillPath(adsrPath);

  // Draw highlight lines on top of each segment
  float highlightWidth = 3.0f;
  g.setColour(envColour);
  g.drawLine(juce::Line<float>(startPt, attackPt), highlightWidth);
  g.drawLine(juce::Line<float>(attackPt, decayPt), highlightWidth);
  g.drawLine(juce::Line<float>(decayPt, sustainPt), highlightWidth);
  g.drawLine(juce::Line<float>(sustainPt.translated(0, -1), endPt), highlightWidth);

  g.drawRect(mVizRect, 2.0f);

  g.setColour(envColour);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), Utils::ROUNDED_AMOUNT, 2.0f);
}

void EnvelopeADSR::resized() {
  juce::Rectangle<int> r = getLocalBounds();
  // Remove padding
  r = r.reduced(Utils::PADDING * 2, Utils::PADDING * 2).withCentre(getLocalBounds().getCentre());
  // Make title rect
  mTitleRect = r.removeFromTop(Utils::TITLE_HEIGHT).toFloat();

  r.removeFromTop(Utils::PADDING);

  // Place labels
  juce::Rectangle<int> labelPanel = r.removeFromBottom(Utils::LABEL_HEIGHT);
  int labelWidth = labelPanel.getWidth() / 4;
  mLabelAttack.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelDecay.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelSustain.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelRelease.setBounds(labelPanel.removeFromLeft(labelWidth));

  // Place sliders
  juce::Rectangle<int> knobPanel = r.removeFromBottom(Utils::KNOB_HEIGHT);
  int knobWidth = knobPanel.getWidth() / 4;
  mSliderAttack.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDecay.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderSustain.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRelease.setBounds(knobPanel.removeFromLeft(knobWidth));

  mVizRect = r.toFloat();
}