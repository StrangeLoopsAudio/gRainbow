/*
  ==============================================================================

    Envelopes.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "Envelopes.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

Envelopes::Envelopes(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mParamColour(Utils::GLOBAL_COLOUR) {
  

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

Envelopes::~Envelopes() {
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void Envelopes::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void Envelopes::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    
  }
}

void Envelopes::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();

  mParamHasChanged.store(true);
  repaint();
}

void Envelopes::paint(juce::Graphics& g) {
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
}

void Envelopes::resized() {
  juce::Rectangle<float> r = getLocalBounds().toFloat();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING);

  
}
