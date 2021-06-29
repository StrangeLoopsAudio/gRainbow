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

  // Draw each white key but rainbowy
  for (int key : WHITE_KEY_INDICES) {
    drawKey(g, key);
  }
  // Draw each black key but rainbowy
  for (int key : BLACK_KEY_INDICES) {
    drawKey(g, key);
  }
}

juce::Rectangle<float> RainbowKeyboard::getKeyRectangle(int pitchClass) {
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

  auto isBlackKey = ((1 << (pitchClass)) & 0x054a) != 0;
  auto width = isBlackKey ? BLACK_NOTE_SIZE_RATIO * keyWidth : keyWidth;
  auto height = isBlackKey ? getHeight() / 2 : getHeight();

  return juce::Rectangle<float>(notePos[pitchClass], 0, width, height);
}

void RainbowKeyboard::drawKey(juce::Graphics& g, int pitchClass) {
  bool isOver = pitchClass == mMouseOverNote;
  bool isDown = isOver && mIsNotePressed;
  float normPc = (pitchClass + 0.5) / NUM_KEYS;
  auto keyColor = Utils::getRainbow12Colour(1.0f - normPc);
  if (isOver) keyColor = keyColor.darker();
  if (isDown) keyColor = keyColor.darker();

  juce::Rectangle<float> area = getKeyRectangle(pitchClass);
  g.setColour(keyColor);
  g.fillRect(area);
  g.setColour(juce::Colours::black);
  auto isBlackKey = ((1 << (pitchClass)) & 0x054a) != 0;
  g.drawRect(area, isBlackKey ? 2 : 1);
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
  auto newNote = xyToNote(pos);
  if (newNote != mMouseOverNote) {
    // Hovering over new note, send note off for old note if necessary
    if (mIsNotePressed && newNote >= 0) {
      mState.noteOff(MIDI_CHANNEL, mPressedNote, mNoteVelocity);
      mPressedNote = -1;
    }
    if (isDown) {
      mState.noteOn(MIDI_CHANNEL, newNote, mNoteVelocity);
      mPressedNote = newNote;
    }
  } else {
    if (isDown && mPressedNote == -1 && newNote >= 0) {
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