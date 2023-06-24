/*
  ==============================================================================

    EnvelopeGrain.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "EnvelopeGrain.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

EnvelopeGrain::EnvelopeGrain(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mParamColour(Utils::GLOBAL_COLOUR),
      mSliderShape(parameters, ParamCommon::Type::GRAIN_SHAPE),
      mSliderTilt(parameters, ParamCommon::Type::GRAIN_TILT),
      mSliderRate(parameters, ParamCommon::Type::GRAIN_RATE),
      mSliderDuration(parameters, ParamCommon::Type::GRAIN_DURATION),
      mPathStroke(2, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::rounded) {
  juce::Colour colour = Utils::GLOBAL_COLOUR;
  mSliderShape.setNumDecimalPlacesToDisplay(2);
  mSliderShape.setRange(0, 1, 0.01);
  addAndMakeVisible(mSliderShape);

  mLabelShape.setText("Shape", juce::dontSendNotification);
  mLabelShape.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelShape.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelShape);

  // Tilt
  mSliderTilt.setNumDecimalPlacesToDisplay(2);
  mSliderTilt.setRange(0, 1, 0.01);
  addAndMakeVisible(mSliderTilt);

  mLabelTilt.setText("Tilt", juce::dontSendNotification);
  mLabelTilt.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelTilt.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelTilt);

  // Rate
  mSliderRate.setNumDecimalPlacesToDisplay(2);
  mSliderRate.setRange(ParamRanges::GRAIN_RATE.start, ParamRanges::GRAIN_RATE.end, 0.01);
  addAndMakeVisible(mSliderRate);

  mLabelRate.setText("Rate", juce::dontSendNotification);
  mLabelRate.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelRate.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRate);

  // Duration
  mSliderDuration.setNumDecimalPlacesToDisplay(2);
  mSliderDuration.setRange(ParamRanges::GRAIN_DURATION.start, ParamRanges::GRAIN_DURATION.end, 0.01);
  mSliderDuration.setTextValueSuffix("s");
  addAndMakeVisible(mSliderDuration);

  mLabelDuration.setText("Duration", juce::dontSendNotification);
  mLabelDuration.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelDuration.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDuration);

  // Sync
  mBtnSync.setButtonText("free");
  mBtnSync.setColour(juce::TextButton::buttonColourId, juce::Colour(GRAIN_SYNC_COLOURS_HEX[0]));
  mBtnSync.setColour(juce::TextButton::buttonOnColourId, juce::Colour(GRAIN_SYNC_COLOURS_HEX[1]));
  mBtnSync.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
  mBtnSync.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
  mBtnSync.onClick = [this]() {
    ParamHelper::setCommonParam(mCurSelectedParams, ParamCommon::Type::GRAIN_SYNC, !mBtnSync.getToggleState());
  };
  addAndMakeVisible(mBtnSync);

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

EnvelopeGrain::~EnvelopeGrain() {
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void EnvelopeGrain::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void EnvelopeGrain::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderShape.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::GRAIN_SHAPE),
                          juce::dontSendNotification);
    mSliderTilt.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::GRAIN_TILT), juce::dontSendNotification);
    mSliderRate.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::GRAIN_RATE), juce::dontSendNotification);
    mSliderDuration.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::GRAIN_DURATION),
                             juce::dontSendNotification);
    mBtnSync.setToggleState(mParameters.getBoolParam(mCurSelectedParams, ParamCommon::Type::GRAIN_SYNC),
                            juce::dontSendNotification);
  }
}

void EnvelopeGrain::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();
  mSliderShape.updateSelectedParams();
  mSliderTilt.updateSelectedParams();
  mSliderRate.updateSelectedParams();
  mSliderDuration.updateSelectedParams();
  mParamHasChanged.store(true);
  repaint();
}

void EnvelopeGrain::paint(juce::Graphics& g) {
  juce::Colour colour = mParamColour;

  // Section title
  g.setColour(Utils::GLOBAL_COLOUR);
  g.fillRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT);
  g.setColour(colour);
  g.drawRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(juce::String(SECTION_TITLE), mTitleRect, juce::Justification::centred);

  float duration = mSliderDuration.getValue();
  float rate = mSliderRate.getValue();

  float envWidth;
  float envOffset;
  if (mBtnSync.getToggleState()) {
    float durDiv = std::pow(2, (int)(duration));
    envWidth = mVizRect.getWidth() / durDiv;
    float rateDiv = std::pow(2, (int)(rate));
    envOffset = envWidth / rateDiv;
  } else {
    envWidth = juce::jmap(duration,
                          mVizRect.getWidth() / MAX_NUM_ENVS, mVizRect.getWidth());
    envOffset = juce::jmap(rate, envWidth * MIN_RATE_RATIO,
                           envWidth * MAX_RATE_RATIO);
  }

  float shapeWidth = envWidth * mSliderShape.getValue() / 2.0f;

  // Draw darker odd numbered envelopes
  float tilt = mSliderTilt.getValue();
  float curXStart = mVizRect.getX() + envOffset;
  juce::Colour envColour = colour.darker(0.5f);
  while (curXStart < mVizRect.getWidth()) {
    juce::Path envPath;
    envPath.startNewSubPath(juce::Point<float>(curXStart, mVizRect.getBottom()));
    envPath.lineTo(juce::Point<float>(limitEnvX(juce::jmax(curXStart, curXStart + (tilt * envWidth) - shapeWidth)), mVizRect.getY()));
    envPath.lineTo(
        juce::Point<float>(limitEnvX(juce::jmin(curXStart + envWidth, curXStart + (tilt * envWidth) + shapeWidth)), mVizRect.getY()));
    envPath.lineTo(juce::Point<float>(limitEnvX(curXStart + envWidth), mVizRect.getBottom()));
    g.setColour(envColour);
    g.strokePath(envPath, mPathStroke);
    curXStart += (envOffset * 2.0f);
  }
  curXStart = mVizRect.getX();
  // Draw brighter even numbered envelopes
  envColour = colour.brighter(0.5f);
  while (curXStart < mVizRect.getWidth()) {
    juce::Path envPath;
    envPath.startNewSubPath(juce::Point<float>(curXStart, mVizRect.getBottom()));
    envPath.lineTo(juce::Point<float>(limitEnvX(juce::jmax(curXStart, curXStart + (tilt * envWidth) - shapeWidth)), mVizRect.getY()));
    envPath.lineTo(
        juce::Point<float>(limitEnvX(juce::jmin(curXStart + envWidth, curXStart + (tilt * envWidth) + shapeWidth)), mVizRect.getY()));
    envPath.lineTo(juce::Point<float>(limitEnvX(curXStart + envWidth), mVizRect.getBottom()));
    g.setColour(envColour);
    g.strokePath(envPath, mPathStroke);
    curXStart += (envOffset * 2.0f);
  }

  g.drawRect(mVizRect.expanded(2).withCentre(mVizRect.getCentre()), 2.0f);
}

void EnvelopeGrain::resized() {
  juce::Rectangle<float> r = getLocalBounds().toFloat();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING).withCentre(getLocalBounds().toFloat().getCentre());

  // Make title rect
  mTitleRect = r.removeFromTop(Utils::TITLE_HEIGHT);

  r.removeFromTop(Utils::PADDING);

  // Place labels
  juce::Rectangle<int> labelPanel = r.removeFromBottom(Utils::LABEL_HEIGHT).toNearestInt();
  int labelWidth = labelPanel.getWidth() / 4;
  mLabelShape.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelTilt.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelRate.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelDuration.setBounds(labelPanel.removeFromLeft(labelWidth));

  // Place sliders
  juce::Rectangle<int> knobPanel = r.removeFromBottom(Utils::KNOB_HEIGHT).toNearestInt();
  int knobWidth = knobPanel.getWidth() / 4;
  mSliderShape.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderTilt.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDuration.setBounds(knobPanel.removeFromLeft(knobWidth));

  r.removeFromBottom(Utils::PADDING);

  // Place button
  juce::Rectangle<int> syncPanel = r.removeFromRight(r.getWidth() * 0.25f).toNearestInt();
  mBtnSync.changeWidthToFitText(Utils::LABEL_HEIGHT);
  mBtnSync.setCentrePosition(syncPanel.getCentre());

  mVizRect = r;
}

float EnvelopeGrain::limitEnvX(float x) { return juce::jmin(x, mVizRect.getRight()); }