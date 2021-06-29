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
PositionMarker::PositionMarker(GrainPositionFinder::GrainPosition gPos,
                               juce::Colour colour)
    : mGPos(gPos), juce::Button(juce::String()), mColour(colour) {
  setToggleState(true, juce::dontSendNotification);
  setClickingTogglesState(true);
}

PositionMarker::~PositionMarker() {}

void PositionMarker::paintButton(juce::Graphics& g,
                                 bool shouldDrawButtonAsHighlighted,
                                 bool shouldDrawButtonAsDown) {
  auto r = getLocalBounds();

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
  g.fillPath(outlinePath);
}

void PositionMarker::clicked() {
  //mGPos.isEnabled = !mGPos.isEnabled;
  repaint();
}
