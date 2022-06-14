/*
  ==============================================================================

    TrimSelection.cpp
    Created: 12 Jun 2022 3:05:13pm
    Author:  fricke

  ==============================================================================
*/

#include "TrimSelection.h"

PointMarker::PointMarker(juce::String name, juce::Colour colour, MouseCallback onMouseDrag, MouseCallback onMouseUp)
    : mName(std::move(name)), mColour(colour), mOnMouseDrag(std::move(onMouseDrag)), mOnMouseUp(std::move(onMouseUp)) {
  setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
}

void PointMarker::resized() {
  juce::Rectangle<int> bounds = getLocalBounds();
  juce::Path path;
  path.addRectangle(bounds.removeFromBottom(HEIGHT));

  path.startNewSubPath(bounds.getBottomLeft().toFloat());
  path.lineTo(bounds.getBottomRight().toFloat());
  juce::Point<float> apex(static_cast<float>(bounds.getX() + (bounds.getWidth() / 2)),
                          static_cast<float>(bounds.getBottom() - APEX_HEIGHT));
  path.lineTo(apex);
  path.closeSubPath();

  path.addLineSegment(juce::Line<float>(apex, juce::Point<float>(apex.getX(), 0)), 1);
  mPath = path;
}

void PointMarker::paint(juce::Graphics& g) {
  g.setColour(mColour);
  g.fillPath(mPath);

  auto height = 10;
  g.setColour(juce::Colours::white);
  g.drawText(mName, getLocalBounds().removeFromBottom(height), juce::Justification::centred);
}

bool PointMarker::hitTest(int x, int y) { return mPath.contains(static_cast<float>(x), static_cast<float>(y)); }

TrimSelection::TrimSelection(juce::AudioFormatManager& formatManager, ParamUI& paramUI)
    : mThumbnailCache(THUMBNAIL_CACHE_SIZE),
      mThumbnail(THUMBNAIL_CACHE_SIZE, formatManager, mThumbnailCache),
      mParamUI(paramUI),
      mStartMarker(
          "S", juce::Colours::green, [this](PointMarker& m, const juce::MouseEvent& e) { this->MarkerMouseDragged(m, e); },
          [this](PointMarker& m, const juce::MouseEvent& e) { this->MarkerMouseUp(m, e); }),
      mEndMarker(
          "E", juce::Colours::red, [this](PointMarker& m, const juce::MouseEvent& e) { this->MarkerMouseDragged(m, e); },
          [this](PointMarker& m, const juce::MouseEvent& e) { this->MarkerMouseUp(m, e); }) {
  mBtnCancel.setButtonText("Cancel");
  mBtnCancel.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
  mBtnCancel.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
  mBtnCancel.onClick = [this] { onCancel(); };
  addAndMakeVisible(mBtnCancel);

  mBtnTestSelection.setButtonText("Test Selection");
  mBtnTestSelection.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
  mBtnTestSelection.setColour(juce::TextButton::buttonOnColourId, juce::Colours::blue);
  mBtnTestSelection.onClick = [this] {
    // TOOD - Enable
    // onProcessSelection(mSelectedRange, false);
  };
  addAndMakeVisible(mBtnTestSelection);

  mBtnSetSelection.setButtonText("Set Selection");
  mBtnSetSelection.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
  mBtnSetSelection.setColour(juce::TextButton::buttonOnColourId, juce::Colours::blue);
  mBtnSetSelection.onClick = [this] { onProcessSelection(mSelectedRange, true); };
  addAndMakeVisible(mBtnSetSelection);

  addAndMakeVisible(mStartMarker);
  addAndMakeVisible(mEndMarker);
}

TrimSelection::~TrimSelection() {}

void TrimSelection::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  int numChannels = mThumbnail.getNumChannels();
  if (numChannels == 0) {
    // If trying to trim a file and the editor is closed emulate pressing the cancel button
    onCancel();
    return;
  }

  // Draw thumbnail
  {
    const int channelHeight = mThumbnailRect.getHeight() / numChannels;
    // draw an outline around the thumbnail
    g.setColour(juce::Colours::grey);
    g.drawRect(mThumbnailRect, 1);

    for (int i = 0; i < numChannels; i++) {
      juce::Rectangle<int> channelBounds = mThumbnailRect.withTrimmedTop(channelHeight * i).withHeight(channelHeight);

      g.setGradientFill(juce::ColourGradient(juce::Colours::lightblue, channelBounds.getTopLeft().toFloat(),
                                             juce::Colours::darkgrey, channelBounds.getBottomLeft().toFloat(), false));
      mThumbnail.drawChannel(g, channelBounds, mVisibleRange.getStart(), mVisibleRange.getEnd(), i, 1.0f);
    }
  }

  // Draw selectors
  {
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawFittedText(mStartTimeString, mSelectorRect, juce::Justification::bottomLeft, 1);
    g.drawFittedText(mEndTimeString, mSelectorRect, juce::Justification::bottomRight, 1);
  }

  // Update text of test results
  {
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawFittedText("Test Selection to be implemented", mTestResultRect, juce::Justification::centred, 1);
  }
}

void TrimSelection::parse(juce::AudioFormatReader* formatReader, juce::int64 hash) {
  mThumbnail.setReader(formatReader, hash);

  mVisibleRange.setStart(0.0f);
  mVisibleRange.setEnd(static_cast<double>(formatReader->lengthInSamples) / formatReader->sampleRate);
  // start with everything selected
  mSelectedRange = mVisibleRange;
  updatePointMarker();
}

void TrimSelection::resized() {
  // Layout of component
  // - Thumbnail
  // - Selector
  // - buttons
  // - test result text

  juce::Rectangle<int> r = getLocalBounds();
  // just enough padding to not be on the side borders
  r.reduce(r.getWidth() * 0.02, 0);
  mThumbnailRect = r.removeFromTop((2 * r.getHeight()) / 3);
  // extra 20 comes from 14 point font for text and some padding
  mSelectorRect = mThumbnailRect.removeFromBottom(PointMarker::height() + 20);

  const int btnHeight = r.getHeight() * 0.3;
  const int btnHeightPadding = r.getHeight() * 0.1;
  const int btnWidth = r.getWidth() * 0.25;
  const int btnWidthPadding = r.getWidth() * (.25 / 4.0);

  juce::Rectangle<int> btnRect = r.removeFromTop(btnHeight);
  mTestResultRect = r;

  btnRect.removeFromTop(btnHeightPadding);
  btnRect.removeFromLeft(btnWidthPadding);
  mBtnCancel.setBounds(btnRect.removeFromLeft(btnWidth));
  btnRect.removeFromLeft(btnWidthPadding);
  mBtnTestSelection.setBounds(btnRect.removeFromLeft(btnWidth));
  btnRect.removeFromLeft(btnWidthPadding);
  btnRect.removeFromRight(btnWidthPadding);
  mBtnSetSelection.setBounds(btnRect.removeFromLeft(btnWidth));
}

void TrimSelection::updatePointMarker() {
  const int halfWidth = 7;
  const int startX = juce::roundToInt(timeToXPosition(mSelectedRange.getStart()) - halfWidth);
  const int endX = juce::roundToInt(timeToXPosition(mSelectedRange.getEnd()) - halfWidth);
  // PointMarkers start at the top to draw the line over the thumbnail
  const int y = mThumbnailRect.getY();
  const int width = halfWidth * 2;
  // The PointerMarker height is the triangle height under the thumbnail
  const int height = mThumbnailRect.getHeight() + PointMarker::height();
  // prevent from going outside thumbnail bounds
  const int minX = mThumbnailRect.getX() - halfWidth;
  const int maxX = mThumbnailRect.getRight() - halfWidth;
  mStartMarker.setBounds(juce::jmax(startX, minX), y, width, height);
  mEndMarker.setBounds(juce::jmin(endX, maxX), y, width, height);

  const int startTotal = static_cast<int>(mSelectedRange.getStart());
  const int startMinute = startTotal / 60;
  const int startSecond = startTotal % 60;
  const int endTotal = static_cast<int>(mSelectedRange.getEnd());
  const int endMinute = endTotal / 60;
  const int endSecond = endTotal % 60;
  mStartTimeString = juce::String::formatted("%02d:%02d", startMinute, startSecond);
  mEndTimeString = juce::String::formatted("%02d:%02d", endMinute, endSecond);
}

double TrimSelection::timeToXPosition(double time) const {
  const double start = time - mVisibleRange.getStart();
  const double width = mThumbnailRect.getWidth();
  const double length = mVisibleRange.getLength();
  const double offset = mThumbnailRect.getX();
  return ((start * width) / length) + offset;
}

double TrimSelection::xPositionToTime(double xPosition) const {
  // xPosition is overall component, need to adjust to thumbnail bounds
  const double x = (xPosition - mThumbnailRect.getX()) * mVisibleRange.getLength();
  const double width = mThumbnailRect.getWidth();
  const double start = mVisibleRange.getStart();
  return (x / width) + start;
}

void TrimSelection::MarkerMouseDragged(PointMarker& marker, const juce::MouseEvent& e) {
  const double x = xPositionToTime(e.getEventRelativeTo(this).position.x);
  if (&marker == &mStartMarker) {
    mSelectedRange.setStart(juce::jmax(x, mVisibleRange.getStart()));
  } else {
    mSelectedRange.setEnd(juce::jmin(x, mVisibleRange.getEnd()));
  }
  updatePointMarker();
}

void TrimSelection::MarkerMouseUp(PointMarker& marker, const juce::MouseEvent& e) {
  const double x = xPositionToTime(e.getEventRelativeTo(this).position.x);
  if (&marker == &mStartMarker) {
    mSelectedRange.setStart(juce::jmax(x, mVisibleRange.getStart()));
  } else {
    mSelectedRange.setEnd(juce::jmin(x, mVisibleRange.getEnd()));
  }
  updatePointMarker();
}
