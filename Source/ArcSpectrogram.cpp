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
    : mParamsNote(paramsNote),
      mParamUI(paramUI),
      juce::Thread("spectrogram thread") {
  setFramesPerSecond(REFRESH_RATE_FPS);
  mBuffers.fill(nullptr);

  mImages[SpecType::LOGO] = juce::PNGImageFormat::loadFrom(
      BinaryData::logo_png, BinaryData::logo_pngSize);
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  juce::File imageFile = parentDir.getChildFile(FILE_SPECTROGRAM);
  if (imageFile.existsAsFile()) {
    mImages[SpecType::SPECTROGRAM] = juce::PNGImageFormat::loadFrom(imageFile);
  }
  imageFile = parentDir.getChildFile(FILE_HPCP);
  if (imageFile.existsAsFile()) {
    mImages[SpecType::HPCP] = juce::PNGImageFormat::loadFrom(imageFile);
  }
  imageFile = parentDir.getChildFile(FILE_DETECTED);
  if (imageFile.existsAsFile()) {
    mImages[SpecType::DETECTED] = juce::PNGImageFormat::loadFrom(imageFile);
  }

  // ComboBox for some reason is not zero indexed like the rest of JUCE and C++
  // for adding items we go by 'id' base but everything else is 'index' based
  mSpecType.addItem("Spectrogram", (int)SpecType::SPECTROGRAM + 1);
  mSpecType.addItem("Harmonic Profile", (int)SpecType::HPCP + 1);
  mSpecType.addItem("Detected Pitches", (int)SpecType::DETECTED + 1);
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
  // Save image files
  juce::PNGImageFormat pngWriter;
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  if (mImages[SpecType::SPECTROGRAM].isValid()) {
    parentDir.getChildFile(FILE_SPECTROGRAM).deleteFile();
    juce::FileOutputStream imageStream(
        parentDir.getChildFile(FILE_SPECTROGRAM));
    pngWriter.writeImageToStream(mImages[SpecType::SPECTROGRAM], imageStream);
  }
  if (mImages[SpecType::HPCP].isValid()) {
    parentDir.getChildFile(FILE_HPCP).deleteFile();
    juce::FileOutputStream imageStream(parentDir.getChildFile(FILE_HPCP));
    pngWriter.writeImageToStream(mImages[SpecType::HPCP], imageStream);
  }
  if (mImages[SpecType::DETECTED].isValid()) {
    parentDir.getChildFile(FILE_DETECTED).deleteFile();
    juce::FileOutputStream imageStream(parentDir.getChildFile(FILE_DETECTED));
    pngWriter.writeImageToStream(mImages[SpecType::DETECTED], imageStream);
  }
}

void ArcSpectrogram::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw selected type
  // if nothing has been loaded, want to keep drawing the logo
  SpecType specType = (mProcessType == SpecType::LOGO)
                          ? SpecType::LOGO
                          : (SpecType)(mSpecType.getSelectedItemIndex());
  juce::Point<float> centerPoint =
      juce::Point<float>(getWidth() / 2.0f, getHeight());
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;

  g.drawImage(
      mImages[specType], getLocalBounds().toFloat(),
      juce::RectanglePlacement(juce::RectanglePlacement::fillDestination),
      false);

  // Draw active grains
  for (ArcGrain& grain : mArcGrains) {
    ParamCandidate* candidate =
        mParamsNote.notes[grain.paramGenerator->noteIdx]->getCandidate(
            grain.paramGenerator->genIdx);
    float xRatio =
        candidate->posRatio +
        (candidate->duration * grain.paramGenerator->positionAdjust->get());
    float grainProg =
        (grain.numFramesActive * grain.envIncSamples) / ENV_LUT_SIZE;
    xRatio += candidate->duration * grainProg;
    float yRatio = (grain.paramGenerator->noteIdx +
                    (grain.paramGenerator->pitchAdjust->get() * 6.0f)) /
                   (float)Utils::PitchClass::COUNT;
    int grainRad = startRadius + (yRatio * bowWidth);
    juce::Point<float> grainPoint = centerPoint.getPointOnCircumference(
        grainRad, (1.5 * juce::MathConstants<float>::pi) +
                      (xRatio * juce::MathConstants<float>::pi));
    float envIdx = juce::jmin(MAX_GRAIN_SIZE - 1.0f,
                              grain.numFramesActive * grain.envIncSamples);
    float grainSize =
        grain.gain * grain.paramGenerator->grainEnv[envIdx] * MAX_GRAIN_SIZE;
    juce::Rectangle<float> grainRect =
        juce::Rectangle<float>(grainSize, grainSize).withCentre(grainPoint);
    g.setColour(juce::Colour(
        Utils::GENERATOR_COLOURS_HEX[grain.paramGenerator->genIdx]));
    g.fillEllipse(grainRect);
    g.setColour(juce::Colour::fromHSV(yRatio, 1.0, 1.0f, 1.0f));
    g.fillEllipse(grainRect.reduced(2));
    grain.numFramesActive++;
  }

  // Remove arc grains that are completed
  mArcGrains.removeIf([this](ArcGrain& g) {
    return (g.numFramesActive * g.envIncSamples) > ENV_LUT_SIZE;
  });
}

void ArcSpectrogram::resized() {
  auto r = getLocalBounds();
  // Spec type combobox
  mSpecType.setBounds(
      r.removeFromRight(SPEC_TYPE_WIDTH).removeFromTop(SPEC_TYPE_HEIGHT));
}

void ArcSpectrogram::run() {
  std::vector<std::vector<float>>& spec = *mBuffers[mProcessType];
  if (spec.size() == 0 || threadShouldExit()) return;

  // Initialize rainbow parameters
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;
  int maxRow = (mProcessType == SpecType::SPECTROGRAM) ? spec[0].size() / 8
                                                       : spec[0].size();
  juce::Point<int> startPoint = juce::Point<int>(getWidth() / 2, getHeight());
  mImages[mProcessType] =
      juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
  juce::Graphics g(mImages[mProcessType]);

  // Draw each column of frequencies
  for (auto i = 0; i < NUM_COLS; ++i) {
    if (threadShouldExit()) return;
    auto specCol = ((float)i / NUM_COLS) * spec.size();
    // Draw each row of frequencies
    for (auto curRadius = startRadius; curRadius < endRadius; curRadius += 1) {
      float radPerc = (curRadius - startRadius) / (float)bowWidth;
      auto specRow = radPerc * maxRow;

      // Choose rainbow color depending on radius
      auto level = juce::jlimit(
          0.0f, 1.0f,
          spec[specCol][specRow] * spec[specCol][specRow] * COLOUR_MULTIPLIER);
      auto rainbowColour = juce::Colour::fromHSV(radPerc, 1.0, 1.0f, level);
      g.setColour(rainbowColour);

      float xPerc = (float)specCol / spec.size();
      float angleRad = (juce::MathConstants<float>::pi * xPerc) -
                       (juce::MathConstants<float>::pi / 2.0f);

      // Create and rotate a rectangle to represent the "pixel"
      juce::Point<float> p =
          startPoint.getPointOnCircumference(curRadius, curRadius, angleRad);
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
}

void ArcSpectrogram::reset() {
  // Reset all images except logo
  for (int i = 1; i < mImages.size(); ++i) {
    mImages[i].clear(mImages[i].getBounds());
  }
}

void ArcSpectrogram::loadBuffer(std::vector<std::vector<float>>* buffer,
                                SpecType type) {
  if (buffer == nullptr) return;
  waitForThreadToExit(BUFFER_PROCESS_TIMEOUT);
  mProcessType = type;
  mBuffers[mProcessType] = buffer;

  const juce::MessageManagerLock lock;

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

void ArcSpectrogram::setNoteOn(Utils::PitchClass pitchClass) {
  mParamsNote.notes[mCurPitchClass]->onGrainCreated = nullptr;
  mParamsNote.notes[pitchClass]->onGrainCreated = [this](int genIdx,
                                                         float envGain) {
    grainCreatedCallback(genIdx, envGain);
  };
  mIsPlayingNote = true;
  mCurPitchClass = pitchClass;
}

void ArcSpectrogram::grainCreatedCallback(int genIdx, float envGain) {
  ParamGenerator* gen =
      mParamsNote.notes[mCurPitchClass]->generators[genIdx].get();
  float envIncSamples =
      ENV_LUT_SIZE / (gen->grainDuration->get() * REFRESH_RATE_FPS);
  mArcGrains.add(ArcGrain(gen, envGain, envIncSamples));
}