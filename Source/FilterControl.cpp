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

  // draw filter type rectangles
  float filterTypeHeight = 40;
  float filterTypeWidth = getWidth() / 3;
  float curStart = 1.0f;
  for (int i = 0; i < 3; i++) {
    juce::Colour tabColour = juce::Colours::darkgrey;
    /*float tabHeight =
        (mCurSelectedTab == i) ? getHeight() + 20.0f : getHeight() - 2.0f;*/
    juce::Rectangle<float> filterTypeRect =
        juce::Rectangle<float>(curStart, 1.0f, filterTypeWidth, filterTypeHeight);
   /* if (mCurHoverTab == i && mCurSelectedTab != i) {
      g.setColour(tabColour.withAlpha(0.3f));
      g.fillRoundedRectangle(tabRect, 10.0f);
    }*/
    g.setColour(tabColour);
    g.drawRoundedRectangle(filterTypeRect, 10.0f, 2.0f);

    g.setColour(juce::Colours::white);
    switch (i) {
      case 0:
        g.drawText(juce::String(Utils::FilterType::LOWPASS),
          filterTypeRect.withHeight(getHeight() - 2.0f),
          juce::Justification::centred);
        break;

      case 1:
        g.drawText(juce::String(Utils::FilterType::BANDPASS),
          filterTypeRect.withHeight(getHeight() - 2.0f),
          juce::Justification::centred);
        break;

      case 2:
        g.drawText(juce::String(Utils::FilterType::HIGHPASS),
          filterTypeRect.withHeight(getHeight() - 2.0f),
          juce::Justification::centred);
        break;
    }
    curStart += filterTypeWidth + 2.0f;
  }
}

void FilterControl::resized() {
  // This method is where you should set the bounds of any child
  // components that your component contains..
}

void FilterControl::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}
