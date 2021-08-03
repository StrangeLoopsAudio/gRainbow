/*
  ==============================================================================

    FilterControl.cpp
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FilterControl.h"

//==============================================================================
FilterControl::FilterControl() {
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
}

FilterControl::~FilterControl() {}

void FilterControl::paint(juce::Graphics& g) {
  juce::Colour envColour = mIsActive
                               ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                               : juce::Colours::darkgrey;
  g.fillAll(getLookAndFeel().findColour(
      juce::ResizableWindow::backgroundColourId));  // clear the background

  g.setColour(juce::Colours::grey);
  g.drawRect(getLocalBounds(), 1);  // draw an outline around the component

  g.setColour(juce::Colours::white);
  g.setFont(14.0f);
  g.drawText("FilterControl", getLocalBounds(), juce::Justification::centred,
             true);  // draw some placeholder text
}

void FilterControl::resized() {
  // This method is where you should set the bounds of any child
  // components that your component contains..
}

void FilterControl::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}
