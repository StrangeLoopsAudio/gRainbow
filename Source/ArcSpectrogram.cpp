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
ArcSpectrogram::ArcSpectrogram() : juce::Thread("spectrogram thread") {
  setFramesPerSecond(10);
  mBuffers.fill(nullptr);

  mImages[Utils::SpecType::LOGO] = juce::PNGImageFormat::loadFrom(
     BinaryData::logo_png, BinaryData::logo_pngSize);
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  juce::File imageFile = parentDir.getChildFile(Utils::FILE_SPECTROGRAM);
  if (imageFile.existsAsFile()) {
    mImages[Utils::SpecType::SPECTROGRAM] =
        juce::PNGImageFormat::loadFrom(imageFile);
  }
  imageFile = parentDir.getChildFile(Utils::FILE_HPCP);
  if (imageFile.existsAsFile()) {
    mImages[Utils::SpecType::HPCP] =
        juce::PNGImageFormat::loadFrom(imageFile);
  }
  imageFile = parentDir.getChildFile(Utils::FILE_NOTES);
  if (imageFile.existsAsFile()) {
    mImages[Utils::SpecType::NOTES] =
        juce::PNGImageFormat::loadFrom(imageFile);
  } 

  mSpecType.addItem("Spectrogram", (int)Utils::SpecType::SPECTROGRAM);
  mSpecType.addItem("Harmonic Profile", (int)Utils::SpecType::HPCP);
  mSpecType.addItem("Detected Pitches", (int)Utils::SpecType::NOTES);
  mSpecType.setSelectedId(0, juce::dontSendNotification);
  mSpecType.onChange = [this](void) {
    if (mSpecType.getSelectedId() != Utils::SpecType::LOGO) {
      mSpecType.setVisible(true);
    }
    repaint();
  };
  addChildComponent(mSpecType);
}

ArcSpectrogram::~ArcSpectrogram() {
  stopThread(4000);
  // Save image files
  juce::PNGImageFormat pngWriter;
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  if (mImages[Utils::SpecType::SPECTROGRAM].isValid()) {
    parentDir.getChildFile(Utils::FILE_SPECTROGRAM).deleteFile();
    juce::FileOutputStream imageStream(parentDir.getChildFile(Utils::FILE_SPECTROGRAM));
    pngWriter.writeImageToStream(mImages[Utils::SpecType::SPECTROGRAM],
                                 imageStream);
  }
  if (mImages[Utils::SpecType::HPCP].isValid()) {
    parentDir.getChildFile(Utils::FILE_HPCP).deleteFile();
    juce::FileOutputStream imageStream(parentDir.getChildFile(Utils::FILE_HPCP));
    pngWriter.writeImageToStream(mImages[Utils::SpecType::HPCP],
                                 imageStream);
  }
  if (mImages[Utils::SpecType::NOTES].isValid()) {
    parentDir.getChildFile(Utils::FILE_NOTES).deleteFile();
    juce::FileOutputStream imageStream(parentDir.getChildFile(Utils::FILE_NOTES));
    pngWriter.writeImageToStream(mImages[Utils::SpecType::NOTES],
                                 imageStream);
  }
}

void ArcSpectrogram::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw selected type
  Utils::SpecType specType = (Utils::SpecType)(mSpecType.getSelectedId());
  juce::Point<float> centerPoint =
      juce::Point<float>(getWidth() / 2.0f, getHeight());
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;
  juce::Image& curImage = mImages[specType];

  if (mIsPlayingNote) {
    juce::Path sunRays;
    juce::Image vibratingImage = juce::Image(curImage);
    vibratingImage.duplicateIfShared();

    for (GrainPositionFinder::GrainPosition& gPos : mGPositions) {
      if (!gPos.isActive) continue;
      auto sweepPos = gPos.pitch.posRatio +
                    ((gPos.pitch.duration / 2) * (mNormalRand(mGenRandom) + 1));
      int startVibRad =
          startRadius +
          (((float)gPos.pitch.pitchClass / Utils::PitchClass::COUNT) *
           bowWidth) -
          (bowWidth / 4.0f);
      int endVibRad = startVibRad + (bowWidth / 2.0f);

      for (int radius = startVibRad; radius < endVibRad; ++radius) {
        auto ogPoint = centerPoint.getPointOnCircumference(
            radius, (1.5 * juce::MathConstants<float>::pi) +
                        (sweepPos * juce::MathConstants<float>::pi));
        juce::Colour ogPixel =
            curImage.getPixelAt(ogPoint.getX(), ogPoint.getY());
        if (ogPixel.getBrightness() > 0.1) {
          float xOffset = mNormalRand(mGenRandom) * MAX_PIXEL_VIBRATION;
          float yOffset = mNormalRand(mGenRandom) * MAX_PIXEL_VIBRATION;
          juce::Point<float> newPoint = juce::Point<float>(
              ogPoint.getX() + xOffset, ogPoint.getY() + yOffset);
          juce::Colour newPixel = curImage.getPixelAt(newPoint.getX(), newPoint.getY());
          juce::Colour newColour = ogPixel.withAlpha(
              juce::jmax(ogPixel.getAlpha(), newPixel.getAlpha()));
          vibratingImage.setPixelAt(newPoint.getX(), newPoint.getY(),
                                    newColour);
          vibratingImage.setPixelAt(newPoint.getX() + 1, newPoint.getY(),
                                    newColour);
          vibratingImage.setPixelAt(newPoint.getX() - 1, newPoint.getY(),
                                    newColour);
          vibratingImage.setPixelAt(newPoint.getX(), newPoint.getY() + 1,
                                    newColour);
          vibratingImage.setPixelAt(newPoint.getX(), newPoint.getY() - 1,
                                    newColour);
          vibratingImage.setPixelAt(newPoint.getX() + 1, newPoint.getY() + 1,
                                    newColour);
          vibratingImage.setPixelAt(newPoint.getX() + 1, newPoint.getY() - 1,
                                    newColour);
          vibratingImage.setPixelAt(newPoint.getX() - 1, newPoint.getY() + 1,
                                    newColour);
          vibratingImage.setPixelAt(newPoint.getX() - 1, newPoint.getY() - 1,
                                    newColour);
        }
      }
    }
    g.drawImage(
        vibratingImage, getLocalBounds().toFloat(),
        juce::RectanglePlacement(juce::RectanglePlacement::fillDestination),
        false);
  } else {
    g.drawImage(
        mImages[specType], getLocalBounds().toFloat(),
        juce::RectanglePlacement(juce::RectanglePlacement::fillDestination),
        false);
  }

  // Draw transient markers
  if (mTransients != nullptr) {
    g.setOpacity(0.7);
    g.setColour(juce::Colours::blue);
    for (int i = 0; i < mTransients->size(); ++i) {
      TransientDetector::Transient transient = mTransients->at(i);
      auto startPoint = centerPoint.getPointOnCircumference(
          getHeight() - 21,
          (1.5 * juce::MathConstants<float>::pi) +
              (transient.posRatio * juce::MathConstants<float>::pi));
      auto endPoint = centerPoint.getPointOnCircumference(
          getHeight() - 17,
          (1.5 * juce::MathConstants<float>::pi) +
              (transient.posRatio * juce::MathConstants<float>::pi));
      g.drawLine(juce::Line<float>(startPoint, endPoint), 1.0f);
    }
  }
}

void ArcSpectrogram::resized() {
  auto r = getLocalBounds();
  // Spec type combobox
  mSpecType.setBounds(
      r.removeFromRight(SPEC_TYPE_WIDTH).removeFromTop(SPEC_TYPE_HEIGHT));
}

void ArcSpectrogram::run() {
    std::vector<std::vector<float>>& spec = *mBuffers[mProcessType - 1];
  if (spec.size() == 0 || threadShouldExit()) return;

  // Initialize rainbow parameters
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;
  int maxRow = (mProcessType == Utils::SpecType::SPECTROGRAM)
                   ? spec[0].size() / 8
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
          0.0f, 1.0f, spec[specCol][specRow] * spec[specCol][specRow] * COLOUR_MULTIPLIER);
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

void ArcSpectrogram::resetBuffers() {
  // Reset all images except logo
  for (int i = 1; i < mImages.size(); ++i) {
    mImages[i].clear(mImages[i].getBounds());
  }
}

void ArcSpectrogram::loadBuffer(std::vector<std::vector<float>>* buffer,
                                Utils::SpecType type) {
  if (buffer == nullptr) return;
  waitForThreadToExit(BUFFER_PROCESS_TIMEOUT);
  mBuffers[type - 1] = buffer;
  mProcessType = type;

  const juce::MessageManagerLock lock;
  if (type == Utils::SpecType::SPECTROGRAM || type == Utils::SpecType::HPCP) {
    mSpecType.setSelectedId(mProcessType, juce::sendNotification);
  }
  // Only make image if component size has been set
  if (getWidth() > 0 && getHeight() > 0) startThread();
}

void ArcSpectrogram::setTransients(
    std::vector<TransientDetector::Transient>* transients) {
  mTransients = transients;
}

void ArcSpectrogram::setPositions(
  std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  mGPositions = gPositions;
  for (int i = 0; i < gPositions.size(); ++i) {
    if (gPositions[i].pitch.pitchClass == Utils::PitchClass::NONE) continue;
    mGPositions.push_back(gPositions[i]);
  }
}

void ArcSpectrogram::setNoteOn(
    int midiNote, std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  mIsPlayingNote = true;
  mCurNote = midiNote;
  mGPositions = gPositions;
  for (int i = 0; i < gPositions.size(); ++i) {
    if (gPositions[i].pitch.pitchClass == Utils::PitchClass::NONE) continue;
    mGPositions.push_back(gPositions[i]);
  }
}