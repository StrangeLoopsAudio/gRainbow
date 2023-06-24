/*
  ==============================================================================

    PositionChanger.cpp
    Created: 3 Jul 2021 10:08:25pm
    Author:  brady

  ==============================================================================
*/

#include "PositionChanger.h"
#include "Utils/Utils.h"

//==============================================================================
PositionChanger::PositionChanger() {}

PositionChanger::~PositionChanger() {}

void PositionChanger::paint(juce::Graphics& g) {
  juce::Colour bgColour = mIsActive ? mColour : juce::Colours::darkgrey;
  juce::Colour blackColour = mIsActive ? Utils::GLOBAL_COLOUR : juce::Colours::darkgrey;

  /* Draw left arrow */
  if (mIsOverLeftArrow) {
    g.setColour(bgColour);
    g.fillPath(mLeftPath);
  }
  g.setColour(bgColour);
  g.strokePath(mLeftPath, juce::PathStrokeType(2));
  g.setColour(blackColour);
  g.drawArrow(mLeftArrowLine, 2, 6, 6);

  /* Draw right arrow */
  if (mIsOverRightArrow) {
    g.setColour(bgColour);
    g.fillPath(mRightPath);
  }
  g.setColour(bgColour);
  g.strokePath(mRightPath, juce::PathStrokeType(2));
  g.setColour(blackColour);
  g.drawArrow(mRightArrowLine, 2, 6, 6);

  /* Draw top/bottom borders */
  g.setColour(bgColour);
  g.drawRect(mTitleRect, 2);

  /* Draw position text */
  juce::String posString;
  if (mIsActive && mPosition >= 0 && mNumPositions > 0) {
    int posNum = (mPosition >= 0) ? mPosition + 1 : 0;
    posString = juce::String(posNum) + juce::String(" / ") + juce::String(mNumPositions);
  } else {
    posString = "-";  // better then a blank boxs
  }
  g.setColour(blackColour);
  g.drawText(posString, mTitleRect, juce::Justification::centred);

  /* Solo button */
  if (mIsOverSolo || mIsSolo) {
    g.setColour(mIsSolo ? juce::Colours::blue : juce::Colours::blue.withAlpha(0.3f));
    g.fillRect(mSoloRect);
  }
  g.setColour(mIsActive ? juce::Colours::blue : juce::Colours::darkgrey);
  g.drawRect(mSoloRect, 2.0f);
  g.setColour(blackColour);
  g.drawFittedText("solo", mSoloRect.reduced(4).toNearestInt(), juce::Justification::centred, 1);
}

void PositionChanger::resized() {
  const int arrowWidth = getWidth() * ARROW_PERC;
  const float soloHeight = getHeight() * SOLO_HEIGHT_PERC;
  const float selectorHeight = getHeight() - soloHeight;

  juce::Rectangle<int> r = getLocalBounds();

  mTitleRect = r.withHeight(selectorHeight).withSizeKeepingCentre(getWidth() * TITLE_PERC, selectorHeight);
  mSoloRect = mTitleRect.translated(0, selectorHeight).withHeight(soloHeight - 2).toFloat();

  mLeftPath.clear();
  mLeftPath.addCentredArc(mTitleRect.getX(), selectorHeight / 2, arrowWidth - 1, selectorHeight / 2 - 1, 0, juce::MathConstants<float>::pi,
                          juce::MathConstants<float>::twoPi, true);
  mLeftPath.closeSubPath();
  mLeftArrowLine = juce::Line<float>(arrowWidth - (arrowWidth * 0.1), selectorHeight / 2, (arrowWidth * 0.3), selectorHeight / 2);

  const int rightStart = getWidth() * (ARROW_PERC + TITLE_PERC);
  mRightPath.clear();
  mRightPath.addCentredArc(mTitleRect.getRight(), selectorHeight / 2, arrowWidth - 1, selectorHeight / 2 - 1, 0, 0,
                           juce::MathConstants<float>::pi, true);
  mRightPath.closeSubPath();
  mRightArrowLine =
      juce::Line<float>(rightStart + (arrowWidth * 0.1), selectorHeight / 2, getWidth() - (arrowWidth * 0.3), selectorHeight / 2);
}

void PositionChanger::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void PositionChanger::setSolo(bool isSolo) {
  mIsSolo = isSolo;
  repaint();
}

void PositionChanger::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();
}

void PositionChanger::setNumPositions(int numPositions) {
  mNumPositions = numPositions;
  repaint();
}

void PositionChanger::mouseMove(const juce::MouseEvent& e) { updateMouseOver(e, false); }

void PositionChanger::mouseDrag(const juce::MouseEvent& e) { updateMouseOver(e, false); }

void PositionChanger::mouseDown(const juce::MouseEvent& e) { updateMouseOver(e, false); }

void PositionChanger::mouseUp(const juce::MouseEvent& e) { updateMouseOver(e, true); }
void PositionChanger::mouseEnter(const juce::MouseEvent& e) { updateMouseOver(e, false); }

void PositionChanger::mouseExit(const juce::MouseEvent& e) { updateMouseOver(e, false); }

void PositionChanger::updateMouseOver(const juce::MouseEvent& e, bool isClick) {
  if (!mIsActive) return;
  auto pos = e.getEventRelativeTo(this).position;
  if (isLeftArrow(pos)) {
    if (isClick) {
      positionChanged(false);
    }
    mIsOverLeftArrow = true;
  } else if (isRightArrow(pos)) {
    if (isClick) {
      positionChanged(true);
    }
    mIsOverRightArrow = true;
  } else {
    mIsOverLeftArrow = false;
    mIsOverRightArrow = false;
  }
  if (mSoloRect.contains(pos)) {
    mIsOverSolo = true;
    if (isClick) {
      mIsSolo = !mIsSolo;
      if (onSoloChanged != nullptr) onSoloChanged(mIsSolo);
    }
  } else {
    mIsOverSolo = false;
  }
  repaint();
}

bool PositionChanger::isLeftArrow(juce::Point<float> point) {
  juce::Rectangle<int> arrowRect =
      getLocalBounds().removeFromLeft(getWidth() * ARROW_PERC).withHeight(getHeight() * (1.0f - SOLO_HEIGHT_PERC));
  return arrowRect.contains(point.toInt());
}

bool PositionChanger::isRightArrow(juce::Point<float> point) {
  juce::Rectangle<int> arrowRect =
      getLocalBounds().removeFromRight(getWidth() * ARROW_PERC).withHeight(getHeight() * (1.0f - SOLO_HEIGHT_PERC));
  return arrowRect.contains(point.toInt());
}

void PositionChanger::positionChanged(bool isRight) {
  if (onPositionChanged != nullptr) {
    onPositionChanged(isRight);
  }
}

void PositionChanger::setPositionNumber(int position) {
  mPosition = position;
  repaint();
}