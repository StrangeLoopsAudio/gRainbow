/*
  ==============================================================================

    NoteGrid.h
    Created: 24 Aug 2021 3:54:04pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h"

//==============================================================================
/*
 */
class NoteGrid : public juce::Component {
 public:
  NoteGrid(ParamsNote& paramsNote);
  ~NoteGrid() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void mouseDown(const juce::MouseEvent& event) override;

 private:
  static constexpr auto PADDING_SIZE = 6;
  static constexpr auto TITLE_HEIGHT = 20;
  static constexpr auto MAIN_TITLE = "note matrix";

  // UI values saved on resize
  juce::Rectangle<int> mGridRect;  // includes dead space
  float mSquareSize;
  juce::Rectangle<int> mSquare;  // size of one square
  juce::Rectangle<int> mColumn;  // size of one column
  juce::Rectangle<int> mTitleRect;
  juce::Rectangle<float> mTitleEdgeRect;
  juce::Rectangle<int> mNoteNamesRect;

  juce::Rectangle<int> mSelectableNoteRects;
  int mSelectableFactorX;
  int mSelectableFactorY;
  juce::Rectangle<int> getNoteSquare(int pitch, int generator);

  ParamsNote &mParamsNote;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteGrid)
};
