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
NoteGrid::NoteGrid(ParamsNote& paramsNote) : mParamsNote(paramsNote) { 
  setFramesPerSecond(REFRESH_RATE_FPS); 
}

NoteGrid::~NoteGrid() {}

void NoteGrid::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);  // clear the background

  juce::Colour mainColour = juce::Colours::white;

  // Note grid
  juce::Rectangle<int> gridRect = getLocalBounds().reduced(PADDING_SIZE);
  float squareSize = gridRect.getWidth() / (float)NUM_NOTES;
  juce::Rectangle<int> square = juce::Rectangle<int>(squareSize, squareSize);
  juce::Rectangle<int> column = juce::Rectangle<int>(squareSize, squareSize * NUM_GENERATORS);

  // Main title
  juce::Rectangle<int> titleRect = gridRect.removeFromTop(TITLE_HEIGHT);
  g.setColour(mainColour);
  g.fillRoundedRectangle(titleRect.reduced(0, PADDING_SIZE / 2).toFloat(), 10.0f);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(MAIN_TITLE), titleRect, juce::Justification::centred);

  juce::Rectangle<int> noteNamesRect = gridRect.removeFromTop(TITLE_HEIGHT);

  for (int i = 0; i < NUM_NOTES; ++i) {
    // Note column
    juce::Colour noteColour = Utils::getRainbow12Colour(i);
    g.setColour(noteColour);
    g.fillRect(column.withPosition(gridRect.getTopLeft() + juce::Point<int>(i * squareSize + 1, 1)).reduced(1.0f));
    // Note title
    g.drawRoundedRectangle(
        column.withPosition(noteNamesRect.getTopLeft() + juce::Point<int>(i * squareSize + 1, 1)).toFloat().reduced(1.0f), 10.0f,
        1.0f);
    g.setColour(juce::Colours::white);
    g.drawText(PITCH_CLASS_NAMES[i], noteNamesRect.getX() + i * squareSize, noteNamesRect.getY(), squareSize, squareSize,
               juce::Justification::centred);
    for (int j = 0; j < NUM_GENERATORS; ++j) {
      juce::Point<int> squarePos = gridRect.getTopLeft() + juce::Point<int>(i * squareSize + 1, j * squareSize + 1);

      // Clear rect area first to eliminate note colour
      g.setColour(juce::Colours::black);
      g.fillRect(square.withPosition(squarePos).reduced(2.0f));
      
      // Fill and label candidate if enabled, draw frame if not
      g.setColour(juce::Colour(Utils::GENERATOR_COLOURS_HEX[j]));
      if (mParamsNote.notes[i]->generators[j]->enable->get()) {
        g.fillRect(square.withPosition(squarePos).reduced(2.0f));
        g.setColour(juce::Colours::black);
        g.drawText(juce::String(mParamsNote.notes[i]->generators[j]->candidate->get() + 1), square.withPosition(squarePos),
                   juce::Justification::centred);
      } else {
        g.drawRect(square.withPosition(squarePos).reduced(1.0f), 2.0f);
      }
    }
  }

  // Outline rect
  g.setColour(mainColour);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
}

void NoteGrid::resized() {

}
