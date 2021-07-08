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

  // Draw each white key, but rainbowy, then black key, but rainbowy, so the
  // black keys rect are fully on top of white key rect visually
  for (int key : WHITE_KEY_INDICES) {
    drawKey(g, key);
  }
  for (int key : BLACK_KEY_INDICES) {
    drawKey(g, key);
  }
}

float RainbowKeyboard::getPitchXRatio(int pitchClass) {
  float noteMiddle = mNoteRectangleMap[pitchClass].getCentreX() /
                     static_cast<float>(getWidth());
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

void RainbowKeyboard::fillNoteRectangleMap() {
  // Keep everything in float to match Rectangle type
  const float componentWidth = static_cast<float>(getWidth());
  const float componentHeight = static_cast<float>(getHeight());

  const float keyWidth = componentWidth / 7.0f;
  const float blackKeyOffset = BLACK_NOTE_SIZE_RATIO * keyWidth / 2.0f;

  const float notePos[NUM_KEYS] = {0.0f * keyWidth,
                                   1.0f * keyWidth - blackKeyOffset,
                                   1.0f * keyWidth,
                                   2.0f * keyWidth - blackKeyOffset,
                                   2.0f * keyWidth,
                                   3.0f * keyWidth,
                                   4.0f * keyWidth - blackKeyOffset,
                                   4.0f * keyWidth,
                                   5.0f * keyWidth - blackKeyOffset,
                                   5.0f * keyWidth,
                                   6.0f * keyWidth - blackKeyOffset,
                                   6.0f * keyWidth};

  const float blackNoteWidth = BLACK_NOTE_SIZE_RATIO * keyWidth;
  const float whiteNoteWidth = keyWidth;
  const float blackNoteHeight = componentHeight / 2.0f;
  const float whiteNoteHeight = componentHeight;

  for (int key = 0; key < NUM_KEYS; key++) {
    const bool blackKey = isBlackKey(key);
    mNoteRectangleMap[key].setBounds(
        notePos[key], 0, (blackKey) ? blackNoteWidth : whiteNoteWidth,
        (blackKey) ? blackNoteHeight : whiteNoteHeight);
  }
}

void RainbowKeyboard::drawKey(juce::Graphics& g, int pitchClass) {
  bool isOver = pitchClass == mMouseOverNote;
  bool isDown = isOver && mIsNotePressed;
  auto keyColor = Utils::getRainbow12Colour(pitchClass);
  if (isOver) keyColor = keyColor.darker();
  // Will make 2nd level darker on press
  if (isDown) keyColor = keyColor.darker();

  juce::Rectangle<float> area = mNoteRectangleMap[pitchClass];
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

void RainbowKeyboard::resized() { fillNoteRectangleMap(); }

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
  bool isValidNote = newNote != INVALID_NOTE;
  if (newNote != mMouseOverNote) {
    // Hovering over new note, send note off for old note if necessary
    if (mIsNotePressed) {
      mState.noteOff(MIDI_CHANNEL, mPressedNote, mNoteVelocity);
      mPressedNote = INVALID_NOTE;
    }
    if (isDown && isValidNote) {
      mState.noteOn(MIDI_CHANNEL, newNote, mNoteVelocity);
      mPressedNote = newNote;
    }
  } else {
    if (isDown && mPressedNote == INVALID_NOTE && isValidNote) {
      // Note on if pressing current note
      mState.noteOn(MIDI_CHANNEL, newNote, mNoteVelocity);
      mPressedNote = newNote;
    } else if (mIsNotePressed && !isDown) {
      // Note off if released current note
      mState.noteOff(MIDI_CHANNEL, mPressedNote, mNoteVelocity);
      mPressedNote = INVALID_NOTE;
    }
  }
  mMouseOverNote = newNote;
  mIsNotePressed = isDown;
  repaint();
}

int RainbowKeyboard::xyToNote(juce::Point<float> pos) {
  if (!reallyContains(pos.toInt(), false)) return INVALID_NOTE;
  // Since the Juce canvas is all in float, keep in float to prevent strange
  // int-vs-float rounding errors selecting the wrong key
  const float componentHeight = static_cast<float>(getHeight());
  const float blackNoteLength = componentHeight / 2.0f;

  // can skip quick checking black notes from y position
  if (pos.getY() < blackNoteLength) {
    for (int note : BLACK_KEY_INDICES) {
      if (mNoteRectangleMap[note].contains(pos)) {
        mNoteVelocity = juce::jmax(0.0f, pos.y / blackNoteLength);
        return note;
      }
    }
  }

  for (int note : WHITE_KEY_INDICES) {
    if (mNoteRectangleMap[note].contains(pos)) {
      mNoteVelocity = juce::jmax(0.0f, pos.y / componentHeight);
      return note;
    }
  }

  // note not found
  mNoteVelocity = 0;
  return INVALID_NOTE;
}