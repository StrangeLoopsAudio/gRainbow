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
mCurSelectedParams(parameters.getSelectedParams()),
mParamColour(Utils::GLOBAL_COLOUR),
mSliderShape(parameters, ParamCommon::Type::GRAIN_SHAPE),
mSliderTilt(parameters, ParamCommon::Type::GRAIN_TILT),
mSliderRate(parameters, ParamCommon::Type::GRAIN_RATE, false),
mSliderDuration(parameters, ParamCommon::Type::GRAIN_DURATION, true),
mBtnSync(parameters, ParamCommon::Type::GRAIN_SYNC),
mPathStroke(2, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::rounded) {
  juce::Colour colour = Utils::GLOBAL_COLOUR;

  // Default slider settings
  std::vector<std::reference_wrapper<CommonSlider>> sliders = { mSliderShape, mSliderTilt, mSliderRate, mSliderDuration };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(slider.get());
  }
  
  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelShape, mLabelTilt, mLabelRate, mLabelDuration };
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
    label.get().setJustificationType(juce::Justification::centredTop);
    label.get().setFont(Utils::getFont());
    addAndMakeVisible(label.get());
  }
  
  // Default button settings
  std::vector<std::reference_wrapper<juce::Button>> buttons = { mBtnSync };
  for (auto& btn : buttons) {
    btn.get().setColour(juce::ToggleButton::ColourIds::tickColourId, Utils::GLOBAL_COLOUR);
    addAndMakeVisible(btn.get());
  }

  // Shape
  mSliderShape.setRange(ParamRanges::GRAIN_SHAPE.start, ParamRanges::GRAIN_SHAPE.end, 0.01);
  mLabelShape.setText("shape", juce::dontSendNotification);

  // Tilt
  mSliderTilt.setRange(ParamRanges::GRAIN_TILT.start, ParamRanges::GRAIN_TILT.end, 0.01);
  mLabelTilt.setText("tilt", juce::dontSendNotification);

  // Rate
  mSliderRate.setRange(ParamRanges::GRAIN_RATE.start, ParamRanges::GRAIN_RATE.end, 0.01);
  mSliderRate.setSuffix("g/s");
  mLabelRate.setText("rate", juce::dontSendNotification);

  // Duration
  mSliderDuration.setRange(ParamRanges::GRAIN_DURATION.start, ParamRanges::GRAIN_DURATION.end, 0.01);
  mSliderDuration.setSuffix("s");
  mLabelDuration.setText("duration", juce::dontSendNotification);

  // Sync
  mBtnSync.setButtonText("hz");

  mParameters.addListener(this);
  mCurSelectedParams->addListener(this);
  selectedCommonParamsChanged(mCurSelectedParams);

  startTimer(Utils::UI_REFRESH_INTERVAL);
}

EnvelopeGrain::~EnvelopeGrain() {
  mParameters.removeListener(this);
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
    mBtnSync.setButtonText(mBtnSync.getToggleState() ? "sync" : "hz");
    mSliderRate.setSync(mBtnSync.getToggleState());
    mSliderRate.setRange(mSliderRate.getRange(), mBtnSync.getToggleState() ? mSliderRate.getRange().getLength() / (ParamRanges::SYNC_DIV_MAX) : 0.01);
    mSliderDuration.setSync(mBtnSync.getToggleState());
    mSliderDuration.setRange(mSliderDuration.getRange(), mBtnSync.getToggleState() ? mSliderDuration.getRange().getLength() / (ParamRanges::SYNC_DIV_MAX) : 0.01);
  }
}

void EnvelopeGrain::selectedCommonParamsChanged(ParamCommon* newParams) {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = newParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();
  mParamHasChanged.store(true);
  repaint();
}

void EnvelopeGrain::paint(juce::Graphics& g) {
  juce::Colour colour = mParamColour;
  
  // Panel rectangle
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
  
  // Visualization rect
  g.setColour(Utils::BG_COLOUR);
  g.fillRect(mVizRect);

  float durSec, rateSec;
  if (mBtnSync.getToggleState()) {
    float div = std::pow(2, juce::roundToInt(ParamRanges::SYNC_DIV_MAX * (1.0f - ParamRanges::GRAIN_DURATION.convertTo0to1(mSliderDuration.getValue()))));
    // Find synced duration/rate using fixed 120 bpm and 4 beats per bar (different from actual synthesis, just for vis)
    durSec = (1.0f / 120) * 60.0f * (4 / div);
    div = std::pow(2, juce::roundToInt(ParamRanges::SYNC_DIV_MAX * ParamRanges::GRAIN_RATE.convertTo0to1(mSliderRate.getValue())));
    rateSec = (1.0f / 120) * 60.0f * (4 / div);
  } else {
    durSec = mSliderDuration.getValue();
    rateSec = 1.0f / mSliderRate.getValue();
  }
  
  float envWidth = (durSec / WINDOW_SECONDS) * mVizRect.getWidth();
  float envOffset = (rateSec / WINDOW_SECONDS) * mVizRect.getWidth();

  float shapeWidth = envWidth * mSliderShape.getValue() / 2.0f;
  
  juce::Path clipPath;
  clipPath.addRectangle(mVizRect);
  auto envBounds = mVizRect.reduced(2, 2);

  // Draw darker odd numbered envelopes
  float tiltWidth = (envWidth * 0.5f) + (mSliderTilt.getValue() * envWidth * 0.5f);
  float curXStart = envBounds.getX() + envOffset;
  juce::Colour envColour = colour.darker(0.5f);
  while (curXStart < envBounds.getWidth()) {
    juce::Path envPath;
    juce::Point<float> lastPt = juce::Point<float>(curXStart, envBounds.getBottom());
    juce::Point<float> pt = juce::Point<float>(juce::jmax(curXStart, curXStart + tiltWidth - shapeWidth), envBounds.getY());
    envPath.addLineSegment(clipPath.getClippedLine(juce::Line(lastPt, pt), false), 1.0f);
    lastPt = pt;
    pt = juce::Point<float>(juce::jmin(curXStart + envWidth, curXStart + tiltWidth + shapeWidth), envBounds.getY());
    envPath.addLineSegment(clipPath.getClippedLine(juce::Line(lastPt, pt), false), 1.0f);
    lastPt = pt;
    pt = juce::Point<float>(curXStart + envWidth, envBounds.getBottom());
    envPath.addLineSegment(clipPath.getClippedLine(juce::Line(lastPt, pt), false), 1.0f);
    g.setColour(envColour);
    g.strokePath(envPath.createPathWithRoundedCorners(5), mPathStroke);
    curXStart += (envOffset * 2.0f);
  }
  curXStart = envBounds.getX();
  // Draw brighter even numbered envelopes
  envColour = colour.brighter(0.5f);
  while (curXStart < envBounds.getWidth()) {
    juce::Path envPath;
    juce::Point<float> lastPt = juce::Point<float>(curXStart, envBounds.getBottom());
    juce::Point<float> pt = juce::Point<float>(juce::jmax(curXStart, curXStart + tiltWidth - shapeWidth), envBounds.getY());
    envPath.addLineSegment(clipPath.getClippedLine(juce::Line(lastPt, pt), false), 1.0f);
    lastPt = pt;
    pt = juce::Point<float>(juce::jmin(curXStart + envWidth, curXStart + tiltWidth + shapeWidth), envBounds.getY());
    envPath.addLineSegment(clipPath.getClippedLine(juce::Line(lastPt, pt), false), 1.0f);
    lastPt = pt;
    pt = juce::Point<float>(curXStart + envWidth, envBounds.getBottom());
    envPath.addLineSegment(clipPath.getClippedLine(juce::Line(lastPt, pt), false), 1.0f);
    g.setColour(envColour);
    g.strokePath(envPath.createPathWithRoundedCorners(5), mPathStroke);
    curXStart += (envOffset * 2.0f);
  }
}

void EnvelopeGrain::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING);

  // Place labels
  auto labelPanel = r.removeFromBottom(Utils::LABEL_HEIGHT);
  int labelWidth = labelPanel.getWidth() / 4;
  mLabelShape.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelTilt.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelRate.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelDuration.setBounds(labelPanel.removeFromLeft(labelWidth));

  // Place sliders
  auto knobPanel = r.removeFromBottom(Utils::KNOB_HEIGHT);
  int knobWidth = knobPanel.getWidth() / 4;
  mSliderShape.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderTilt.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDuration.setBounds(knobPanel.removeFromLeft(knobWidth));

  // Place button
  auto syncPanel = r.removeFromRight(r.getWidth() * 0.25f);
  mBtnSync.setBounds(syncPanel.withSizeKeepingCentre(syncPanel.getWidth(), Utils::LABEL_HEIGHT + 4));

  r.removeFromRight(Utils::PADDING);
  r.removeFromBottom(Utils::PADDING);
  
  mVizRect = r.toFloat();
}
