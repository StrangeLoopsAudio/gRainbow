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
#include "BinaryData.h"

WaveformPanel::WaveformPanel(Parameters& parameters)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
mParamColour(Utils::GLOBAL_COLOUR) {

  mCurSelectedParams->addListener(this);
  updateSelectedParams();
        
  // Generator toggle button
  juce::Image normal = juce::PNGImageFormat::loadFrom(BinaryData::btnPowerOff_png, BinaryData::btnPowerOff_pngSize);
  juce::Image down = juce::PNGImageFormat::loadFrom(BinaryData::btnPowerOn_png, BinaryData::btnPowerOn_pngSize);
  mBtnGenEnable.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, juce::Image(), 1.0f,
                        juce::Colours::transparentBlack, down, 1.0f, juce::Colours::transparentBlack);
  mBtnGenEnable.setTooltip("Enable/disable generator");
  mBtnGenEnable.setToggleable(true);
  mBtnGenEnable.setClickingTogglesState(true);
  mBtnGenEnable.onClick = [this]() {
    auto* gen = dynamic_cast<ParamGenerator*>(mCurSelectedParams);
    if (gen) {
      ParamHelper::setParam(gen->enable, mBtnGenEnable.getToggleState());
      updateWaveBars();
    }
  };
  addChildComponent(mBtnGenEnable);

  // Generator toggle button
  normal = juce::PNGImageFormat::loadFrom(BinaryData::unlock_png, BinaryData::unlock_pngSize);
  down = juce::PNGImageFormat::loadFrom(BinaryData::lock_png, BinaryData::lock_pngSize);
  mBtnLock.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, juce::Image(), 1.0f,
                        juce::Colours::transparentBlack, down, 1.0f, juce::Colours::transparentBlack);
  mBtnLock.setTooltip("Lock view to generator");
  mBtnLock.setToggleable(true);
  mBtnLock.setClickingTogglesState(true);
  addChildComponent(mBtnLock);
  
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
  if (mBtnLock.isVisible() && mBtnLock.getToggleState() && mParameters.selectedParams->type != ParamType::GLOBAL) return; // Skip if locked to generator (but not if wanting to change to global)
  // Remove solo from note if was in generator mode
  auto* gen = dynamic_cast<ParamGenerator*>(mCurSelectedParams);
  if (gen) {
    ParamHelper::setParam(mParameters.note.notes[gen->noteIdx]->soloIdx, SOLO_NONE);
  }
  
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);
  mParamColour = mParameters.getSelectedParamColour();
  
  // Reset lock if necessary
  if (mBtnLock.getToggleState() && mCurSelectedParams->type == ParamType::GLOBAL) mBtnLock.setToggleState(false, juce::dontSendNotification);
  
  bool isGeneratorMode = mCurSelectedParams->type == ParamType::GENERATOR;
  // Set zoom range
  if (isGeneratorMode && mBuffer.getNumSamples() > 0) {
    auto* gen = dynamic_cast<ParamGenerator*>(mCurSelectedParams);
    if (gen) {
      ParamCandidate* candidate = mParameters.getGeneratorCandidate(gen);
      int start = candidate->posRatio * mBuffer.getNumSamples();
      int end = start + (candidate->duration * mBuffer.getNumSamples());
      int duration = end - start;
      mZoomRange = juce::Range<int>(start - duration / 2, end + duration / 2);
      mBtnGenEnable.setVisible(true);
      mBtnLock.setVisible(true);
      mBtnGenEnable.setToggleState(gen->enable->get(), juce::dontSendNotification);
      ParamHelper::setParam(mParameters.note.notes[gen->noteIdx]->soloIdx, gen->genIdx);
    }
  } else {
    mZoomRange = juce::Range<int>(0, mBuffer.getNumSamples());
    mBtnGenEnable.setVisible(false);
    mBtnLock.setVisible(false);
  }
  mSamplesPerBar = mZoomRange.getLength() / NUM_WAVE_BARS;
  updateWaveBars();

  mParamHasChanged.store(true);
  repaint();
}

void WaveformPanel::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat();
  
  // Outline
  g.setColour(Utils::GLOBAL_COLOUR);
  g.fillRoundedRectangle(r, 10.0f);
  g.setColour(Utils::BG_COLOUR);
  g.fillRoundedRectangle(r.reduced(Utils::PADDING / 2.0), 10.0f);
  
  // Wave bars
  if (mBuffer.getNumSamples() == 0) return; // Skip if we don't even have a buffer
  g.setColour(Utils::GLOBAL_COLOUR);
  for (WaveBar& bar : mWaveBars) {
    auto barColour = bar.pitchClass == Utils::PitchClass::NONE ? Utils::GLOBAL_COLOUR.darker() : Utils::getRainbow12Colour(bar.pitchClass);
    if (mHoverBar == &bar && bar.pitchClass != Utils::PitchClass::NONE) barColour = barColour.brighter();
    else if (mHoverBar != nullptr && mHoverBar->pitchClass != Utils::PitchClass::NONE) barColour = barColour.darker(0.2f);
    g.setColour(barColour);
    if (bar.isEnabled) {
      g.fillRoundedRectangle(bar.rect.reduced(1, 1), 5);
    } else {
      g.drawRoundedRectangle(bar.rect.reduced(1, 1), 5, 1);
    }
  }
}

void WaveformPanel::resized() {
  if (mBuffer.getNumSamples() == 0) return; // Skip if we don't even have a buffer
  auto r = getLocalBounds().reduced(Utils::PADDING, Utils::PADDING);
  
  // Positioning lock and enable buttons
  mBtnGenEnable.setBounds(r.withSize(18, 18));
  mBtnLock.setBounds(r.withWidth(18).withTop(r.getBottom() - 24).reduced(2));
  
  int barWidth = r.getWidth() / NUM_WAVE_BARS;
  float barMaxHeight = r.getHeight();
  float centreY = r.getCentreY();
  float curX = r.getX();
  for (WaveBar& bar : mWaveBars) {
    bar.rect = juce::Rectangle<float>(barWidth, juce::jmax(4.0f, bar.magnitude * barMaxHeight)).withCentre({curX + barWidth / 2, centreY});
    curX += barWidth;
  }
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
  
  auto* gen = dynamic_cast<ParamGenerator*>(mCurSelectedParams);
  ParamCandidate* candidate = nullptr;
  juce::Range<int> cRange; // Candidate sample range
  if (gen) {
    candidate = mParameters.getGeneratorCandidate(gen);
    cRange.setStart(candidate->posRatio * mBuffer.getNumSamples());
    cRange.setLength(candidate->duration * mBuffer.getNumSamples());
  }
  
  // Populate wave bar magnitudes
  int curSample = mZoomRange.getStart();
  for (auto& bar : mWaveBars) {
    float magnitude = (curSample > 0 && curSample < mBuffer.getNumSamples()) ? mBuffer.getMagnitude(0, curSample, mSamplesPerBar) : 0.0f;
    bar = WaveBar(magnitude);
    if (candidate) {
      // Color the bars within candidate area
      if (cRange.contains(curSample)) {
        bar.pitchClass = (Utils::PitchClass)gen->noteIdx;
        bar.isEnabled = gen->enable->get();
      }
    }
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
  }
  
  resized();
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
    mWaveBars[closestBarIdx].generator = gen.get();
  }
  
}

void WaveformPanel::mouseMove(const juce::MouseEvent& evt) {
  mHoverBar = nullptr;
  if (mBuffer.getNumSamples() == 0 || mCurSelectedParams->type == ParamType::GENERATOR) { repaint(); return; }
  auto pos = evt.getEventRelativeTo(this).getPosition();
  for (auto& bar : mWaveBars) {
    if (bar.rect.contains(pos.toFloat())) {
      mHoverBar = &bar;
      return;
    }
  }
  repaint();
}

void WaveformPanel::mouseDown(const juce::MouseEvent& evt) {
  if (mCurSelectedParams->type != ParamType::GENERATOR) return; // Skip if not viewing a generator
  mLastDragX = evt.getPosition().x;
}

void WaveformPanel::mouseDrag(const juce::MouseEvent& evt) {
  if (mCurSelectedParams->type != ParamType::GENERATOR) return; // Skip if not viewing a generator
  // Move candidate position based on drag
  auto* gen = dynamic_cast<ParamGenerator*>(mCurSelectedParams);
  ParamCandidate* candidate = mParameters.getGeneratorCandidate(gen);
  if (!candidate) return;
  const double diffProportion = (evt.getPosition().x - mLastDragX) / (double)getWidth();
  const double zoomProportion = mZoomRange.getLength() / (double)mBuffer.getNumSamples();
  candidate->posRatio -= diffProportion * zoomProportion;
  int start = candidate->posRatio * mBuffer.getNumSamples();
  int end = start + (candidate->duration * mBuffer.getNumSamples());
  int duration = end - start;
  mZoomRange = juce::Range<int>(start - duration / 2, end + duration / 2);
  mLastDragX = evt.getPosition().x;
  updateWaveBars();
}

void WaveformPanel::mouseUp(const juce::MouseEvent& evt) {
  if (mCurSelectedParams->type == ParamType::GENERATOR) return; // Skip if viewing a generator
  auto pos = evt.getEventRelativeTo(this).getPosition();
  for (auto& bar : mWaveBars) {
    if (bar.rect.contains(pos.toFloat()) && bar.pitchClass != Utils::PitchClass::NONE) {
      mParameters.selectedParams = bar.generator;
      if (mParameters.onSelectedChange) mParameters.onSelectedChange();
      return;
    }
  }
}

void WaveformPanel::mouseExit(const juce::MouseEvent&) {
  mHoverBar = nullptr;
  repaint();
}
