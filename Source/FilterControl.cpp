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
  //mColour = juce::Colours::blue.brighter().brighter();
  juce::Colour envColour = mIsActive ? mColour : juce::Colours::darkgrey;
 /* g.fillAll(getLookAndFeel().findColour(
      juce::ResizableWindow::backgroundColourId));  // clear the background */

  g.setColour(juce::Colours::grey);
  g.drawRect(getLocalBounds(), 1);  // draw an outline around the component

  g.setColour(juce::Colours::white);
  g.setFont(14.0f);

  // draw filter type rectangles

  float filterTypeWidth = getWidth() / 3 - 1.0f;
  float curStart = 1.0f;
  for (int i = 0; i < 3; i++) {
    juce::Colour tabColour = juce::Colours::darkgrey;
    /*float tabHeight =
        (mCurSelectedTab == i) ? getHeight() + 20.0f : getHeight() - 2.0f;*/
    juce::Rectangle<float> filterTypeRect =
        juce::Rectangle<float>(curStart, 1.0f, filterTypeWidth, FILTER_TYPE_BUTTON_HEIGHT);
   /* if (mCurHoverTab == i && mCurSelectedTab != i) {
      g.setColour(tabColour.withAlpha(0.3f));
      g.fillRoundedRectangle(tabRect, 10.0f);
    }*/
    g.setColour(tabColour);
    g.drawRoundedRectangle(filterTypeRect, 10.0f, 2.0f);

    g.setColour(juce::Colours::white);
    g.drawText(FILTER_TYPE_NAMES[i+1], filterTypeRect.withHeight(FILTER_TYPE_BUTTON_HEIGHT),
      juce::Justification::centred);
    
    curStart += filterTypeWidth + 1.0f;
  }

  g.setFillType(
      juce::ColourGradient(envColour, getLocalBounds().getTopLeft().toFloat(),
                           envColour.withAlpha(0.4f),
                           getLocalBounds().getBottomLeft().toFloat(), false));
  // Draw Filter path

  // low pass filter only for now
  filterPath.startNewSubPath(juce::Point<float>(0, FILTER_TYPE_BUTTON_HEIGHT + 10));
  filterPath.lineTo(juce::Point<float>(getWidth() * mCutoff, FILTER_TYPE_BUTTON_HEIGHT + 10));
 /* filterPath.lineTo(filterPath.getCurrentPosition()
                      .translated((1.0 - mStrength) * 0.5f * getWidth(), 0.0f)
                      .withY(getHeight()));
  filterPath.lineTo(juce::Point<float>(0, getHeight()));
  filterPath.lineTo(juce::Point<float>(0, 0));*/
  filterPath.closeSubPath();
  //g.fillPath(filterPath);

  // Draw highlights on top of path
  float highlightWidth = 3.0f;

  g.setColour(mIsActive ? envColour.brighter() : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(0, FILTER_TYPE_BUTTON_HEIGHT + 10, 
                               mCutoff * getWidth(), FILTER_TYPE_BUTTON_HEIGHT + 10), 
                               highlightWidth);

  g.setColour(mIsActive ? envColour.brighter().brighter()
                        : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(mCutoff * getWidth(), FILTER_TYPE_BUTTON_HEIGHT + 10,
                               (getWidth() * mCutoff) + (1.0f - mStrength) * 0.5f * getWidth(), getHeight() * 1.0f),
                               highlightWidth);
}

void FilterControl::resized() {
}

void FilterControl::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void FilterControl::setCutoff(float cutoff) { 
  mCutoff = cutoff; 
  repaint();
}

void FilterControl::setStrength(float strength) {
  mStrength = strength; 
  repaint();
}

void FilterControl::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();

}void FilterControl::setFilterType(Utils::FilterType filterType) {
  mFilterType = filterType;
  repaint();
}