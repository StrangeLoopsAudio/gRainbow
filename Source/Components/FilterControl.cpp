/*
  ==============================================================================

    FilterControl.cpp
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#include "FilterControl.h"
#include "../Utils.h"

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

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

FilterControl::~FilterControl() { 
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void FilterControl::parameterValueChanged(int idx, float value) { mParamHasChanged.store(true); }

void FilterControl::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderCutoff.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::FILT_CUTOFF), juce::dontSendNotification);
    mSliderResonance.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::FILT_RESONANCE),
                              juce::dontSendNotification);
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
  repaint();
}


void FilterControl::paint(juce::Graphics& g) {
  juce::Colour colour = Utils::GLOBAL_COLOUR;
  g.setFont(14.0f);

  // Section title
  g.setColour(colour);
  g.fillRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT);
  g.setColour(juce::Colours::white);
  g.drawText(juce::String(SECTION_TITLE), mTitleRect, juce::Justification::centred);

  // draw filter type rectangles
  g.setColour(colour);
  g.drawRoundedRectangle(mLowPassRect, Utils::ROUNDED_AMOUNT, 2.0f);
  g.drawRoundedRectangle(mHighPassRect, Utils::ROUNDED_AMOUNT, 2.0f);
  g.drawRoundedRectangle(mBandPassRect, Utils::ROUNDED_AMOUNT, 2.0f);

  // Select current filter type
  switch (mCurHoverFilterType) {
    g.setColour(colour.withAlpha(0.3f));
    case (Utils::FilterType::LOWPASS):
      g.fillRoundedRectangle(mLowPassRect, Utils::ROUNDED_AMOUNT);
      break;
    case (Utils::FilterType::HIGHPASS):
      g.fillRoundedRectangle(mHighPassRect, Utils::ROUNDED_AMOUNT);
      break;
    case (Utils::FilterType::BANDPASS):
      g.fillRoundedRectangle(mBandPassRect, Utils::ROUNDED_AMOUNT);
      break;
  }

  // Label filter types
  g.setColour(Utils::GLOBAL_COLOUR);
  g.drawText(FILTER_TYPE_NAMES[1], mLowPassRect, juce::Justification::centred);
  g.drawText(FILTER_TYPE_NAMES[2], mHighPassRect, juce::Justification::centred);
  g.drawText(FILTER_TYPE_NAMES[3], mBandPassRect, juce::Justification::centred);

  // Draw selected filter path
  juce::Path mFilterPath;
  int resPadding = mVizRect.getHeight() * 0.3f; // both width and height max size of resonance peak
  float cutoffWidth = mVizRect.getWidth() *
                      ParamRanges::CUTOFF.convertTo0to1(P_FLOAT(mCurSelectedParams->common[ParamCommon::Type::FILT_CUTOFF])->get());
  float resHeight =
      mVizRect.getHeight() * 0.3f *
      ParamRanges::RESONANCE.convertTo0to1(P_FLOAT(mCurSelectedParams->common[ParamCommon::Type::FILT_RESONANCE])->get());

  juce::Point<float> startPt = mVizRect.getBottomLeft();
  juce::Point<float> midPt1, midPt2, midPt3, midPt4;
  juce::Point<float> endPt = mVizRect.getBottomRight();

  switch (P_CHOICE(mCurSelectedParams->common[ParamCommon::Type::FILT_TYPE])->getIndex()) {
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

  g.setColour(colour);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), Utils::ROUNDED_AMOUNT, 2.0f);
}

void FilterControl::resized() {
  juce::Rectangle<float> r = getLocalBounds().toFloat();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING).withCentre(getLocalBounds().toFloat().getCentre());

  // Make title rect
  mTitleRect = r.removeFromTop(Utils::TITLE_HEIGHT);

  r.removeFromTop(Utils::PADDING);
  
  // Make button rects
  juce::Rectangle<float> buttonPanel = r.removeFromTop(FILTER_TYPE_BUTTON_HEIGHT); 
  int buttonWidth = buttonPanel.getWidth() / 3;
  mLowPassRect = buttonPanel.removeFromLeft(buttonWidth);
  mHighPassRect = buttonPanel.removeFromLeft(buttonWidth);
  mBandPassRect = buttonPanel;

  r.removeFromTop(Utils::PADDING);

  // Place labels
  juce::Rectangle<int> labelPanel = r.removeFromBottom(Utils::LABEL_HEIGHT).toNearestInt();
  int labelWidth = labelPanel.getWidth() / 2;
  mLabelCutoff.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelResonance.setBounds(labelPanel.removeFromLeft(labelWidth));

  // Place sliders
  juce::Rectangle<int> knobPanel = r.removeFromBottom(Utils::KNOB_HEIGHT).toNearestInt();
  juce::Rectangle<int> knobRect = juce::Rectangle<int>(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT);
  mSliderCutoff.setBounds(knobRect.withCentre(juce::Point<int>(knobPanel.getX() + knobPanel.getWidth() * 0.25f, knobPanel.getCentreY())));
  mSliderResonance.setBounds(
      knobRect.withCentre(juce::Point<int>(knobPanel.getX() + knobPanel.getWidth() * 0.75f, knobPanel.getCentreY())));

  r.removeFromBottom(Utils::PADDING);

  mVizRect = r;
}

void FilterControl::mouseMove(const juce::MouseEvent& event) {
  mCurHoverFilterType = getFilterTypeFromMouse(event);
  repaint();
}
void FilterControl::mouseExit(const juce::MouseEvent& event) {
  mCurHoverFilterType = Utils::NO_FILTER;
  repaint();
}
void FilterControl::mouseUp(const juce::MouseEvent& event) {
  if (event.eventComponent != this) return;
  Utils::FilterType newFilterType = getFilterTypeFromMouse(event);
  if (newFilterType == P_CHOICE(mCurSelectedParams->common[ParamCommon::Type::FILT_TYPE])->getIndex()) {
    ParamHelper::setParam(P_CHOICE(mCurSelectedParams->common[ParamCommon::Type::FILT_TYPE]), Utils::FilterType::NO_FILTER);
  } else {
    ParamHelper::setParam(P_CHOICE(mCurSelectedParams->common[ParamCommon::Type::FILT_TYPE]), newFilterType);
    switch (newFilterType) {
      case (Utils::FilterType::LOWPASS):
        ParamHelper::setParam(P_FLOAT(mCurSelectedParams->common[ParamCommon::Type::FILT_CUTOFF]),
                              ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
        break;
      case (Utils::FilterType::HIGHPASS):
        ParamHelper::setParam(P_FLOAT(mCurSelectedParams->common[ParamCommon::Type::FILT_CUTOFF]),
                              ParamDefaults::FILTER_HP_CUTOFF_DEFAULT_HZ);
        break;
      case (Utils::FilterType::BANDPASS):
        ParamHelper::setParam(P_FLOAT(mCurSelectedParams->common[ParamCommon::Type::FILT_CUTOFF]),
                              ParamDefaults::FILTER_BP_CUTOFF_DEFAULT_HZ);
        break;
    }
    ParamHelper::setParam(P_FLOAT(mCurSelectedParams->common[ParamCommon::Type::FILT_RESONANCE]),
                          ParamDefaults::FILTER_RESONANCE_DEFAULT);
  }
  
  repaint();
}

Utils::FilterType FilterControl::getFilterTypeFromMouse(const juce::MouseEvent& event) {
  juce::Point<float> pos = event.getEventRelativeTo(this).getPosition().toFloat();
  if (mLowPassRect.contains(pos)) {
    return Utils::FilterType::LOWPASS;
  } else if (mHighPassRect.contains(pos)) {
    return Utils::FilterType::HIGHPASS;
  } else if (mBandPassRect.contains(pos)) {
    return Utils::FilterType::BANDPASS;
  } else {
    return Utils::FilterType::NO_FILTER;
  }
}

float FilterControl::filterTypeToCutoff(Utils::FilterType filterType) {
  switch (filterType) {
    case (Utils::FilterType::LOWPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
    case (Utils::FilterType::HIGHPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_HP_CUTOFF_DEFAULT_HZ);
    case (Utils::FilterType::BANDPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_BP_CUTOFF_DEFAULT_HZ);
  }
}
