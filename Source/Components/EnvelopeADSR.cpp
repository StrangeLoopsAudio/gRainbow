/*
  ==============================================================================

    EnvelopeADSR.cpp
    Created: 12 Jul 2021 12:02:12am
    Author:  brady

  ==============================================================================
*/

#include "EnvelopeADSR.h"
#include "Utils/Utils.h"

//==============================================================================
EnvelopeADSR::EnvelopeADSR(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mSliderAttack(parameters, ParamCommon::Type::ATTACK),
      mSliderDecay(parameters, ParamCommon::Type::DECAY),
      mSliderSustain(parameters, ParamCommon::Type::SUSTAIN),
      mSliderRelease(parameters, ParamCommon::Type::RELEASE)
{
  juce::Colour knobColour = Utils::GLOBAL_COLOUR;
  // Attack
  mSliderAttack.setNumDecimalPlacesToDisplay(2);
  mSliderAttack.setRange(ParamRanges::ATTACK.start, ParamRanges::ATTACK.end, 0.01);
  mSliderAttack.setTextValueSuffix("s");
  mSliderAttack.setPopupDisplayEnabled(true, true, this);
  addAndMakeVisible(mSliderAttack);

  mLabelAttack.setText("attack", juce::dontSendNotification);
  mLabelAttack.setColour(juce::Label::ColourIds::textColourId, knobColour);
  mLabelAttack.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelAttack);

  // Decay
  mSliderDecay.setNumDecimalPlacesToDisplay(2);
  mSliderDecay.setRange(ParamRanges::DECAY.start, ParamRanges::DECAY.end, 0.01);
  mSliderDecay.setTextValueSuffix("s");
  mSliderDecay.setPopupDisplayEnabled(true, true, this);
  addAndMakeVisible(mSliderDecay);

  mLabelDecay.setText("decay", juce::dontSendNotification);
  mLabelDecay.setColour(juce::Label::ColourIds::textColourId, knobColour);
  mLabelDecay.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDecay);

  // Sustain
  mSliderSustain.setNumDecimalPlacesToDisplay(2);
  mSliderSustain.setRange(0.0, 1.0, 0.01);
  mSliderSustain.setPopupDisplayEnabled(true, true, this);
  addAndMakeVisible(mSliderSustain);

  mLabelSustain.setText("sustain", juce::dontSendNotification);
  mLabelSustain.setColour(juce::Label::ColourIds::textColourId, knobColour);
  mLabelSustain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelSustain);

  // Release
  mSliderRelease.setNumDecimalPlacesToDisplay(2);
  mSliderRelease.setRange(ParamRanges::RELEASE.start, ParamRanges::RELEASE.end, 0.01);
  mSliderRelease.setTextValueSuffix("s");
  mSliderRelease.setPopupDisplayEnabled(true, true, this);
  addAndMakeVisible(mSliderRelease);

  mLabelRelease.setText("release", juce::dontSendNotification);
  mLabelRelease.setColour(juce::Label::ColourIds::textColourId, knobColour);
  mLabelRelease.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRelease);

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

EnvelopeADSR::~EnvelopeADSR() {
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void EnvelopeADSR::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void EnvelopeADSR::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderAttack.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::ATTACK), juce::dontSendNotification);
    mSliderDecay.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::DECAY), juce::dontSendNotification);
    mSliderSustain.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::SUSTAIN), juce::dontSendNotification);
    mSliderRelease.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::RELEASE), juce::dontSendNotification);
  }
}

void EnvelopeADSR::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();
  mSliderAttack.updateSelectedParams();
  mSliderDecay.updateSelectedParams();
  mSliderSustain.updateSelectedParams();
  mSliderRelease.updateSelectedParams();
  mParamHasChanged.store(true);
  repaint();
}

void EnvelopeADSR::paint(juce::Graphics& g) {
  juce::Colour colour = mParamColour;

  // Section title
  g.setColour(Utils::GLOBAL_COLOUR);
  g.fillRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT);
  g.setColour(colour);
  g.drawRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(juce::String(SECTION_TITLE), mTitleRect, juce::Justification::centred);

  // TODO: include other non-global values as well
  float attack = ParamRanges::ATTACK.convertTo0to1(mSliderAttack.getValue());
  float decay = ParamRanges::DECAY.convertTo0to1(mSliderDecay.getValue());
  float sustain = ParamRanges::SUSTAIN.convertTo0to1(mSliderSustain.getValue());
  float release = ParamRanges::RELEASE.convertTo0to1(mSliderRelease.getValue());

  // Draw ADSR path
  g.setFillType(juce::ColourGradient(colour, mVizRect.getTopLeft(), colour.withAlpha(0.4f), mVizRect.getBottomLeft(), false));

  juce::Path adsrPath;
  juce::Point<float> startPt = mVizRect.getBottomLeft();
  juce::Point<float> attackPt = startPt.translated(attack * mVizRect.getWidth() * 0.375f, -mVizRect.getHeight());
  juce::Point<float> decayPt = attackPt.translated(decay * mVizRect.getWidth() * 0.375f, (1.0f - sustain) * (mVizRect.getHeight()));
  juce::Point<float> sustainPt = mVizRect.getBottomLeft().translated(mVizRect.getWidth() * 0.75f, -sustain * (mVizRect.getHeight()));
  juce::Point<float> endPt = sustainPt.translated(release * mVizRect.getWidth() * 0.25f, 0.0f).withY(mVizRect.getBottom());

  adsrPath.startNewSubPath(startPt);
  adsrPath.lineTo(attackPt);
  adsrPath.lineTo(decayPt);
  adsrPath.lineTo(sustainPt);
  adsrPath.lineTo(endPt);
  adsrPath.closeSubPath();
  g.fillPath(adsrPath);

  // Draw highlight lines on top of each segment
  float highlightWidth = 3.0f;
  g.setColour(colour);
  g.drawLine(juce::Line<float>(startPt, attackPt), highlightWidth);
  g.drawLine(juce::Line<float>(attackPt, decayPt), highlightWidth);
  g.drawLine(juce::Line<float>(decayPt, sustainPt), highlightWidth);
  g.drawLine(juce::Line<float>(sustainPt.translated(0, -1), endPt), highlightWidth);

  g.drawRect(mVizRect.expanded(2).withCentre(mVizRect.getCentre()), 2.0f);
}

void EnvelopeADSR::resized() {
  juce::Rectangle<int> r = getLocalBounds();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING).withCentre(getLocalBounds().getCentre());
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

  r.removeFromBottom(Utils::PADDING);

  mVizRect = r.toFloat();
}
