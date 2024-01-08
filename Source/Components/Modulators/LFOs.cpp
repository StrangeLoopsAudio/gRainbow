/*
  ==============================================================================

    LFOs.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "LFOs.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"
#include "Modulators.h"

LFOs::LFOs(Parameters& parameters): mParameters(parameters),
  mSliderRate(parameters.global.lfo1.rate, ParamRanges::LFO_RATE, false),
  mSliderDepth(parameters.global.lfo1.depth),
  mSliderPhase(parameters.global.lfo1.phase) {
    
  int shapeId = 1;
  for (const LFOModSource::Shape &shape : LFOModSource::LFO_SHAPES) {
    mChoiceShape.addItem(shape.name, shapeId++);
  }
  mChoiceShape.setSelectedId(1, juce::dontSendNotification);
  mChoiceShape.onChange = [this]() {
    ParamHelper::setParam(mParameters.global.lfo1.shape, mChoiceShape.getSelectedId() - 1);
  };
  mChoiceShape.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mChoiceShape);
    
  // Default slider settings
  std::vector<std::reference_wrapper<juce::Slider>> sliders = { mSliderRate, mSliderDepth, mSliderPhase };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(slider.get());
  }
  mSliderRate.setSync(false);
  mSliderRate.setRange(ParamRanges::LFO_RATE.start, ParamRanges::LFO_RATE.end, 0.01);
  mSliderRate.setSuffix("hz");
  mSliderDepth.setRange(ParamRanges::LFO_DEPTH.start, ParamRanges::LFO_DEPTH.end, 0.01);
  mSliderPhase.setRange(ParamRanges::LFO_PHASE.start, ParamRanges::LFO_PHASE.end, 0.01);
  
  // Default button settings
  std::vector<std::reference_wrapper<juce::TextButton>> buttons = { mBtnSync, mBtnBipolar };
  for (auto& button : buttons) {
    button.get().setToggleable(true);
    button.get().setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    button.get().setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    button.get().setColour(juce::TextButton::buttonColourId, Utils::GLOBAL_COLOUR);
    button.get().setColour(juce::TextButton::buttonOnColourId, Utils::GLOBAL_COLOUR);
    addAndMakeVisible(button.get());
  }
  mBtnSync.setButtonText("free");
  mBtnSync.onClick = [this]() {
    ParamHelper::setParam(mParameters.global.lfo1.sync, !mBtnSync.getToggleState());
  };
  mBtnBipolar.setButtonText("bi");
  mBtnBipolar.onClick = [this]() {
    ParamHelper::setParam(mParameters.global.lfo1.bipolar, !mBtnBipolar.getToggleState());
  };

  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelShape, mLabelRate, mLabelDepth, mLabelPhase };
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
    label.get().setJustificationType(juce::Justification::centredTop);
    label.get().setFont(juce::Font(14));
    addAndMakeVisible(label.get());
  }
  mLabelShape.setText("shape", juce::dontSendNotification);
  mLabelRate.setText("rate", juce::dontSendNotification);
  mLabelDepth.setText("depth", juce::dontSendNotification);
  mLabelPhase.setText("phase", juce::dontSendNotification);
  
  
  // Add listeners for relevant params
  mParameters.global.lfo1.shape->addListener(this);
  mParameters.global.lfo1.rate->addListener(this);
  mParameters.global.lfo1.phase->addListener(this);
  mParameters.global.lfo1.depth->addListener(this);
  mParameters.global.lfo1.sync->addListener(this);
  mParameters.global.lfo1.bipolar->addListener(this);
    
  mParamHasChanged.store(true); // Init param values
  
  startTimer(Utils::UI_REFRESH_INTERVAL);
}

LFOs::~LFOs() {
  // Remove listeners before destruction
  mParameters.global.lfo1.shape->removeListener(this);
  mParameters.global.lfo1.rate->removeListener(this);
  mParameters.global.lfo1.phase->removeListener(this);
  mParameters.global.lfo1.depth->removeListener(this);
  mParameters.global.lfo1.sync->removeListener(this);
  mParameters.global.lfo1.bipolar->removeListener(this);
  stopTimer();
}

void LFOs::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void LFOs::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mBtnSync.setToggleState(mParameters.global.lfo1.sync->get(), juce::dontSendNotification);
    mBtnSync.setButtonText(mBtnSync.getToggleState() ? "sync" : "free");
    mBtnBipolar.setToggleState(mParameters.global.lfo1.bipolar->get(), juce::dontSendNotification);
    mBtnBipolar.setButtonText(mBtnBipolar.getToggleState() ? "bi" : "uni");
    mChoiceShape.setSelectedId(mParameters.global.lfo1.shape->getIndex() + 1, juce::dontSendNotification);
    mSliderRate.setValue(mParameters.global.lfo1.rate->get(), juce::dontSendNotification);
    mSliderRate.setSync(mBtnSync.getToggleState());
    mSliderRate.setRange(mSliderRate.getRange(), mBtnSync.getToggleState() ? mSliderRate.getRange().getLength() / (ParamRanges::SYNC_DIV_MAX) : 0.01);
    mSliderDepth.setValue(mParameters.global.lfo1.depth->get(), juce::dontSendNotification);
    mSliderPhase.setValue(mParameters.global.lfo1.phase->get(), juce::dontSendNotification);
    updateLfoPath();
  }
  repaint();
}

void LFOs::paint(juce::Graphics& g) {
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
  
  g.setColour(Utils::BG_COLOUR);
  g.fillRect(mVizRect);
  
  // Draw bipolar/unipolar line
  g.setColour(Utils::GLOBAL_COLOUR.withAlpha(0.5f));
  const int poleY = mBtnBipolar.getToggleState() ? mVizRect.getCentreY() : mVizRect.getBottom();
  g.drawHorizontalLine(poleY, mVizRect.getX(), mVizRect.getRight());
  
  // Draw LFO path
  g.setColour(Utils::GLOBAL_COLOUR);
  g.strokePath(mLfoPath, juce::PathStrokeType(2, juce::PathStrokeType::JointStyle::curved));
  
  const int lfoY = mVizRect.getCentreY() - (mParameters.global.lfo1.getOutput() * mVizRect.getHeight() / 2.0f);
  g.fillEllipse(juce::Rectangle<float>(3, 3).withPosition(mVizRect.getRight() + 1, lfoY));
}

void LFOs::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING);
  
  auto labelPanel = r.removeFromBottom(Utils::LABEL_HEIGHT);
  const int labelWidth = labelPanel.getWidth() / 4;
  mLabelShape.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelRate.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelDepth.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelPhase.setBounds(labelPanel.removeFromLeft(labelWidth));
  
  auto knobPanel = r.removeFromBottom(Utils::KNOB_HEIGHT);
  const int knobWidth = knobPanel.getWidth() / 4;
  mChoiceShape.setBounds(knobPanel.removeFromLeft(knobWidth).withTrimmedBottom(Utils::PADDING));
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mSliderDepth.setBounds(knobPanel.removeFromLeft(knobWidth).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mSliderPhase.setBounds(knobPanel.removeFromLeft(knobWidth).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  
  auto btnPanel = r.removeFromRight(r.getWidth() * 0.25f);
  mBtnSync.changeWidthToFitText(Utils::LABEL_HEIGHT);
  auto syncPanel = btnPanel.removeFromTop(btnPanel.getHeight() / 2);
  syncPanel.removeFromBottom(Utils::PADDING);
  mBtnSync.setBounds(syncPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mBtnBipolar.setBounds(btnPanel.removeFromTop(Utils::LABEL_HEIGHT));
  
  r.removeFromRight(Utils::PADDING);
  r.removeFromBottom(Utils::PADDING);
    
  mVizRect = r.toFloat();
  
}

void LFOs::updateLfoPath() {
  // Create LFO path
  mLfoPath.clear();
  float periodSec = 1.0 / mSliderRate.getValue();
  if (mBtnSync.getToggleState()) {
    float div = std::pow(2, juce::roundToInt(ParamRanges::SYNC_DIV_MAX * ParamRanges::LFO_RATE.convertTo0to1(mSliderRate.getValue())));
    // Find synced period using fixed 120 bpm and 4 beats per bar (different from actual synthesis, just for vis)
    periodSec = (1.0f / 120) * 60.0f * (4 / div);
  }
  const float numPeriods = WINDOW_SECONDS / periodSec;
  const int depthPx = mSliderDepth.getValue() * mVizRect.getHeight() / 2.0f;
  // Draw lfo shape
  float pxPerSamp = mVizRect.getWidth() / NUM_LFO_SAMPLES;
  float radPerSamp = (2.0f * M_PI * numPeriods) / NUM_LFO_SAMPLES;
  float curX = mVizRect.getX();
  float curRad = mSliderPhase.getValue();
  const float centerY = mBtnBipolar.getToggleState() ? mVizRect.getCentreY() : mVizRect.getBottom() - depthPx;
  const float startY = centerY - depthPx * LFOModSource::LFO_SHAPES[mChoiceShape.getSelectedId() - 1].calc(curRad);
  mLfoPath.startNewSubPath(curX, startY);
  for (int i = 0; i < NUM_LFO_SAMPLES; ++i) {
    float y = centerY - depthPx * LFOModSource::LFO_SHAPES[mChoiceShape.getSelectedId() - 1].calc(curRad);
    mLfoPath.lineTo(curX, y);
    curX += pxPerSamp;
    curRad += radPerSamp;
  }
}
