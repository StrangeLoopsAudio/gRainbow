/*
  ==============================================================================

    PositionChanger.cpp
    Created: 3 Jul 2021 10:08:25pm
    Author:  brady

  ==============================================================================
*/

#include "PositionChanger.h"

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

  /* Draw title rect */
  juce::Rectangle<int> titleRect = getLocalBounds().withSizeKeepingCentre(
      getWidth() * TITLE_PERC, getHeight());
  g.setColour(bgColour);
  g.fillRect(titleRect);

  /* Draw title text */
  g.setColour(juce::Colours::black);
  g.setFont(14.0f);
  juce::String posString = juce::String("Position #") + juce::String(mPosition);
  g.drawText(posString, titleRect, juce::Justification::centred, true);
}

void PositionChanger::resized() {}

void PositionChanger::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void PositionChanger::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();
}

void PositionChanger::setPosition(int position) {
  mPosition = position;
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