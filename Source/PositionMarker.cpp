/*
  ==============================================================================

    PositionItem.cpp
    Created: 29 Apr 2021 8:18:02pm
    Author:  brady

  ==============================================================================
*/

#include "PositionMarker.h"

#include <JuceHeader.h>

//==============================================================================
PositionMarker::PositionMarker(GrainPositionFinder::GrainPosition gPos, juce::Colour colour)
    : mGPos(gPos), juce::Button(juce::String()), mColour(colour) {
  setToggleState(gPos.isEnabled, juce::dontSendNotification);
  setClickingTogglesState(true);
}

PositionMarker::~PositionMarker() {}

void PositionMarker::paintButton(juce::Graphics& g,
                                 bool shouldDrawButtonAsHighlighted,
                                 bool shouldDrawButtonAsDown) {
  auto r = getLocalBounds();

  if (mGPos.isEnabled) {
    // Draw rect
    auto box = r.removeFromTop(r.getHeight() * RECT_RATIO);
    box.setLeft(2);
    box.setRight(getWidth() - 2);
    box.setTop(2);
    g.setColour(mColour);
    g.fillRect(box);

    // Draw point
    juce::Path pointPath;
    pointPath.startNewSubPath(r.getTopLeft().translated(2, 0).toFloat());
    pointPath.lineTo(r.getTopRight().translated(-2, 0).toFloat());
    pointPath.lineTo(r.getWidth() / 2, r.getBottom());
    pointPath.closeSubPath();
    g.fillPath(pointPath);
  }

  // Draw outline
  r = getLocalBounds();
  juce::Path outlinePath;
  outlinePath.startNewSubPath(r.getTopLeft().toFloat());
  outlinePath.lineTo(r.getTopRight().toFloat());
  outlinePath.lineTo(getWidth(), r.getHeight() * RECT_RATIO);
  outlinePath.lineTo(r.getWidth() / 2, r.getBottom());
  outlinePath.lineTo(0, r.getHeight() * RECT_RATIO);
  outlinePath.closeSubPath();
  g.setColour(mColour);
  g.strokePath(outlinePath, juce::PathStrokeType(
                                1, juce::PathStrokeType::JointStyle::mitered));
}

void PositionMarker::clicked() {
  mGPos.isEnabled = !mGPos.isEnabled;
  repaint();
}
