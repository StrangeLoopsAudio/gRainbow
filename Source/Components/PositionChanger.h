/*
  ==============================================================================

    PositionChanger.h
    Created: 3 Jul 2021 10:08:25pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
 */
class PositionChanger : public juce::Component {
 public:
  PositionChanger();
  ~PositionChanger() override;

  void paint(juce::Graphics&) override;
  void resized() override;
  void mouseMove(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseEnter(const juce::MouseEvent&) override;
  void mouseExit(const juce::MouseEvent&) override;

  void setSolo(bool isSolo);
  void setColour(juce::Colour colour);
  void setNumPositions(int numPositions);
  int getPositionNumber() { return mPosition; }
  void setPositionNumber(int position);

  std::function<void(bool isRight)> onPositionChanged = nullptr;
  std::function<void(bool isSolo)> onSoloChanged = nullptr;

 private:
  static constexpr auto TITLE_PERC = 0.6;
  static constexpr auto ARROW_PERC = (1.0 - TITLE_PERC) / 2.0;
  static constexpr auto BUBBLE_WIDTH = 10;
  static constexpr auto BUBBLE_PADDING = 5;
  static constexpr auto SOLO_HEIGHT_PERC = 0.4;

  bool mIsSolo = false;
  juce::Colour mColour;
  int mNumPositions = 0;
  int mIndexInBoxes;  // Index among other boxes on screen
  int mPosition;      // Position index
  bool mIsClickingArrow = false;
  bool mIsOverLeftArrow = false;
  bool mIsOverRightArrow = false;
  bool mIsOverSolo = false;

  // UI values saved on resize
  juce::Path mLeftPath;
  juce::Path mRightPath;
  juce::Line<float> mLeftArrowLine;
  juce::Line<float> mRightArrowLine;
  juce::Rectangle<int> mTitleRect;
  juce::Rectangle<float> mSoloRect;

  void updateMouseOver(const juce::MouseEvent& e, bool isDown);
  bool isLeftArrow(juce::Point<float> point);
  bool isRightArrow(juce::Point<float> point);
  void positionChanged(bool isRight);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionChanger)
};
