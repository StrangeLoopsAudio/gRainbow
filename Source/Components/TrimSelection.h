/*
  ==============================================================================

    TrimSelection.h
    Created: 12 Jun 2022 3:05:13pm
    Author:  fricke

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Parameters.h"

/**
 * @brief Small triangles used to select trim range
 */
class PointMarker : public juce::Component {
 public:
  using MouseCallback = std::function<void(PointMarker&, const juce::MouseEvent&)>;
  PointMarker(juce::String name, juce::Colour colour, MouseCallback onMouseDrag, MouseCallback onMouseUp);
  static int height() { return HEIGHT + APEX_HEIGHT; }

 private:
  void resized() override;
  void paint(juce::Graphics& g) override;
  bool hitTest(int x, int y) override;

  void mouseDrag(const juce::MouseEvent& e) override { mOnMouseDrag(*this, e); }
  void mouseUp(const juce::MouseEvent& e) override { mOnMouseUp(*this, e); }

  static constexpr int HEIGHT = 10;
  static constexpr int APEX_HEIGHT = 6;
  juce::String mName;
  juce::Path mPath;
  juce::Colour mColour;
  MouseCallback mOnMouseDrag;
  MouseCallback mOnMouseUp;
};

/**
 * @brief juce::AudioThumbnail are not juce::Component and this class is just to put a mouse listener ontop of the AudioThumbnail
 */
class AudioThumbnailShadow : public juce::Component {
 public:
  using MouseCallback = std::function<void(const juce::MouseEvent&)>;
  AudioThumbnailShadow(MouseCallback onMouseDown, MouseCallback onMouseDrag, MouseCallback onMouseUp)
      : mOnMouseDown(std::move(onMouseDown)), mOnMouseDrag(std::move(onMouseDrag)), mOnMouseUp(std::move(onMouseUp)){};
  void paint(juce::Graphics& g) override{};
  void resized() override{};

 private:
  void mouseDown(const juce::MouseEvent& e) override { mOnMouseDown(e); }
  void mouseDrag(const juce::MouseEvent& e) override { mOnMouseDrag(e); }
  void mouseUp(const juce::MouseEvent& e) override { mOnMouseUp(e); }
  MouseCallback mOnMouseDown;
  MouseCallback mOnMouseDrag;
  MouseCallback mOnMouseUp;
};

/**
 * @brief Lets the user trim the sample when loading it
 */
class TrimSelection : public juce::Component {
 public:
  TrimSelection(juce::AudioFormatManager& formatManager, ParamUI& paramUI);
  ~TrimSelection();

  void paint(juce::Graphics& g) override;
  void resized() override;

  void parse(const juce::AudioBuffer<float>& audioBuffer, double sampleRate, juce::String& error);

  std::function<void(void)> onCancel = nullptr;
  std::function<void(juce::Range<double>)> onProcessSelection = nullptr;

 private:
  static constexpr double MIN_SELECTION_SEC = 5.0;

  juce::AudioThumbnailCache mThumbnailCache;
  juce::AudioThumbnail mThumbnail;
  AudioThumbnailShadow mThumbnailShadow;

  ParamUI& mParamUI;

  // For the UI everything is in seconds as a double
  juce::Range<double> mVisibleRange;
  juce::Range<double> mSelectedRange;

  juce::TextButton mBtnCancel;
  juce::TextButton mBtnPlayback;
  juce::TextButton mBtnSetSelection;

  PointMarker mStartMarker;
  PointMarker mEndMarker;
  juce::String mStartTimeString;
  juce::String mEndTimeString;
  juce::DrawableRectangle mPlaybackMarker;

  // Resized bound values
  juce::Rectangle<int> mThumbnailRect;
  juce::Rectangle<int> mSelectorRect;

  void cleanup();

  void updatePointMarker();
  double timeToXPosition(double time) const;
  double xPositionToTime(double xPosition) const;
  double sampleToXPosition(int sample) const;
  int timeToSample(double time) const;

  void MarkerMouseDragged(PointMarker& marker, const juce::MouseEvent& e);
  void MarkerMouseUp(PointMarker& marker, const juce::MouseEvent& e);
  void ThumbnailMouseDown(const juce::MouseEvent& e);
  void ThumbnailMouseDrag(const juce::MouseEvent& e);
  void ThumbnailMouseUp(const juce::MouseEvent& e);
};
