/*
  ==============================================================================

    PositionVisualizer.cpp
    Created: 27 Apr 2021 9:17:07pm
    Author:  brady

    Visualizes grain positions for granular playback

  ==============================================================================
*/

#include "PositionVisualizer.h"

#include <JuceHeader.h>

//==============================================================================
PositionVisualizer::PositionVisualizer() {
  setFramesPerSecond(10); 
}

PositionVisualizer::~PositionVisualizer() { }

void PositionVisualizer::setPositions(
    std::vector<GranularSynth::GrainPosition> gPositions) {
  mGPositions = gPositions;
}

void PositionVisualizer::paint(juce::Graphics& g) {

  g.fillAll(getLookAndFeel().findColour(
      juce::ResizableWindow::backgroundColourId));  // clear the background

  auto r = getLocalBounds();
  g.setColour(juce::Colours::white);
  for (int i = 0; i < mGPositions.size(); ++i) {
    if (r.getHeight() > ITEM_HEIGHT) {
      auto rawString = "Ratio: " +
                       juce::String::toDecimalStringWithSignificantFigures(
                           mGPositions[i].posRatio, 2) +
                       ",  Gain: " +
                       juce::String::toDecimalStringWithSignificantFigures(
                           mGPositions[i].gain, 2) +
                       ", rate: " +
                       juce::String::toDecimalStringWithSignificantFigures(
                           mGPositions[i].pbRate, 2);
      g.drawFittedText(rawString, r.removeFromTop(ITEM_HEIGHT),
                       juce::Justification::left, 1);
    }
  }

}

void PositionVisualizer::resized() {
}