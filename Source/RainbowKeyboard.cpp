/*
  ==============================================================================

    RainbowKeyboard.cpp
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#include "RainbowKeyboard.h"

#include <JuceHeader.h>

#include "Utils.h"

//==============================================================================
RainbowKeyboard::RainbowKeyboard(juce::MidiKeyboardState& state)
    : mState(state) {}

RainbowKeyboard::~RainbowKeyboard() {}

void RainbowKeyboard::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw each white key then black key so the black keys rect are fully on top
  // of white key rect visually
  for (int key : WHITE_KEY_INDICES) {
    drawKey(g, key);
  }
  for (int key : BLACK_KEY_INDICES) {
    drawKey(g, key);
  }
}

float RainbowKeyboard::getPitchXRatio(int pitchClass) {
  float noteMiddle = getKeyRectangle(pitchClass).getCentreX() / getWidth();
  bool needsLeftShift = ((1 << (pitchClass)) & 0x021) != 0;
  bool needsRightShift = ((1 << (pitchClass)) & 0x810) != 0;
  float shiftAmount = (1.0f / 7.0f) * (BLACK_NOTE_SIZE_RATIO / 4.0f);
  if (needsLeftShift) { 
    return noteMiddle - shiftAmount;
  } else if (needsRightShift) {
    return noteMiddle + shiftAmount;
  } else {
    return noteMiddle;
  }
}

juce::Rectangle<float> RainbowKeyboard::getKeyRectangle(int pitchClass) {
  // Originially this was a mix of float and ints
  // Was changed to bring everything to floats
  // if any precision issue occur, feel free to keep as int as long as needed
  float keyWidth = static_cast<float>(getWidth() / 7);
  float blackKeyOffset = BLACK_NOTE_SIZE_RATIO * keyWidth / 2.0f;
  static const float notePos[] = {0.0f,
                                  keyWidth - blackKeyOffset,
                                  keyWidth,
                                  2.0f * keyWidth - blackKeyOffset,
                                  2.0f * keyWidth,
                                  3.0f * keyWidth,
                                  4.0f * keyWidth - blackKeyOffset,
                                  4.0f * keyWidth,
                                  5.0f * keyWidth - blackKeyOffset,
                                  5.0f * keyWidth,
                                  6.0f * keyWidth - blackKeyOffset,
                                  6.0f * keyWidth};

  bool blackKey = isBlackKey(pitchClass);
  auto width = blackKey ? BLACK_NOTE_SIZE_RATIO * keyWidth : keyWidth;
  auto height = blackKey ? getHeight() / 2 : getHeight();

  return juce::Rectangle<float>(notePos[pitchClass], 0, width, height);
}

void RainbowKeyboard::drawKey(juce::Graphics& g, int pitchClass) {
  bool isOver = pitchClass == mMouseOverNote;
  bool isDown = isOver && mIsNotePressed;
  auto keyColor = Utils::getRainbow12Colour(pitchClass);
  if (isOver) keyColor = keyColor.darker();
  // Will make 2nd level darker on press
  if (isDown) keyColor = keyColor.darker();

  juce::Rectangle<float> area = getKeyRectangle(pitchClass);
  g.setColour(keyColor);
  if (isDown) {
    g.setFillType(juce::ColourGradient(
        keyColor.brighter(), juce::Point<float>(0.0f, 0.0f), keyColor,
        juce::Point<float>(0, getHeight()), false));
  } else {
    g.setFillType(juce::ColourGradient(
        keyColor, juce::Point<float>(0.0f, 0.0f), keyColor.brighter(),
        juce::Point<float>(0, getHeight()), false));
  }

  g.fillRect(area);
  g.setColour(juce::Colours::black);
  g.drawRect(area, isBlackKey(pitchClass) ? 2 : 1);
}

void RainbowKeyboard::resized() {}

void RainbowKeyboard::mouseMove(const juce::MouseEvent& e) {
  updateNoteOver(e, false);
}

void RainbowKeyboard::mouseDrag(const juce::MouseEvent& e) {
  updateNoteOver(e, true);
}

void RainbowKeyboard::mouseDown(const juce::MouseEvent& e) {
  updateNoteOver(e, true);
}

void RainbowKeyboard::mouseUp(const juce::MouseEvent& e) {
  updateNoteOver(e, false);
}
void RainbowKeyboard::mouseEnter(const juce::MouseEvent& e) {
  updateNoteOver(e, false);
}

void RainbowKeyboard::mouseExit(const juce::MouseEvent& e) {
  updateNoteOver(e, false);
}

void RainbowKeyboard::updateNoteOver(const juce::MouseEvent& e, bool isDown) {
  auto pos = e.getEventRelativeTo(this).position;
  int newNote = xyToNote(pos);
  bool isValidNote = newNote >= 0 && newNote <= 12;
  if (newNote != mMouseOverNote) {
    // Hovering over new note, send note off for old note if necessary
    if (mIsNotePressed) {
      mState.noteOff(MIDI_CHANNEL, mPressedNote, mNoteVelocity);
      mPressedNote = -1;
    }
    if (isDown && isValidNote) {
      mState.noteOn(MIDI_CHANNEL, newNote, mNoteVelocity);
      mPressedNote = newNote;
    }
  } else {
    if (isDown && mPressedNote == -1 && isValidNote) {
      // Note on if pressing current note
      mState.noteOn(MIDI_CHANNEL, newNote, mNoteVelocity);
      mPressedNote = newNote;
    } else if (mIsNotePressed && !isDown) {
      // Note off if released current note
      mState.noteOff(MIDI_CHANNEL, mPressedNote, mNoteVelocity);
      mPressedNote = -1;
    }
  }
  mMouseOverNote = newNote;
  mIsNotePressed = isDown;
  repaint();
}

int RainbowKeyboard::xyToNote(juce::Point<float> pos) {
  if (!reallyContains(pos.toInt(), false)) return -1;
  auto p = pos;
  auto blackNoteLength = getHeight() / 2;

  if (pos.getY() < blackNoteLength) {
    for (int note : BLACK_KEY_INDICES) {
      if (getKeyRectangle(note).contains(pos)) {
        mNoteVelocity = juce::jmax(0.0f, pos.y / blackNoteLength);
        return note;
      }
    }
  }

  for (int note : WHITE_KEY_INDICES) {
    if (getKeyRectangle(note).contains(pos)) {
      auto whiteNoteLength = getHeight();
      mNoteVelocity = juce::jmax(0.0f, pos.y / (float)whiteNoteLength);
      return note;
    }
  }

  mNoteVelocity = 0;
  return -1;
}