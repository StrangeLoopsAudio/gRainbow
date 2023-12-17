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

LFOs::LFOs(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mParamColour(Utils::GLOBAL_COLOUR) {
  

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

LFOs::~LFOs() {
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void LFOs::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void LFOs::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    
  }
}

void LFOs::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();

  mParamHasChanged.store(true);
  repaint();
}

void LFOs::paint(juce::Graphics& g) {
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
}

void LFOs::resized() {
  juce::Rectangle<float> r = getLocalBounds().toFloat();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING);

  
}
