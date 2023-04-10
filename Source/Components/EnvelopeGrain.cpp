/*
  ==============================================================================

    EnvelopeGrain.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "EnvelopeGrain.h"
#include "../Utils.h"

EnvelopeGrain::EnvelopeGrain(Parameters& parameters)
    : mParameters(parameters),
      mPathStroke(2, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::rounded) {

  juce::Colour colour = juce::Colours::white;
  // Knob params
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;
  // Shape
  mSliderShape.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderShape.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderShape.setRotaryParameters(rotaryParams);
  mSliderShape.setNumDecimalPlacesToDisplay(2);
  mSliderShape.setRange(0, 1, 0.01);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, colour);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, colour.brighter());
  // mSliderShape.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->attack, mSliderShape.getValue()); };
  addAndMakeVisible(mSliderShape);

  mLabelShape.setText("Shape", juce::dontSendNotification);
  mLabelShape.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelShape);

  // Tilt
  mSliderTilt.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderTilt.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderTilt.setRotaryParameters(rotaryParams);
  mSliderTilt.setNumDecimalPlacesToDisplay(2);
  mSliderTilt.setRange(0, 1, 0.01);
  mSliderTilt.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, colour);
  mSliderTilt.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, colour.brighter());
  // mSliderTilt.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->attack, mSliderTilt.getValue()); };
  addAndMakeVisible(mSliderTilt);

  mLabelTilt.setText("Tilt", juce::dontSendNotification);
  mLabelTilt.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelTilt);
  
  // Rate
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRate.setRotaryParameters(rotaryParams);
  mSliderRate.setNumDecimalPlacesToDisplay(2);
  mSliderRate.setRange(ParamRanges::GRAIN_RATE.start, ParamRanges::GRAIN_RATE.end, 0.01);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, colour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, colour.brighter());
  // mSliderRate.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->attack, mSliderRate.getValue()); };
  addAndMakeVisible(mSliderRate);

  mLabelRate.setText("Rate", juce::dontSendNotification);
  mLabelRate.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRate);

  // Duration
  mSliderDuration.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDuration.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDuration.setRotaryParameters(rotaryParams);
  mSliderDuration.setNumDecimalPlacesToDisplay(2);
  mSliderDuration.setRange(ParamRanges::GRAIN_DURATION.start, ParamRanges::GRAIN_DURATION.end, 0.01);
  mSliderDuration.setTextValueSuffix("s");
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, colour);
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, colour.brighter());
  // mSliderDuration.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->attack, mSliderDuration.getValue()); };
  addAndMakeVisible(mSliderDuration);

  mLabelDuration.setText("Duration", juce::dontSendNotification);
  mLabelDuration.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDuration);

  // Sync
  mBtnSync.setButtonText("free");
  mBtnSync.setColour(juce::TextButton::buttonColourId, juce::Colour(GRAIN_SYNC_COLOURS_HEX[0]));
  mBtnSync.setColour(juce::TextButton::buttonOnColourId, juce::Colour(GRAIN_SYNC_COLOURS_HEX[1]));
  mBtnSync.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
  mBtnSync.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
  // mBtnSync.onClick = [this]() { ParamHelper::setParam(getCurrentGenerator()->grainSync, !mBtnSync.getToggleState()); };
  addAndMakeVisible(mBtnSync);

  startTimer(500);
}

void EnvelopeGrain::parameterValueChanged(int idx, float value) { mParamHasChanged.store(true); }

void EnvelopeGrain::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    // TODO: all like this
    // mSliderAttack.setValue(mParameters.global.attack->get(), juce::dontSendNotification);
  }
}

void EnvelopeGrain::paint(juce::Graphics& g) {
  juce::Colour colour = juce::Colours::white;

  // Filter section title
  g.setColour(colour);
  g.fillRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_TITLE), mTitleRect, juce::Justification::centred);

  float envWidth;
  float envOffset;
  if (mParameters.global.grainSync->get()) {
    float durDiv = std::pow(2, (int)(ParamRanges::SYNC_DIV_MAX * mParameters.global.grainDuration->get()));
    envWidth = mVizRect.getWidth() / durDiv;
    float rateDiv = std::pow(2, (int)(ParamRanges::SYNC_DIV_MAX * mParameters.global.grainRate->get()));
    envOffset = envWidth / rateDiv;
  } else {
    envWidth = juce::jmap(mParameters.global.grainDuration->get(), mVizRect.getWidth() / MAX_NUM_ENVS, mVizRect.getWidth());
    envOffset = juce::jmap(mParameters.global.grainRate->get(), envWidth * MIN_RATE_RATIO, envWidth * MAX_RATE_RATIO);
  }

  float shapeWidth = envWidth * mParameters.global.grainShape->get() / 2.0f;

  // Draw darker odd numbered envelopes
  float tilt = mParameters.global.grainTilt->get();
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

  g.setColour(colour);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), Utils::ROUNDED_AMOUNT, 2.0f);
}

void EnvelopeGrain::resized() {
  juce::Rectangle<float> r = getLocalBounds().toFloat();
  // Remove padding
  r = r.reduced(Utils::PADDING * 2, Utils::PADDING * 2).withCentre(getLocalBounds().toFloat().getCentre());

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

  // Place button
  juce::Rectangle<int> syncPanel = r.removeFromRight(r.getWidth() * 0.25f).toNearestInt();
  mBtnSync.changeWidthToFitText(Utils::LABEL_HEIGHT);
  mBtnSync.setCentrePosition(syncPanel.getCentre());

  mVizRect = r;
}

float EnvelopeGrain::limitEnvX(float x) { return juce::jmin(x, mVizRect.getRight()); }