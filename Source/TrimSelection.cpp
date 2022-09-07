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

  g.setColour(juce::Colours::white);
  g.drawText(mName, getLocalBounds().removeFromBottom(HEIGHT), juce::Justification::centred);
}

bool PointMarker::hitTest(int x, int y) { return mPath.contains(static_cast<float>(x), static_cast<float>(y)); }

TrimSelection::TrimSelection(juce::AudioFormatManager& formatManager, ParamUI& paramUI)
    : mThumbnailCache(1), // need a cache if not used, this is smallest possible
      mThumbnail(512, formatManager, mThumbnailCache),
      mThumbnailShadow([this](const juce::MouseEvent& e) { this->ThumbnailMouseDown(e); },
                       [this](const juce::MouseEvent& e) { this->ThumbnailMouseDrag(e); },
                       [this](const juce::MouseEvent& e) { this->ThumbnailMouseUp(e); }),
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
  mBtnCancel.onClick = [this] {
    onCancel();
    cleanup();
  };
  addAndMakeVisible(mBtnCancel);

  mBtnPlayback.setButtonText("Play/Stop");
  mBtnPlayback.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
  mBtnPlayback.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
  mBtnPlayback.setClickingTogglesState(true);
  mBtnPlayback.onClick = [this] {
    mParamUI.trimPlaybackOn = !mParamUI.trimPlaybackOn;
    if (mParamUI.trimPlaybackOn) {
      mParamUI.trimPlaybackSample = timeToSample(mSelectedRange.getStart());
    }
  };
  addAndMakeVisible(mBtnPlayback);

  mBtnSetSelection.setButtonText("Set Selection");
  mBtnSetSelection.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
  mBtnSetSelection.setColour(juce::TextButton::buttonOnColourId, juce::Colours::blue);
  mBtnSetSelection.onClick = [this] {
    onProcessSelection(mSelectedRange);
    cleanup();
  };
  addAndMakeVisible(mBtnSetSelection);

  addAndMakeVisible(mStartMarker);
  addAndMakeVisible(mEndMarker);
  addAndMakeVisible(mThumbnailShadow);

  mPlaybackMarker.setFill(juce::Colours::white.withAlpha(0.9f));
  addAndMakeVisible(mPlaybackMarker);
}

TrimSelection::~TrimSelection() { cleanup(); }

void TrimSelection::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  int numChannels = mThumbnail.getNumChannels();
  if (numChannels == 0) {
    // If trying to trim a file and the editor is closed emulate pressing the cancel button
    onCancel();
    return;
  }

  // Draw thumbnail
  const int thumbnailHeight = mThumbnailRect.getHeight();
  {
    const int channelHeight = thumbnailHeight / numChannels;
    // draw an outline around the thumbnail
    g.setColour(juce::Colours::grey);
    g.drawRect(mThumbnailRect, 1);

    for (int i = 0; i < numChannels; i++) {
      juce::Rectangle<int> channelBounds = mThumbnailRect.withTrimmedTop(channelHeight * i).withHeight(channelHeight);

      g.setGradientFill(juce::ColourGradient(juce::Colours::lightblue, channelBounds.getTopLeft().toFloat(),
                                             juce::Colours::darkgrey, channelBounds.getBottomLeft().toFloat(), false));
      mThumbnail.drawChannel(g, channelBounds, 0.0f, mVisibleRange.getEnd(), i, 1.0f);
    }
  }

  // Darken areas outside of selectors
  {
    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    g.fillRect(mThumbnailRect.withWidth(timeToXPosition(mSelectedRange.getStart()) - mThumbnailRect.getX()));
    int endX = timeToXPosition(mSelectedRange.getEnd());
    g.fillRect(endX, mThumbnailRect.getY(), mThumbnailRect.getRight() - endX, thumbnailHeight);
  }

  // Draw selectors
  {
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawFittedText(mStartTimeString, mSelectorRect, juce::Justification::bottomLeft, 1);
    g.drawFittedText(juce::String(mSelectedRange.getLength(), 1) + " seconds", mSelectorRect, juce::Justification::centredBottom,
                     1);
    g.drawFittedText(mEndTimeString, mSelectorRect, juce::Justification::bottomRight, 1);
  }

  // Draw line showing where audio is playing from
  {
    const float xPosition = sampleToXPosition(mParamUI.trimPlaybackSample);
    mPlaybackMarker.setRectangle(juce::Rectangle<float>(xPosition, 0, 1.5f, thumbnailHeight));
  }
}

void TrimSelection::parse(const juce::AudioBuffer<float>& audioBuffer, double sampleRate, juce::String& error) {
  cleanup();  // in case while trim selecting, user selects a new file
  const double duration = static_cast<double>(audioBuffer.getNumSamples()) / sampleRate;
  if (duration <= MIN_SELECTION_SEC) {
    error =
        juce::String::formatted("The audio file is  %.1f seconds but must be greater than %d seconds", duration, MIN_SELECTION_SEC);
    // If another valid file was opened, cancel as that selection will now fail as the formatReader is bad now
    onCancel();
    return;
  }

  // It is slightly faster to use the setReader() with the AudioFormatReader for the thumbnail and make use of
  // juce::AudioThumbnailCache, but the main reasons to add the thumbnail block manually are
  // 1. The file might be at a different sampleRate than the audio buffer which can seem like it doesn't match the playback
  // 2. It is possible to update the thumbnail if the audio buffer is modified
  // 3. It is not obvious that setReader() will delete the AudioFormatReader when called a second time
  mThumbnail.clear();
  mThumbnail.reset(audioBuffer.getNumChannels(), sampleRate, audioBuffer.getNumSamples());
  mThumbnail.addBlock(0, audioBuffer, 0, audioBuffer.getNumSamples());

  mVisibleRange.setStart(0.0);  // never is not zero
  mVisibleRange.setEnd(duration);
  // start with everything selected
  mSelectedRange = mVisibleRange;
  updatePointMarker();
}

void TrimSelection::resized() {
  // Layout of component
  // - Thumbnail
  // - Selector
  // - buttons

  juce::Rectangle<int> r = getLocalBounds();
  // just enough padding to not be on the side borders
  r.reduce(r.getWidth() * 0.02, 0);
  mThumbnailRect = r.removeFromTop((2 * r.getHeight()) / 3);
  // extra 20 comes from 14 point font for text and some padding
  mSelectorRect = mThumbnailRect.removeFromBottom(PointMarker::height() + 20);
  mThumbnailShadow.setBounds(mThumbnailRect);

  const int btnHeightPadding = r.getHeight() * 0.1;
  const int btnWidth = r.getWidth() * 0.25;
  const int btnWidthPadding = r.getWidth() * (.25 / 4.0);
  juce::Rectangle<int> btnRect = r;

  btnRect.removeFromTop(btnHeightPadding);
  btnRect.removeFromLeft(btnWidthPadding);
  mBtnCancel.setBounds(btnRect.removeFromLeft(btnWidth));
  btnRect.removeFromLeft(btnWidthPadding);
  mBtnPlayback.setBounds(btnRect.removeFromLeft(btnWidth));
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

// There is a single instance of TrimSelection so clean up between uses
void TrimSelection::cleanup() { mParamUI.trimPlaybackOn = false; }

double TrimSelection::timeToXPosition(double time) const {
  const double start = time;
  const double width = mThumbnailRect.getWidth();
  const double length = mVisibleRange.getLength();
  const double offset = mThumbnailRect.getX();
  return ((start * width) / length) + offset;
}

double TrimSelection::xPositionToTime(double xPosition) const {
  // xPosition is overall component, need to adjust to thumbnail bounds
  const double x = (xPosition - mThumbnailRect.getX()) * mVisibleRange.getLength();
  const double width = mThumbnailRect.getWidth();
  return (x / width);
}

double TrimSelection::sampleToXPosition(int sample) const {
  const double start = static_cast<double>(sample);
  const double length = static_cast<double>(mParamUI.trimPlaybackMaxSample);
  const double width = mThumbnailRect.getWidth();
  const double offset = mThumbnailRect.getX();
  return ((start * width) / length) + offset;
}

int TrimSelection::timeToSample(double time) const {
  const double start = time;
  const double width = static_cast<double>(mParamUI.trimPlaybackMaxSample);
  const double length = mVisibleRange.getLength();
  return static_cast<int>((start * width) / length);
}

void TrimSelection::MarkerMouseDragged(PointMarker& marker, const juce::MouseEvent& e) {
  const double x = xPositionToTime(e.getEventRelativeTo(this).position.x);
  if (&marker == &mStartMarker) {
    if (mSelectedRange.getEnd() - x >= MIN_SELECTION_SEC) {
      mSelectedRange.setStart(juce::jmax(x, 0.0));
    } else {
      mSelectedRange.setStart(juce::jmax(mSelectedRange.getEnd() - MIN_SELECTION_SEC, 0.0));
    }
  } else if (&marker == &mEndMarker) {
    if (x - mSelectedRange.getStart() >= MIN_SELECTION_SEC) {
      mSelectedRange.setEnd(juce::jmin(x, mVisibleRange.getEnd()));
    } else {
      mSelectedRange.setEnd(juce::jmin(mSelectedRange.getStart() + MIN_SELECTION_SEC, mVisibleRange.getEnd()));
    }
  }
  updatePointMarker();
}

void TrimSelection::MarkerMouseUp(PointMarker& marker, const juce::MouseEvent& e) {
  const double x = xPositionToTime(e.getEventRelativeTo(this).position.x);
  if (&marker == &mStartMarker && mSelectedRange.getEnd() - x >= MIN_SELECTION_SEC) {
    mSelectedRange.setStart(juce::jmax(x, 0.0));
  } else if (&marker == &mEndMarker && x - mSelectedRange.getStart() >= MIN_SELECTION_SEC) {
    mSelectedRange.setEnd(juce::jmin(x, mVisibleRange.getEnd()));
  }
  updatePointMarker();
}

void TrimSelection::ThumbnailMouseDown(const juce::MouseEvent& e) {
  mParamUI.trimPlaybackOn = false;
  ThumbnailMouseDrag(e);  // same logic to select position
}

void TrimSelection::ThumbnailMouseDrag(const juce::MouseEvent& e) {
  double time = xPositionToTime(e.getEventRelativeTo(this).position.x);
  int sample = timeToSample(time);
  // prevent from selecting out of the thumbnail
  sample = juce::jmax(sample, 0);
  sample = juce::jmin(sample, mParamUI.trimPlaybackMaxSample);
  mParamUI.trimPlaybackSample = sample;
}

void TrimSelection::ThumbnailMouseUp(const juce::MouseEvent& e) {
  mParamUI.trimPlaybackOn = true;
  mBtnPlayback.setToggleState(true, juce::dontSendNotification);
}