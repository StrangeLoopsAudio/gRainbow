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

  g.setFont(14.0f);

  // draw filter type rectangles

  float totalButtonWidth = getWidth() - 4 * PADDING_SIZE;
  float filterTypeWidth = totalButtonWidth / 3;

  //juce::Colour tabColour = juce::Colours::darkgrey;
    /*float tabHeight =
        (mCurSelectedTab == i) ? getHeight() + 20.0f : getHeight() - 2.0f;*/

  mLowPassRect = juce::Rectangle<float>(PADDING_SIZE, 1.0f, filterTypeWidth, FILTER_TYPE_BUTTON_HEIGHT);
  g.setColour(envColour.brighter());
  g.drawRoundedRectangle(mLowPassRect, 10.0f, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(FILTER_TYPE_NAMES[1], mLowPassRect.withHeight(FILTER_TYPE_BUTTON_HEIGHT), juce::Justification::centred);

  mHighPassRect = juce::Rectangle<float>(2 * PADDING_SIZE + filterTypeWidth, 1.0f, filterTypeWidth, FILTER_TYPE_BUTTON_HEIGHT);
  g.setColour(envColour.brighter());
  g.drawRoundedRectangle(mHighPassRect, 10.0f, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(FILTER_TYPE_NAMES[2], mHighPassRect.withHeight(FILTER_TYPE_BUTTON_HEIGHT), juce::Justification::centred);

  mBandPassRect = juce::Rectangle<float>(3 * PADDING_SIZE + 2 * filterTypeWidth, 1.0f, filterTypeWidth, FILTER_TYPE_BUTTON_HEIGHT);
  g.setColour(envColour.brighter());
  g.drawRoundedRectangle(mBandPassRect, 10.0f, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(FILTER_TYPE_NAMES[3], mBandPassRect.withHeight(FILTER_TYPE_BUTTON_HEIGHT), juce::Justification::centred);

  switch (mCurHoverFilterType) { 
    
    case (Utils::FilterType::LOWPASS):
      g.setColour(envColour.withAlpha(0.3f));
      g.fillRoundedRectangle(mLowPassRect, 10.0f);
      break;
    case (Utils::FilterType::HIGHPASS):
      g.setColour(envColour.withAlpha(0.3f));
      g.fillRoundedRectangle(mHighPassRect, 10.0f);
      break;
    case (Utils::FilterType::BANDPASS):
      g.setColour(envColour.withAlpha(0.3f));
      g.fillRoundedRectangle(mBandPassRect, 10.0f);
      break;

  }
  // Draw Filter path
  g.setFillType(juce::ColourGradient(envColour, getLocalBounds().getTopLeft().toFloat(), envColour.withAlpha(0.4f),
                                     getLocalBounds().getBottomLeft().toFloat(), false));

  juce::Path mFilterPath;
  mFilterPath.startNewSubPath(juce::Point<float>(0, FILTER_TYPE_BUTTON_HEIGHT + 2 * PADDING_SIZE));
  mFilterPath.lineTo(juce::Point<float>(getWidth() * mCutoff, FILTER_TYPE_BUTTON_HEIGHT + 2 * PADDING_SIZE));
  mFilterPath.lineTo(mFilterPath.getCurrentPosition()
                      .translated((1.0 - mResonance) * 0.5f * getWidth(), 0.0f)
                      .withY(getHeight()));
  mFilterPath.lineTo(juce::Point<float>(0, getHeight()));
  mFilterPath.lineTo(juce::Point<float>(0, 0));
  mFilterPath.closeSubPath();
  g.fillPath(mFilterPath);

  // Draw highlights on top of path
  float highlightWidth = 3.0f;

  g.setColour(mIsActive ? envColour.brighter().brighter() : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(0, FILTER_TYPE_BUTTON_HEIGHT + 2 * PADDING_SIZE, 
                               mCutoff * getWidth(),
                               FILTER_TYPE_BUTTON_HEIGHT + 2 * PADDING_SIZE), 
                               highlightWidth);

  g.setColour(mIsActive ? envColour.brighter().brighter()
                        : juce::Colours::darkgrey);
  g.drawLine(juce::Line<float>(mCutoff * getWidth(), FILTER_TYPE_BUTTON_HEIGHT + 2 * PADDING_SIZE,
                               (getWidth() * mCutoff) + (1.0f - mResonance) * 0.5f * getWidth(), getHeight() * 1.0f),
                               highlightWidth);
}

void FilterControl::resized() {
}

void FilterControl::mouseMove(const juce::MouseEvent& event) { 
  if ((event.y > FILTER_TYPE_BUTTON_HEIGHT + PADDING_SIZE) || (event.x < PADDING_SIZE) || (event.x > getWidth() - PADDING_SIZE)) {
    mCurHoverFilterType = Utils::NO_FILTER;
  } else {
    mFilter = ((event.getEventRelativeTo(this).getPosition().getX() / (float)getWidth()) * 3 + 1);
    mCurHoverFilterType = static_cast<Utils::FilterType>(mFilter);
  }
  repaint(); 
}
void FilterControl::mouseExit(const juce::MouseEvent& event) { 

  repaint(); 
}
void FilterControl::mouseUp(const juce::MouseEvent& event) { 

  repaint(); 
}

void FilterControl::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void FilterControl::setCutoff(float cutoff) { 
  mCutoff = cutoff; 
  repaint();
}

void FilterControl::setResonance(float Resonance) {
  mResonance = Resonance; 
  repaint();
}

void FilterControl::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();

}void FilterControl::setFilterType(Utils::FilterType filterType) {
  mFilterType = filterType;
  repaint();
}