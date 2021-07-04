/*
  ==============================================================================

    PositionChanger.cpp
    Created: 3 Jul 2021 10:08:25pm
    Author:  brady

  ==============================================================================
*/

#include "PositionChanger.h"
#include "GranularSynth.h"
#include <JuceHeader.h>

//==============================================================================
PositionChanger::PositionChanger() {
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
}

PositionChanger::~PositionChanger() {}

void PositionChanger::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);  // clear the background
  juce::Colour bgColour = mIsActive ? mColour : juce::Colours::darkgrey;

  int arrowWidth = getWidth() * ARROW_PERC;
  /* Draw left arrow */
  juce::Path leftPath;
  leftPath.addEllipse(1, 1, (arrowWidth * 2) - 1, getHeight() - 2);
  if (mIsOverLeftArrow) {
    juce::Colour fillColour =
        mIsClickingArrow ? bgColour
                         : bgColour.interpolatedWith(juce::Colours::black, 0.8);
    g.setColour(fillColour);
    g.fillPath(leftPath);
  }
  g.setColour(bgColour);
  g.strokePath(leftPath, juce::PathStrokeType(2));
  g.drawArrow(
      juce::Line<float>(arrowWidth - (arrowWidth * 0.1), getHeight() / 2,
                        (arrowWidth * 0.3), getHeight() / 2),
      2, 6, 6);

  /* Draw right arrow */
  int rightStart = getWidth() * (ARROW_PERC + TITLE_PERC);
  juce::Path rightPath;
  rightPath.addEllipse(rightStart - arrowWidth, 1, (arrowWidth * 2) - 1,
                       getHeight() - 2);
  if (mIsOverRightArrow) {
    juce::Colour fillColour =
        mIsClickingArrow ? bgColour
                         : bgColour.interpolatedWith(juce::Colours::black, 0.8);
    g.setColour(fillColour);
    g.fillPath(rightPath);
  }
  g.setColour(bgColour);
  g.strokePath(rightPath, juce::PathStrokeType(2));
  g.drawArrow(
      juce::Line<float>(rightStart + (arrowWidth * 0.1), getHeight() / 2,
                        getWidth() - (arrowWidth * 0.3), getHeight() / 2),
      2, 6, 6);

  juce::Rectangle<int> titleRect = getLocalBounds().withSizeKeepingCentre(
      getWidth() * TITLE_PERC, getHeight());

  /* Fill in title section to mask ellipses */
  g.setColour(juce::Colours::black);
  g.fillRect(titleRect);

  /* Draw top/bottom borders */
  g.setColour(bgColour);
  g.drawRect(titleRect, 2);

  /* Draw position bubbles */
  if (mGlobalPositions.size() > 0) {
    int totalWidth =
        (BUBBLE_WIDTH * mNumPositions) + (BUBBLE_PADDING * (mNumPositions - 1));
    int startX = (getWidth() / 2) - (totalWidth / 2);
    for (int i = 0; i < mNumPositions; ++i) {
      int bubbleStart = startX + i * (BUBBLE_WIDTH + BUBBLE_PADDING);
      juce::Rectangle<float> bubbleRect =
          juce::Rectangle<float>(BUBBLE_WIDTH, BUBBLE_WIDTH);
      bubbleRect = bubbleRect.withCentre(juce::Point<float>(
          bubbleStart + (BUBBLE_WIDTH / 2), getHeight() / 2));
      
      if (i == mGlobalPositions[mIndexInBoxes]) {
        g.setColour(mColour);
        g.fillEllipse(bubbleRect);
      } else {
        bool isTaken =
            std::find(mGlobalPositions.begin(), mGlobalPositions.end(), i) !=
            mGlobalPositions.end();
        g.setColour(juce::Colours::darkgrey);
        if (isTaken) g.fillEllipse(bubbleRect);
        else g.drawEllipse(bubbleRect, 1);
      }
    }
  }
}

void PositionChanger::resized() {}

void PositionChanger::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void PositionChanger::setColour(int indexInBoxes, juce::Colour colour) {
  mIndexInBoxes = indexInBoxes;
  mColour = colour;
  repaint();
}

void PositionChanger::mouseMove(const juce::MouseEvent& e) {
  updateMouseOver(e, false);
}

void PositionChanger::mouseDrag(const juce::MouseEvent& e) {
  updateMouseOver(e, true);
}

void PositionChanger::mouseDown(const juce::MouseEvent& e) {
  updateMouseOver(e, true);
}

void PositionChanger::mouseUp(const juce::MouseEvent& e) {
  updateMouseOver(e, false);
}
void PositionChanger::mouseEnter(const juce::MouseEvent& e) {
  updateMouseOver(e, false);
}

void PositionChanger::mouseExit(const juce::MouseEvent& e) {
  updateMouseOver(e, false);
}

void PositionChanger::updateMouseOver(const juce::MouseEvent& e, bool isDown) {
  auto pos = e.getEventRelativeTo(this).position;
  if (isLeftArrow(pos)) {
    if (!mIsClickingArrow && isDown) {
      positionChanged(false);
    }
    mIsOverLeftArrow = true;
  } else if (isRightArrow(pos)) {
    if (!mIsClickingArrow && isDown) {
      positionChanged(true);
    }
    mIsOverRightArrow = true;
  } else {
    mIsOverLeftArrow = false;
    mIsOverRightArrow = false;
  }
  mIsClickingArrow = isDown;
  repaint();
}

bool PositionChanger::isLeftArrow(juce::Point<float> point) {
  juce::Rectangle<int> arrowRect =
      getLocalBounds().removeFromLeft(getWidth() * ARROW_PERC);
  return arrowRect.contains(point.toInt());
}

bool PositionChanger::isRightArrow(juce::Point<float> point) {
  juce::Rectangle<int> arrowRect =
      getLocalBounds().removeFromRight(getWidth() * ARROW_PERC);
  return arrowRect.contains(point.toInt());
}

void PositionChanger::positionChanged(bool isRight) {
  if (onPositionChanged != nullptr) {
    onPositionChanged(isRight);
  }
}

void PositionChanger::setGlobalPositions(std::vector<int> positions) {
  mGlobalPositions = positions;
  repaint();
}

void PositionChanger::setNumPositions(int numPositions) {
  mNumPositions = numPositions;
  repaint();
}