/*
  ==============================================================================

    ArcSpectrogram.cpp
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#define _USE_MATH_DEFINES

#include "ArcSpectrogram.h"

#include <JuceHeader.h>
#include <limits.h>
#include <math.h>
#include "Utils.h"

//==============================================================================
ArcSpectrogram::ArcSpectrogram():
      juce::Thread("spectrogram thread") {
  setFramesPerSecond(10);
  mBuffers.fill(nullptr);

  for (int i = 0; i < SpecType::NUM_TYPES; ++i) {
    mImages[i] = juce::Image(juce::Image::RGB, 512, 512, true);
  }

  mImages[SpecType::LOGO] =
      juce::PNGImageFormat::loadFrom(BinaryData::logo_png, BinaryData::logo_pngSize);

  mSpecType.addItem("Spectrogram", (int)SpecType::SPECTROGRAM);
  mSpecType.addItem("Harmonic Profile", (int)SpecType::HPCP);
  mSpecType.addItem("Detected Pitches", (int)SpecType::NOTES);
  mSpecType.setSelectedId(0, juce::dontSendNotification);
  mSpecType.onChange = [this](void) {
    if (mSpecType.getSelectedId() != SpecType::LOGO) {
      mSpecType.setVisible(true);
    }
    repaint();
  };
  addChildComponent(mSpecType);
}

ArcSpectrogram::~ArcSpectrogram() { stopThread(4000); }

void ArcSpectrogram::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw selected type
  SpecType specType = (SpecType)(mSpecType.getSelectedId());
  juce::Point<float> centerPoint =
      juce::Point<float>(getWidth() / 2.0f, getHeight());
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;
  juce::Image& curImage = mImages[specType];

  /*if (specType != SpecType::LOGO) {
    auto bgColourStart = mIsPlayingNote ? juce::Colour(COLOUR_SUN_REGULAR_RAYS)
                                        : juce::Colour(COLOUR_NIGHT);
    auto bgColourEnd =
        mIsPlayingNote ? juce::Colour(COLOUR_DAY)
                                      : juce::Colour(COLOUR_NIGHT);
    g.setFillType(juce::ColourGradient(bgColourStart, centerPoint, bgColourEnd,
                                       centerPoint.withY(0), true));
    g.fillAll();
  } */

  if (mIsPlayingNote) {
    juce::Path sunRays;
    juce::Image vibratingImage = juce::Image(curImage);
    vibratingImage.duplicateIfShared();

    for (GrainPositionFinder::GrainPosition gPos : mGPositions) {
      auto sweepPos = gPos.pitch.posRatio +
                    ((gPos.pitch.duration / 2) * (mNormalRand(mGenRandom) + 1));

      // Fraw sun ray underneath the spectrogram
      auto middlePos = gPos.pitch.posRatio + (gPos.pitch.duration / 2);
      auto rayPoint1 = centerPoint.getPointOnCircumference(
          getHeight(), (1.5 * M_PI) + ((middlePos - SUN_RAY_WIDTH) * M_PI));
      auto rayPoint2 = centerPoint.getPointOnCircumference(
          getHeight(), (1.5 * M_PI) + ((middlePos + SUN_RAY_WIDTH) * M_PI));

      sunRays.addTriangle(centerPoint, rayPoint1, rayPoint2);


      for (int i = 0; i < bowWidth; ++i) {
        auto ogPoint = centerPoint.getPointOnCircumference(
            (getHeight() / 4.0f) + i, (1.5 * M_PI) + (sweepPos * M_PI));
        auto ogPixel = curImage.getPixelAt(ogPoint.getX(), ogPoint.getY());
        if (ogPixel.getBrightness() > 0.1) {
          auto xOffset = mNormalRand(mGenRandom) * MAX_PIXEL_VIBRATION;
          auto yOffset = mNormalRand(mGenRandom) * MAX_PIXEL_VIBRATION;
          auto newPoint = juce::Point<float>(ogPoint.getX() + xOffset,
                                             ogPoint.getY() + yOffset);
          auto distance = ogPixel.getBrightness();
          vibratingImage.setPixelAt(newPoint.getX(), newPoint.getY(),
                                    ogPixel.withBrightness(distance));
          vibratingImage.setPixelAt(newPoint.getX() + 1, newPoint.getY(), ogPixel.withBrightness(distance));
          vibratingImage.setPixelAt(newPoint.getX() - 1, newPoint.getY(), ogPixel.withBrightness(distance));
          vibratingImage.setPixelAt(newPoint.getX(), newPoint.getY() + 1, ogPixel.withBrightness(distance));
          vibratingImage.setPixelAt(newPoint.getX(), newPoint.getY() - 1, ogPixel.withBrightness(distance));
          vibratingImage.setPixelAt(newPoint.getX() + 1, newPoint.getY() + 1, ogPixel.withBrightness(distance));
          vibratingImage.setPixelAt(newPoint.getX() + 1, newPoint.getY() - 1, ogPixel.withBrightness(distance));
          vibratingImage.setPixelAt(newPoint.getX() - 1, newPoint.getY() + 1, ogPixel.withBrightness(distance));
          vibratingImage.setPixelAt(newPoint.getX() - 1, newPoint.getY() - 1, ogPixel.withBrightness(distance));
        }
      }
    }
    g.drawImage(
        vibratingImage, getLocalBounds().toFloat(),
        juce::RectanglePlacement(juce::RectanglePlacement::fillDestination),
        false);

    // Draw the sun rays
   /* g.setFillType(juce::ColourGradient(
        juce::Colour(COLOUR_SUN_CENTER), centerPoint,
        juce::Colour(COLOUR_RAYS_END), centerPoint.withY(0), true));
    g.fillPath(sunRays); */

    // Draw the sun
   /* g.setFillType(juce::ColourGradient(
        juce::Colour(COLOUR_SUN_CENTER), centerPoint,
        juce::Colour(COLOUR_SUN_END), centerPoint.withY(getHeight() / 4.0f), true));
    auto sunArea =
        juce::Rectangle<float>(getHeight() / 2.0f, getHeight() / 2.0f);
    sunArea = sunArea.withCentre(centerPoint);
    g.fillEllipse(sunArea); */



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
          getHeight() - 21, (1.5 * M_PI) + (transient.posRatio * M_PI));
      auto endPoint = centerPoint.getPointOnCircumference(
          getHeight() - 17, (1.5 * M_PI) + (transient.posRatio * M_PI));
      g.drawLine(juce::Line<float>(startPoint, endPoint), 1.0f);
    }
  }
}

void ArcSpectrogram::resized() {
  auto r = getLocalBounds();
  // Spec type combobox
  mSpecType.setBounds(
      r.removeFromRight(SPEC_TYPE_WIDTH).removeFromTop(SPEC_TYPE_HEIGHT));

  // Position markers around rainbow
  juce::Point<int> startPoint = juce::Point<int>(getWidth() / 2, getHeight());
  for (int i = 0; i < mPositionMarkers.size(); ++i) {
    auto middlePos =
        mGPositions[i].pitch.posRatio + (mGPositions[i].pitch.duration / 2);
    float angleRad = (M_PI * middlePos) - (M_PI / 2.0f);
    juce::Point<float> p =
        startPoint.getPointOnCircumference(getHeight() - 10, getHeight() - 10, angleRad);
    mPositionMarkers[i]->setSize(POSITION_MARKER_WIDTH, POSITION_MARKER_HEIGHT);
    mPositionMarkers[i]->setCentrePosition(0, 0);
    mPositionMarkers[i]->setTransform(
        juce::AffineTransform::rotation(angleRad).translated(p));
  }
}

void ArcSpectrogram::run() {
    std::vector<std::vector<float>>& spec = *mBuffers[mProcessType - 1];
  if (spec.size() == 0 || threadShouldExit()) return;

  // Initialize rainbow parameters
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight() - POSITION_MARKER_HEIGHT;
  int bowWidth = endRadius - startRadius;
  int maxRow =
      (mProcessType == SpecType::SPECTROGRAM) ? spec[0].size() / 8 : spec[0].size();
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
      auto rainbowColour = juce::Colour::fromHSV(1 - radPerc, 1.0, level, 1.0f);
      g.setColour(rainbowColour);

      float xPerc = (float)specCol / spec.size();
      float angleRad = (M_PI * xPerc) - (M_PI / 2.0f);

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

void ArcSpectrogram::loadBuffer(std::vector<std::vector<float>>* buffer, SpecType type) {
  if (buffer == nullptr) return;
  waitForThreadToExit(BUFFER_PROCESS_TIMEOUT);
  mBuffers[type - 1] = buffer;
  mProcessType = type;

  const juce::MessageManagerLock lock;
  if (type == SpecType::SPECTROGRAM || type == SpecType::HPCP) {
    mSpecType.setSelectedId(mProcessType, juce::sendNotification);
  }
  startThread();
}

void ArcSpectrogram::setTransients(
    std::vector<TransientDetector::Transient>* transients) {
  mTransients = transients;
}

void ArcSpectrogram::setNoteOn(int midiNote,
    std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  mIsPlayingNote = true;
  mGPositions = gPositions;
  mCurNote = midiNote;
  mGPositions = gPositions;
  mPositionMarkers.clear();
  for (int i = 0; i < gPositions.size(); ++i) {
    auto newItem = new PositionMarker(gPositions[i], juce::Colour(MARKER_COLOURS[i]));
    newItem->addListener(this);
    mPositionMarkers.add(newItem);
    addAndMakeVisible(newItem);
  }
  resized();
}

void ArcSpectrogram::buttonClicked(juce::Button* btn) {
  if (onPositionUpdated == nullptr) return;
  for (int i = 0; i < mPositionMarkers.size(); ++i) {
    if (btn == mPositionMarkers[i]) {
      auto gPos = mGPositions[i];
      //gPos.isEnabled = btn->getToggleState();
      onPositionUpdated(mCurNote, gPos);
    }
  }
}