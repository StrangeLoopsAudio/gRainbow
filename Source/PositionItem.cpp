/*
  ==============================================================================

    PositionItem.cpp
    Created: 29 Apr 2021 8:18:02pm
    Author:  brady

  ==============================================================================
*/

#include "PositionItem.h"

#include <JuceHeader.h>

//==============================================================================
PositionItem::PositionItem(GrainPositionFinder::GrainPosition gPos) : mGPos(gPos) {
  auto rawString = "Ratio: " +
                   juce::String::toDecimalStringWithSignificantFigures(mGPos.posRatio, 2) +
                   ",  Gain: " +
                   juce::String::toDecimalStringWithSignificantFigures(mGPos.gain, 2) +
                   ", Rate: " +
                   juce::String::toDecimalStringWithSignificantFigures(mGPos.pbRate, 2);
  mLabelDesc.setText(rawString, juce::dontSendNotification);
  addAndMakeVisible(mLabelDesc);

  btnEnabled.setToggleState(gPos.isEnabled, juce::dontSendNotification);
  mLabelEnabled.setText("Enabled", juce::dontSendNotification);
  addAndMakeVisible(mLabelEnabled);
  mLabelEnabled.attachToComponent(&btnEnabled, true);
  addAndMakeVisible(btnEnabled);
  btnSolo.setToggleState(gPos.solo, juce::dontSendNotification);
  mLabelSolo.setText("Solo", juce::dontSendNotification);
  addAndMakeVisible(mLabelSolo);
  mLabelSolo.attachToComponent(&btnSolo, true);
  addAndMakeVisible(btnSolo);
}

PositionItem::~PositionItem() {}

void PositionItem::addListener(juce::Button::Listener* listener) {
  btnEnabled.addListener(listener);
  btnSolo.addListener(listener);
}

void PositionItem::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(
      juce::ResizableWindow::backgroundColourId));  // clear the background
}

void PositionItem::resized() {
  auto r = getLocalBounds();
  int toggleWidth = getWidth() / 3;
  auto toggleRect = r.removeFromLeft(toggleWidth);
  toggleRect.removeFromRight(10); // Remove padding space
  auto enabledRect = toggleRect.removeFromTop(toggleRect.getHeight() / 2);
  btnEnabled.setBounds(enabledRect.removeFromRight(enabledRect.getHeight()));
  btnSolo.setBounds(toggleRect.removeFromRight(toggleRect.getHeight()));
  mLabelDesc.setBounds(r);


}
