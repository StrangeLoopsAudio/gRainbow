/*
  ==============================================================================

    WaveformPanel.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "WaveformPanel.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

WaveformPanel::WaveformPanel(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mParamColour(Utils::GLOBAL_COLOUR) {
  

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

WaveformPanel::~WaveformPanel() {
  mCurSelectedParams->removeListener(this);
  stopTimer();
}

void WaveformPanel::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void WaveformPanel::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    
  }
}

void WaveformPanel::updateSelectedParams() {
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();

  mParamHasChanged.store(true);
  repaint();
}

void WaveformPanel::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat();
  
  // Outline
  g.setColour(Utils::BG_COLOUR);
  g.fillRoundedRectangle(r, 10.0f);
  g.setColour(Utils::GLOBAL_COLOUR);
  g.drawRoundedRectangle(r, 10.0f, 2.0f);
  
  // Wave bars
  if (mBuffer == nullptr) return; // Skip if we don't even have a buffer
  int barWidth = getWidth() / NUM_WAVE_BARS;
  r.reduce(barWidth / 2, Utils::PADDING); // Reduce the area so wave bars fit nicely
  float barMaxHeight = r.getHeight();
  g.setColour(Utils::GLOBAL_COLOUR);
  float centreY = r.getCentreY();
  float curX = r.getX();
  for (WaveBar& bar : mWaveBars) {
    auto barRect = juce::Rectangle<float>(barWidth, bar.magnitude * barMaxHeight);
    g.setColour(bar.pitchClass == Utils::PitchClass::NONE ? Utils::GLOBAL_COLOUR : Utils::getRainbow12Colour(bar.pitchClass));
    g.fillRoundedRectangle(barRect.withCentre({curX + barWidth / 2, centreY}), 5);
    curX += barWidth;
  }
}

void WaveformPanel::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING, Utils::PADDING);
  
}

void WaveformPanel::load(juce::AudioBuffer<float> *buffer) {
  mBuffer = buffer;
  mZoomRange = juce::Range<int>(0, buffer->getNumSamples());
  updateWaveBars();
}

void WaveformPanel::updateWaveBars() {
  if (mBuffer == nullptr) return;
  
  // Populate wave bar magnitudes
  int samplesPerBar = mZoomRange.getLength() / NUM_WAVE_BARS;
  int curSample = mZoomRange.getStart();
  for (auto& bar : mWaveBars) {
    bar = WaveBar(mBuffer->getMagnitude(0, curSample, samplesPerBar), Utils::PitchClass::NONE);
    curSample += samplesPerBar;
  }
  
  // Color the bars with note generators (should be candidates too in the future)
  for (auto& note : mParameters.note.notes) {
    if (note->candidates.empty()) continue;
    for (auto& gen : note->generators) {
      if (!gen->enable->get()) continue; // Skip if generator is off (though should still show probably later)
      int sample = note->candidates[gen->candidate->get()].posRatio * mBuffer->getNumSamples();
      if (!mZoomRange.contains(sample)) continue; // Skip if we're outside of the visible range
      // Find the wave bar closest to this generator
      int closestBarIdx = (sample - mZoomRange.getStart()) / samplesPerBar;
      mWaveBars[closestBarIdx].pitchClass = (Utils::PitchClass)note->noteIdx;
    }
  }
  
  repaint();
}
