/*
  ==============================================================================

    NoteGrid.cpp
    Created: 24 Aug 2021 3:54:04pm
    Author:  brady

  ==============================================================================
*/

#include <JuceHeader.h>
#include "NoteGrid.h"
#include "Utils.h"

//==============================================================================
NoteGrid::NoteGrid(ParamsNote& paramsNote) : mParamsNote(paramsNote) {}

NoteGrid::~NoteGrid() {}

juce::Rectangle<int> NoteGrid::getNoteSquare(int pitch, int generator) {
  jassert(pitch < Utils::PitchClass::COUNT);
  jassert(generator < NUM_GENERATORS);
  juce::Point<int> squarePos = mGridRect.getTopLeft() + juce::Point<int>(pitch * mSquareSize + 1, generator * mSquareSize + 1);
  return mSquare.withPosition(squarePos);
}

void NoteGrid::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);  // clear the background

  juce::Colour mainColour = juce::Colours::white;

  // Main title
  g.setColour(mainColour);
  g.fillRoundedRectangle(mTitleEdgeRect, 10.0f);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(MAIN_TITLE), mTitleRect, juce::Justification::centred);

  for (int i = 0; i < Utils::PitchClass::COUNT; ++i) {
    // Note column
    juce::Colour noteColour = Utils::getRainbow12Colour(i);
    g.setColour(noteColour);
    g.fillRect(mColumn.withPosition(mGridRect.getTopLeft() + juce::Point<int>(i * mSquareSize + 1, 1)).reduced(1.0f));
    // Note title
    g.drawRoundedRectangle(
        mColumn.withPosition(mNoteNamesRect.getTopLeft() + juce::Point<int>(i * mSquareSize + 1, 1)).toFloat().reduced(1.0f), 10.0f,
        1.0f);
    g.setColour(juce::Colours::white);
    g.drawText(PITCH_CLASS_NAMES[i], mNoteNamesRect.getX() + i * mSquareSize, mNoteNamesRect.getY(), mSquareSize, mSquareSize,
               juce::Justification::centred);
    for (int j = 0; j < NUM_GENERATORS; ++j) {
      juce::Rectangle<int> squareRect = getNoteSquare(i, j);

      // draw frame
      g.setColour(noteColour);
      g.drawRect(squareRect.reduced(1.0f), 2.0f);

      const bool selected = mParamsNote.notes[i]->generators[j]->selected;
      if (mParamsNote.notes[i]->generators[j]->enable->get()) {
        // Fill and label candidate if enabled
        g.setColour(selected ? juce::Colours::white : noteColour);
        g.fillRect(squareRect.reduced(2.0f));
        g.setColour(juce::Colours::black);
        g.drawText(juce::String(mParamsNote.notes[i]->generators[j]->candidate->get() + 1), squareRect,
                   juce::Justification::centred);
      } else {
        // Clear rect area only if not enabled
        g.setColour(selected ? juce::Colours::white : juce::Colours::black);
        g.fillRect(squareRect.reduced(2.0f));
      }
    }
  }

  // Outline rect
  g.setColour(mainColour);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
}

void NoteGrid::resized() {
  mGridRect = getLocalBounds().reduced(PADDING_SIZE);
  mSquareSize = mGridRect.getWidth() / static_cast<float>(Utils::PitchClass::COUNT);
  mSquare = juce::Rectangle<int>(mSquareSize, mSquareSize);
  mColumn = juce::Rectangle<int>(mSquareSize, mSquareSize * NUM_GENERATORS);
  mTitleRect = mGridRect.removeFromTop(TITLE_HEIGHT);
  mTitleEdgeRect = mTitleRect.reduced(0, PADDING_SIZE / 2).toFloat();
  mNoteNamesRect = mGridRect.removeFromTop(TITLE_HEIGHT);

  juce::Rectangle<int> topLeftNote = getNoteSquare(0, 0);
  juce::Rectangle<int> bottomRightNote = getNoteSquare(Utils::PitchClass::COUNT - 1, NUM_GENERATORS - 1);
  mSelectableNoteRects = juce::Rectangle<int>(topLeftNote.getTopLeft(), bottomRightNote.getBottomRight());
  mSelectableFactorX = mSelectableNoteRects.getWidth() / Utils::PitchClass::COUNT;
  mSelectableFactorY = mSelectableNoteRects.getHeight() / NUM_GENERATORS;
}

void NoteGrid::mouseDown(const juce::MouseEvent& event) {
  if (mSelectableNoteRects.contains(event.getMouseDownPosition())) {
    int pitchClass = (event.getMouseDownX() - mSelectableNoteRects.getX()) / mSelectableFactorX;
    int generator = (event.getMouseDownY() - mSelectableNoteRects.getY()) / mSelectableFactorY;
    mParamsNote.notes[pitchClass]->generators[generator]->selected =
        !mParamsNote.notes[pitchClass]->generators[generator]->selected;
  }
}