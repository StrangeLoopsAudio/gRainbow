/*
  ==============================================================================

    ArcSpectrogram.cpp
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#include "ArcSpectrogram.h"

#include <JuceHeader.h>
#include <limits.h>

#include "Utils.h"

//==============================================================================
ArcSpectrogram::ArcSpectrogram(ParamsNote& paramsNote, ParamUI& paramUI)
    : mCurPitchClass(Utils::PitchClass::C),
      mIsPlayingNote(false),
      mProcessType(ParamUI::SpecType::INVALID),
      mParamsNote(paramsNote),
      mParamUI(paramUI),
      juce::Thread("spectrogram thread") {
  setFramesPerSecond(REFRESH_RATE_FPS);
  mBuffers.fill(nullptr);

  mLogoImage = juce::PNGImageFormat::loadFrom(BinaryData::logo_png, BinaryData::logo_pngSize);

  // check if params has images, which would mean the plugin was reopened
  if (mParamUI.specComplete) {
    mProcessType = (ParamUI::SpecType)mParamUI.specType;
    mSpecType.setVisible(true);
  } else {
    // if not complete, we assume all images will be remade, no "half way"
    // support currently
    for (int i = 0; i < (int)ParamUI::SpecType::COUNT; i++) {
      mImagesComplete[(ParamUI::SpecType)i] = false;
    }
  }

  // ComboBox for some reason is not zero indexed like the rest of JUCE and C++
  // for adding items we go by 'id' base but everything else is 'index' based
  mSpecType.addItem("Spectrogram", (int)ParamUI::SpecType::SPECTROGRAM + 1);
  mSpecType.addItem("Harmonic Profile", (int)ParamUI::SpecType::HPCP + 1);
  mSpecType.addItem("Detected Pitches", (int)ParamUI::SpecType::DETECTED + 1);
  mSpecType.setTooltip("Select different spectrum type");
  mSpecType.onChange = [this](void) {
    // Will get called from user using UI ComboBox and from inside this class
    // when loading buffers
    mParamUI.specType = mSpecType.getSelectedItemIndex();
    repaint();
  };

  addChildComponent(mSpecType);
}

ArcSpectrogram::~ArcSpectrogram() {
  mParamsNote.notes[mCurPitchClass]->onGrainCreated = nullptr;
  stopThread(4000);
}

void ArcSpectrogram::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw selected type
  // if nothing has been loaded, want to keep drawing the logo
  juce::Image& drawImage = mLogoImage;
  if (mProcessType != ParamUI::SpecType::INVALID) {
    int imageIndex = mSpecType.getSelectedItemIndex();
    // When loading up a plugin a second time, need to set the ComboBox state,
    // but can't in the constructor so there is the first spot we can enforce
    // it. Without this, the logo will appear when reopening the plugin
    if (imageIndex == -1) {
      mSpecType.setSelectedItemIndex(mProcessType, juce::dontSendNotification);
      imageIndex = (int)mProcessType;
    }
    drawImage = mParamUI.specImages[imageIndex];
  }

  juce::Point<float> centerPoint = juce::Point<float>(getWidth() / 2.0f, getHeight());
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;

  g.drawImage(drawImage, getLocalBounds().toFloat(), juce::RectanglePlacement(juce::RectanglePlacement::fillDestination), false);

  // Draw active grains
  for (ArcGrain& grain : mArcGrains) {
    ParamCandidate* candidate = mParamsNote.notes[grain.paramGenerator->noteIdx]->getCandidate(grain.paramGenerator->genIdx);
    float xRatio = candidate->posRatio + (candidate->duration * grain.paramGenerator->positionAdjust->get());
    float grainProg = (grain.numFramesActive * grain.envIncSamples) / ENV_LUT_SIZE;
    xRatio += (candidate->duration / candidate->pbRate) * grainProg;
    float pitchClass = grain.paramGenerator->noteIdx - (std::log(candidate->pbRate) / std::log(Utils::TIMESTRETCH_RATIO));
    float yRatio = (pitchClass + 0.25f + (grain.paramGenerator->pitchAdjust->get() * 6.0f)) / (float)Utils::PitchClass::COUNT;
    int grainRad = startRadius + (yRatio * bowWidth);
    juce::Point<float> grainPoint = centerPoint.getPointOnCircumference(
        grainRad, (1.5 * juce::MathConstants<float>::pi) + (xRatio * juce::MathConstants<float>::pi));
    float envIdx = juce::jmin(ENV_LUT_SIZE - 1.0f, grain.numFramesActive * grain.envIncSamples);
    float grainSize = grain.gain * grain.paramGenerator->grainEnv[envIdx] * MAX_GRAIN_SIZE;
    juce::Rectangle<float> grainRect = juce::Rectangle<float>(grainSize, grainSize).withCentre(grainPoint);
    g.setColour(juce::Colour(Utils::GENERATOR_COLOURS_HEX[grain.paramGenerator->genIdx]));
    g.fillEllipse(grainRect);
    g.setColour(juce::Colour::fromHSV(yRatio, 1.0, 1.0f, 1.0f));
    g.fillEllipse(grainRect.reduced(2));
    grain.numFramesActive++;
  }

  // Remove arc grains that are completed
  mArcGrains.removeIf([this](ArcGrain& grain) { return (grain.numFramesActive * grain.envIncSamples) > ENV_LUT_SIZE; });
}

void ArcSpectrogram::resized() {
  auto r = getLocalBounds();
  // Spec type combobox
  mSpecType.setBounds(r.removeFromRight(SPEC_TYPE_WIDTH).removeFromTop(SPEC_TYPE_HEIGHT));
}

void ArcSpectrogram::run() {
  Utils::SpecBuffer& spec = *mBuffers[mProcessType];
  if (spec.size() == 0 || threadShouldExit()) return;

  // Initialize rainbow parameters
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;
  int maxRow = (mProcessType == ParamUI::SpecType::SPECTROGRAM) ? spec[0].size() / 8 : spec[0].size();
  juce::Point<int> startPoint = juce::Point<int>(getWidth() / 2, getHeight());
  mParamUI.specImages[mProcessType] = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
  juce::Graphics g(mParamUI.specImages[mProcessType]);

  // Draw each column of frequencies
  for (auto i = 0; i < NUM_COLS; ++i) {
    if (threadShouldExit()) return;
    auto specCol = ((float)i / NUM_COLS) * spec.size();
    // Draw each row of frequencies
    for (auto curRadius = startRadius; curRadius < endRadius; curRadius += 1) {
      float radPerc = (curRadius - startRadius) / (float)bowWidth;
      auto specRow = radPerc * maxRow;

      // Choose rainbow color depending on radius
      auto level = juce::jlimit(0.0f, 1.0f, spec[specCol][specRow] * spec[specCol][specRow] * COLOUR_MULTIPLIER);
      auto rainbowColour = juce::Colour::fromHSV(radPerc, 1.0, 1.0f, level);
      g.setColour(rainbowColour);

      float xPerc = (float)specCol / spec.size();
      float angleRad = (juce::MathConstants<float>::pi * xPerc) - (juce::MathConstants<float>::pi / 2.0f);

      // Create and rotate a rectangle to represent the "pixel"
      juce::Point<float> p = startPoint.getPointOnCircumference(curRadius, curRadius, angleRad);
      juce::AffineTransform rotation = juce::AffineTransform();
      rotation = rotation.rotated(angleRad, p.x, p.y);
      juce::Rectangle<float> rect = juce::Rectangle<float>(2, 2);
      rect = rect.withCentre(p);
      rect = rect.transformedBy(rotation);
      juce::Path rectPath;
      rectPath.addRectangle(rect);

      // Finally, draw the rectangle
      g.fillPath(rectPath, rotation);
    }
  }

  // pass type as another thread can change member variable right after run() is
  // done
  onImageComplete(mProcessType);
}

void ArcSpectrogram::onImageComplete(ParamUI::SpecType specType) {
  mImagesComplete[specType] = true;
  for (int i = 0; i < (int)ParamUI::SpecType::COUNT; i++) {
    if (mImagesComplete[(ParamUI::SpecType)i] == false) {
      return;
    }
  }
  mParamUI.specComplete = true;
  // Lets UI know it so it can enable other UI components
  onImagesComplete();
}

void ArcSpectrogram::reset() {
  // Reset all images
  for (int i = 0; i < mParamUI.specImages.size(); i++) {
    mParamUI.specImages[i].clear(mParamUI.specImages[i].getBounds());
  }
  for (int i = 0; i < (int)ParamUI::SpecType::COUNT; i++) {
    mImagesComplete[(ParamUI::SpecType)i] = false;
  }
  mParamUI.specComplete = false;
}

void ArcSpectrogram::loadBuffer(Utils::SpecBuffer* buffer, ParamUI::SpecType type) {
  if (buffer == nullptr) return;
  waitForThreadToExit(BUFFER_PROCESS_TIMEOUT);
  if (mImagesComplete[type]) return;
  mProcessType = type;
  mBuffers[mProcessType] = buffer;

  // make visible when loading the buffer if it isn't already
  mSpecType.setVisible(true);

  // As each buffer is loaded, want to display it being generated
  // Will be loaded in what ever order loaded from async callbacks
  // The last item loaded will be the first item selected in ComboBox
  if ((int)mProcessType < mSpecType.getNumItems()) {
    mSpecType.setSelectedItemIndex(mProcessType, juce::sendNotification);
  }
  // Only make image if component size has been set
  if (getWidth() > 0 && getHeight() > 0) startThread();
}

// loadBuffer is never called when a preset is loaded
void ArcSpectrogram::loadPreset() {
  // make visible if preset was loaded first
  mSpecType.setVisible(true);
  mProcessType = (ParamUI::SpecType)mParamUI.specType;
  mSpecType.setSelectedItemIndex(mProcessType, juce::dontSendNotification);
  repaint();
}

void ArcSpectrogram::setNoteOn(Utils::PitchClass pitchClass) {
  mParamsNote.notes[mCurPitchClass]->onGrainCreated = nullptr;
  mParamsNote.notes[pitchClass]->onGrainCreated = [this](int genIdx, float durationSec, float envGain) {
    grainCreatedCallback(genIdx, durationSec, envGain);
  };
  mIsPlayingNote = true;
  mCurPitchClass = pitchClass;
}

void ArcSpectrogram::grainCreatedCallback(int genIdx, float durationSec, float envGain) {
  if (mArcGrains.size() >= MAX_NUM_GRAINS) return;
  ParamGenerator* gen = mParamsNote.notes[mCurPitchClass]->generators[genIdx].get();
  float envIncSamples;
  if (gen->grainSync->get()) {
    envIncSamples = ENV_LUT_SIZE / (durationSec * REFRESH_RATE_FPS);
  } else {
    envIncSamples = ENV_LUT_SIZE / (durationSec * REFRESH_RATE_FPS);
  }

  mArcGrains.add(ArcGrain(gen, envGain, envIncSamples));
}