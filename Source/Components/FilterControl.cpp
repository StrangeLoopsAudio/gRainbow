/*
  ==============================================================================

    FilterControl.cpp
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#include "FilterControl.h"
#include "Utils/Utils.h"

//==============================================================================
FilterControl::FilterControl(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mSliderCutoff(parameters, ParamCommon::Type::FILT_CUTOFF),
      mSliderResonance(parameters, ParamCommon::Type::FILT_RESONANCE) {
  juce::Colour colour = Utils::GLOBAL_COLOUR;

  mSliderCutoff.setNumDecimalPlacesToDisplay(2);
  mSliderCutoff.setRange(ParamRanges::CUTOFF.start, ParamRanges::CUTOFF.end, 0.01);
  mSliderCutoff.setTextValueSuffix("Hz");
  addAndMakeVisible(mSliderCutoff);

  mLabelCutoff.setText("Cutoff", juce::dontSendNotification);
  mLabelCutoff.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelCutoff.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelCutoff);

  // Resonance
  mSliderResonance.setNumDecimalPlacesToDisplay(2);
  mSliderResonance.setRange(ParamRanges::RESONANCE.start, ParamRanges::RESONANCE.end, 0.01);
  addAndMakeVisible(mSliderResonance);

  mLabelResonance.setText("Resonance", juce::dontSendNotification);
  mLabelResonance.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelResonance.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelResonance);

  for (int i = 0; i < FILTER_TYPE_NAMES.size(); ++i) {
    mFilterType.addItem(FILTER_TYPE_NAMES[i], i + 1);
  }
  mFilterType.onChange = [this]() {
    int type = mFilterType.getSelectedId() - 1;
    ParamHelper::setCommonParam(mCurSelectedParams, ParamCommon::Type::FILT_TYPE, type);
  };
  addAndMakeVisible(mFilterType);

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

FilterControl::~FilterControl() {
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void FilterControl::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void FilterControl::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderCutoff.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::FILT_CUTOFF), juce::dontSendNotification);
    mSliderResonance.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::FILT_RESONANCE),
                              juce::dontSendNotification);
    mFilterType.setSelectedId(mParameters.getChoiceParam(mCurSelectedParams, ParamCommon::Type::FILT_TYPE) + 1,
                              juce::dontSendNotification);
    repaint();
  }
}

void FilterControl::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();
  mSliderCutoff.updateSelectedParams();
  mSliderResonance.updateSelectedParams();
  mParamHasChanged.store(true);
}

void FilterControl::paint(juce::Graphics& g) {
  juce::Colour colour = mParamColour;
  g.setFont(14.0f);

  // Section title
  g.setColour(Utils::GLOBAL_COLOUR);
  g.fillRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT);
  g.setColour(colour);
  g.drawRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(juce::String(SECTION_TITLE), mTitleRect, juce::Justification::centred);

  // Draw selected filter path
  juce::Path mFilterPath;
  int resPadding = mVizRect.getHeight() * 0.3f; // both width and height max size of resonance peak
  float cutoffWidth = mVizRect.getWidth() *
                      ParamRanges::CUTOFF.convertTo0to1(mSliderCutoff.getValue());
  float resHeight =
      mVizRect.getHeight() * 0.3f * ParamRanges::RESONANCE.convertTo0to1(mSliderResonance.getValue());

  juce::Point<float> startPt = mVizRect.getBottomLeft();
  juce::Point<float> midPt1, midPt2, midPt3, midPt4;
  juce::Point<float> endPt = mVizRect.getBottomRight();

  switch (mFilterType.getSelectedId() - 1) {
    case (Utils::FilterType::LOWPASS):
      midPt1 = juce::Point<float>(mVizRect.getX(), mVizRect.getY() + resPadding);
      midPt2 = juce::Point<float>(mVizRect.getX() + cutoffWidth - resPadding / 2, mVizRect.getY() + resPadding);
      midPt3 = juce::Point<float>(mVizRect.getX() + cutoffWidth, mVizRect.getY() + (resPadding - resHeight));
      midPt4 = juce::Point<float>(juce::jmin(mVizRect.getRight(), mVizRect.getX() + cutoffWidth + resPadding / 2), mVizRect.getBottom());
      break;
    case (Utils::FilterType::HIGHPASS):
      cutoffWidth = mVizRect.getWidth() - cutoffWidth; // Reverse for HP
      midPt1 = juce::Point<float>(juce::jmax(mVizRect.getX(), mVizRect.getRight() - cutoffWidth - resPadding / 2), mVizRect.getBottom());
      midPt2 = juce::Point<float>(mVizRect.getRight() - cutoffWidth, mVizRect.getY() + (resPadding - resHeight));
      midPt3 = juce::Point<float>(mVizRect.getRight() - cutoffWidth + resPadding / 2, mVizRect.getY() + resPadding);
      midPt4 = juce::Point<float>(mVizRect.getRight(), mVizRect.getY() + resPadding);
      break;
    case (Utils::FilterType::BANDPASS):
      midPt1 = juce::Point<float>(juce::jmax(mVizRect.getX(), mVizRect.getCentreX() - cutoffWidth / 2 - resPadding / 2),
                                  mVizRect.getBottom());
      midPt2 = juce::Point<float>(mVizRect.getCentreX() - cutoffWidth / 2, mVizRect.getY() + (resPadding - resHeight));
      midPt3 = juce::Point<float>(mVizRect.getCentreX() + cutoffWidth / 2, mVizRect.getY() + (resPadding - resHeight));
      midPt4 = juce::Point<float>(juce::jmin(mVizRect.getRight(), mVizRect.getCentreX() + cutoffWidth / 2 + resPadding / 2),
                                  mVizRect.getBottom());
      break;
    case (Utils::FilterType::NO_FILTER):
      midPt1 = juce::Point<float>(mVizRect.getTopLeft());
      midPt2 = juce::Point<float>(mVizRect.getTopLeft());
      midPt3 = juce::Point<float>(mVizRect.getTopRight());
      midPt4 = juce::Point<float>(mVizRect.getTopRight());
      break;
  }

  // Set gradient
  g.setFillType(juce::ColourGradient(colour, mVizRect.getTopLeft(), colour.withAlpha(0.4f), mVizRect.getBottomLeft(), false));
  mFilterPath.startNewSubPath(startPt);
  mFilterPath.lineTo(midPt1);
  mFilterPath.lineTo(midPt2);
  mFilterPath.lineTo(midPt3);
  mFilterPath.lineTo(midPt4);
  mFilterPath.lineTo(endPt);
  mFilterPath.closeSubPath();
  g.fillPath(mFilterPath);

  // Draw highlight over path
  float highlightWidth = 3.0f;
  g.setColour(colour);
  g.drawLine(juce::Line<float>(startPt, midPt1), highlightWidth);
  g.drawLine(juce::Line<float>(midPt1, midPt2), highlightWidth);
  g.drawLine(juce::Line<float>(midPt2, midPt3), highlightWidth);
  g.drawLine(juce::Line<float>(midPt3, midPt4), highlightWidth);
  g.drawLine(juce::Line<float>(midPt4, endPt), highlightWidth);

  g.setColour(colour);
  g.drawRect(mVizRect.expanded(2).withCentre(mVizRect.getCentre()), 2.0f);
}

void FilterControl::resized() {
  juce::Rectangle<int> r = getLocalBounds();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING).withCentre(getLocalBounds().getCentre());

  // Make title rect
  mTitleRect = r.removeFromTop(Utils::TITLE_HEIGHT).toFloat();

  r.removeFromTop(Utils::PADDING);

  int panelWidth = r.getWidth() / 3.0f;
  juce::Rectangle<int> knobPanel = r.removeFromBottom(Utils::LABEL_HEIGHT + Utils::KNOB_HEIGHT);
  juce::Rectangle<int> panel = knobPanel.removeFromLeft(panelWidth);
  mLabelCutoff.setBounds(panel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderCutoff.setBounds(panel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));

  panel = knobPanel.removeFromRight(panelWidth);
  mLabelResonance.setBounds(panel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderResonance.setBounds(
      panel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));


  mFilterType.setBounds(knobPanel);

  r.removeFromBottom(Utils::PADDING);

  mVizRect = r.toFloat();
}

float FilterControl::filterTypeToCutoff(Utils::FilterType filterType) {
  switch (filterType) {
    case (Utils::FilterType::LOWPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
    case (Utils::FilterType::HIGHPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_HP_CUTOFF_DEFAULT_HZ);
    case (Utils::FilterType::BANDPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_BP_CUTOFF_DEFAULT_HZ);
    case (Utils::FilterType::NO_FILTER):
      jassertfalse;
      return 0.0f;
  }
}
