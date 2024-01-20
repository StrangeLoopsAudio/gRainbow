/*
  ==============================================================================

    FxPanel.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "FxPanel.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

FxPanel::FxPanel(Parameters& parameters)
    : mParameters(parameters), mCurSelectedParams(parameters.selectedParams), mParamColour(Utils::Colour::GLOBAL) {
  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

FxPanel::~FxPanel() {
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void FxPanel::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void FxPanel::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
  }
}

void FxPanel::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();

  mParamHasChanged.store(true);
  repaint();
}

void FxPanel::paint(juce::Graphics& g) {
  g.setColour(Utils::Colour::PANEL);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
}

void FxPanel::resized() {
  juce::Rectangle<float> r = getLocalBounds().toFloat();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING);
}
