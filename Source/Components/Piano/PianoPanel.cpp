/*
  ==============================================================================

    PianoPanel.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "PianoPanel.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

PianoPanel::PianoPanel(juce::MidiKeyboardState& state, Parameters& parameters)
    : waveform(parameters),
      keyboard(state, parameters),
      mParameters(parameters),
      mCurSelectedParams(parameters.getSelectedParams()),
      mParamColour(Utils::GLOBAL_COLOUR) {
  addAndMakeVisible(keyboard);
  addAndMakeVisible(waveform);

  mParameters.addListener(this);
  mCurSelectedParams->addListener(this);
  selectedCommonParamsChanged(mCurSelectedParams);

  startTimer(100);
}

PianoPanel::~PianoPanel() {
  mParameters.removeListener(this);
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void PianoPanel::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void PianoPanel::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
  }
}

void PianoPanel::selectedCommonParamsChanged(ParamCommon* newParams) {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = newParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();
  
  waveform.updateSelectedParams();
  
  mParamHasChanged.store(true);
  repaint();
}

void PianoPanel::paint(juce::Graphics& g) {
  // Panel outline
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().toFloat(), 10);
}

void PianoPanel::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING, Utils::PADDING);

  keyboard.setBounds(r.removeFromBottom(r.getHeight() * 0.6));
  r.removeFromBottom(Utils::PADDING);
  waveform.setBounds(r);
}
