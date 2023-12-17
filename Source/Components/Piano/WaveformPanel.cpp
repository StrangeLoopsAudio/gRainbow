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
  
  updateWaveBars();

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
  if (mBuffer.getNumSamples() == 0) return; // Skip if we don't even have a buffer
  int barWidth = getWidth() / NUM_WAVE_BARS;
  r.reduce(barWidth / 2, Utils::PADDING); // Reduce the area so wave bars fit nicely
  float barMaxHeight = r.getHeight();
  g.setColour(Utils::GLOBAL_COLOUR);
  float centreY = r.getCentreY();
  float curX = r.getX();
  for (WaveBar& bar : mWaveBars) {
    auto barRect = juce::Rectangle<float>(barWidth, bar.magnitude * barMaxHeight);
    g.setColour(bar.pitchClass == Utils::PitchClass::NONE ? Utils::GLOBAL_COLOUR : Utils::getRainbow12Colour(bar.pitchClass));
    if (bar.isEnabled) {
      g.fillRoundedRectangle(barRect.withCentre({curX + barWidth / 2, centreY}), 5);
    } else {
      g.drawRoundedRectangle(barRect.withCentre({curX + barWidth / 2, centreY}).reduced(1, 1), 5, 2);
    }
    curX += barWidth;
  }
}

void WaveformPanel::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING, Utils::PADDING);
  
}

void WaveformPanel::load(juce::AudioBuffer<float> &buffer) {
  mBuffer.setSize(1, buffer.getNumSamples());
  float gain = 1.0f / buffer.getMagnitude(0, 0, buffer.getNumSamples());
  mBuffer.copyFrom(0, 0, buffer.getReadPointer(0), buffer.getNumSamples(), gain);
  mZoomRange = juce::Range<int>(0, mBuffer.getNumSamples());
  mSamplesPerBar = mZoomRange.getLength() / NUM_WAVE_BARS;
  updateWaveBars();
}

void WaveformPanel::updateWaveBars() {
  if (mBuffer.getNumSamples() == 0) return;
  
  // Populate wave bar magnitudes
  int curSample = mZoomRange.getStart();
  for (auto& bar : mWaveBars) {
    bar = WaveBar(mBuffer.getMagnitude(0, curSample, mSamplesPerBar));
    curSample += mSamplesPerBar;
  }
  
  
  if (mCurSelectedParams->type == ParamType::GLOBAL) {
    // Color the bars with all note generators
    for (auto& note : mParameters.note.notes) {
      addBarsForNote(note.get(), false);
    }
  } else if (mCurSelectedParams->type == ParamType::NOTE) {
    // Note view (just show generators and candidates for particular note)
    addBarsForNote(dynamic_cast<ParamNote*>(mCurSelectedParams), true);
  } else {
    // Generator view
    
  }
  
  repaint();
}

void WaveformPanel::addBarsForNote(ParamNote* note, bool showCandidates) {
  if (note->candidates.empty()) return;
  for (auto& gen : note->generators) {
    if (!gen->enable->get() && !showCandidates) continue; // Skip if generator is off (unless we want to show its candidate)
    int sample = note->candidates[gen->candidate->get()].posRatio * mBuffer.getNumSamples();
    if (!mZoomRange.contains(sample)) continue; // Skip if we're outside of the visible range
    // Find the wave bar closest to this generator
    int closestBarIdx = (sample - mZoomRange.getStart()) / mSamplesPerBar;
    mWaveBars[closestBarIdx].pitchClass = (Utils::PitchClass)note->noteIdx;
    mWaveBars[closestBarIdx].isEnabled = gen->enable->get();
  }
  
}
