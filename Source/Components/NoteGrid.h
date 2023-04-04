/*
  ==============================================================================

    NoteGrid.h
    Created: 24 Aug 2021 3:54:04pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "../Parameters.h"

//==============================================================================
/*
 */
class NoteGrid : public juce::AnimatedAppComponent {
 public:
  NoteGrid(ParamsNote& paramsNote);
  ~NoteGrid() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void update() override{};

 private:
  static constexpr auto REFRESH_RATE_FPS = 5;
  static constexpr auto PADDING_SIZE = 6;
  static constexpr auto TITLE_HEIGHT = 20;
  static constexpr auto MAIN_TITLE = "note matrix";

  // UI values saved on resize
  juce::Rectangle<int> mGridRect;
  float mSquareSize;
  juce::Rectangle<int> mSquare;
  juce::Rectangle<int> mColumn;
  juce::Rectangle<int> mTitleRect;
  juce::Rectangle<float> mTitleEdgeRect;
  juce::Rectangle<int> mNoteNamesRect;

  ParamsNote &mParamsNote;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteGrid)
};
