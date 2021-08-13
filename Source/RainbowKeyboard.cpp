/*
  ==============================================================================

    RainbowKeyboard.cpp
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#include "RainbowKeyboard.h"

#include <JuceHeader.h>

//==============================================================================
RainbowKeyboard::RainbowKeyboard(juce::MidiKeyboardState& state)
    : mState(state) {
  // Currently, only need to set mapping once at start
  const char keyboardMapping[Utils::PitchClass::COUNT + 1] = "awsedftgyhuj";
  for (Utils::PitchClass pitchClass : Utils::ALL_PITCH_CLASS) {
    mKeyPresses[pitchClass] = juce::KeyPress(keyboardMapping[pitchClass], 0, 0);
  }
}

RainbowKeyboard::~RainbowKeyboard() {}

void RainbowKeyboard::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw each white key, but rainbowy, then black key, but rainbowy, so the
  // black keys rect are fully on top of white key rect visually
  for (Utils::PitchClass pitchClass : WHITE_KEYS_PITCH_CLASS) {
    drawKey(g, pitchClass);
  }
  for (Utils::PitchClass pitchClass : BLACK_KEYS_PITCH_CLASS) {
    drawKey(g, pitchClass);
  }
}

float RainbowKeyboard::getPitchXRatio(Utils::PitchClass pitchClass) {
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

  const float notePos[Utils::PitchClass::COUNT] = {
      0.0f * keyWidth,
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

  for (Utils::PitchClass key : Utils::ALL_PITCH_CLASS) {
    const bool blackKey = isBlackKey(key);
    mNoteRectangleMap[key].setBounds(
        notePos[key], 0, (blackKey) ? blackNoteWidth : whiteNoteWidth,
        (blackKey) ? blackNoteHeight : whiteNoteHeight);
  }
}

void RainbowKeyboard::drawKey(juce::Graphics& g, Utils::PitchClass pitchClass) {
  auto keyColor = Utils::getRainbow12Colour(pitchClass);
  bool isDown = pitchClass == mCurrentNote.pitch;

  // if down, extra dark
  // if no note is down, lightly darken if mouse is hovering it
  if (isDown) {
    keyColor = keyColor.darker().darker();
  } else if (pitchClass == mHoverNote.pitch) {
    keyColor = keyColor.darker();
  }

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
  updateMouseState(e, false);
}

void RainbowKeyboard::mouseDrag(const juce::MouseEvent& e) {
  updateMouseState(e, true);
}

void RainbowKeyboard::mouseDown(const juce::MouseEvent& e) {
  updateMouseState(e, true);
}

void RainbowKeyboard::mouseUp(const juce::MouseEvent& e) {
  updateMouseState(e, false);
}

void RainbowKeyboard::mouseEnter(const juce::MouseEvent& e) {
  // This is NOT called if mouseDrag() is still happening
  updateMouseState(e, false);
}

void RainbowKeyboard::mouseExit(const juce::MouseEvent& e) {
  // This is NOT called if mouseDrag() is still happening
  updateMouseState(e, false);
}

void RainbowKeyboard::updateMouseState(const juce::MouseEvent& e, bool isDown) {
  auto pos = e.getEventRelativeTo(this).position;
  mHoverNote = xyMouseToNote(pos);

  if (mCurrentNote.input != InputType::MOUSE &&
      mCurrentNote.input != InputType::NONE) {
    return;
  }

  // Will be invalid if mouse is dragged outside of keyboard
  bool isValidNote = mHoverNote.pitch != Utils::PitchClass::NONE;
  if (mHoverNote.pitch != mCurrentNote.pitch) {
    // Hovering over new note, send note off for old note if necessary
    // Will turn off also if mouse exit keyboard
    if (mCurrentNote.pitch != Utils::PitchClass::NONE) {
      mState.noteOff(MIDI_CHANNEL, mCurrentNote.pitch, mCurrentNote.velocity);
      mCurrentNote = RainbowKeyboard::Note();
    }
    if (isDown && isValidNote) {
      mState.noteOn(MIDI_CHANNEL, mHoverNote.pitch, mHoverNote.velocity);
      mCurrentNote = mHoverNote;
    }
  } else {
    if (isDown && (mCurrentNote.pitch == Utils::PitchClass::NONE) &&
        isValidNote) {
      // Note on if pressing current note
      mState.noteOn(MIDI_CHANNEL, mHoverNote.pitch, mHoverNote.velocity);
      mCurrentNote = mHoverNote;
    } else if ((mCurrentNote.pitch != Utils::PitchClass::NONE) && !isDown) {
      // Note off if released current note
      mState.noteOff(MIDI_CHANNEL, mCurrentNote.pitch, mCurrentNote.velocity);
      mCurrentNote = RainbowKeyboard::Note();
    } else {
      // still update state
      mCurrentNote = mHoverNote;
    }
  }
  repaint();
}

RainbowKeyboard::Note RainbowKeyboard::xyMouseToNote(juce::Point<float> pos) {
  if (!reallyContains(pos.toInt(), false)) return RainbowKeyboard::Note();
  // Since the Juce canvas is all in float, keep in float to prevent strange
  // int-vs-float rounding errors selecting the wrong key
  const float componentHeight = static_cast<float>(getHeight());
  const float blackNoteLength = componentHeight / 2.0f;

  // can skip quick checking black notes from y position
  if (pos.getY() < blackNoteLength) {
    for (Utils::PitchClass pitchClass : BLACK_KEYS_PITCH_CLASS) {
      if (mNoteRectangleMap[pitchClass].contains(pos)) {
        return RainbowKeyboard::Note(pitchClass,
                                     juce::jmax(0.0f, pos.y / blackNoteLength),
                                     InputType::MOUSE);
      }
    }
  }

  for (Utils::PitchClass pitchClass : WHITE_KEYS_PITCH_CLASS) {
    if (mNoteRectangleMap[pitchClass].contains(pos)) {
      return RainbowKeyboard::Note(pitchClass,
                                   juce::jmax(0.0f, pos.y / componentHeight),
                                   InputType::MOUSE);
    }
  }

  // note not found
  return RainbowKeyboard::Note();
}

void RainbowKeyboard::updateKeyState(const juce::KeyPress* pKey,
                                     bool isKeyDown) {
  if (mCurrentNote.input != InputType::KEYBOARD &&
      mCurrentNote.input != InputType::NONE) {
    // if other input types are being used, ignore keyboard inputs all together
    return;
  }

  const bool notePlaying = mCurrentNote.pitch != Utils::PitchClass::NONE;
  bool stateChange = false;

  // Get the last pressed note
  if (pKey != nullptr) {
    jassert(isKeyDown == true);  // juce::KeyListener only gives key on presses
    const int keyCode = pKey->getKeyCode();

    if (notePlaying && mKeyPresses[mLastPressedKey].isKeyCode(keyCode)) {
      return;  // same note being played is being held down
    }

    // look for new key being pressed
    bool validKey = false;
    for (Utils::PitchClass pitchClass : Utils::ALL_PITCH_CLASS) {
      if (mKeyPresses[pitchClass].isKeyCode(keyCode)) {
        mLastPressedKey = pitchClass;
        validKey = true;
        break;
      }
    }

    if (!validKey) {
      // another key, that is not related to keyboard, was pressed
      return;
    }
  } else if (isKeyDown) {
    // if pKey is null and note is down, then its just
    // juce::KeyListener::keyStateChanged giving redundant information
    return;
  }

  if (isKeyDown) {
    // new key is pressed
    if (notePlaying) {
      // two keys were pressed, so need to turn off old one first
      mState.noteOff(MIDI_CHANNEL, mCurrentNote.pitch, mCurrentNote.velocity);
    }
    mCurrentNote =
        RainbowKeyboard::Note(mLastPressedKey, 0.5f, InputType::KEYBOARD);
    mState.noteOn(MIDI_CHANNEL, mCurrentNote.pitch, mCurrentNote.velocity);
    stateChange = true;
  } else {
    if (!mKeyPresses[mCurrentNote.pitch].isCurrentlyDown()) {
      // key has been released
      mState.noteOff(MIDI_CHANNEL, mCurrentNote.pitch, mCurrentNote.velocity);
      mCurrentNote = RainbowKeyboard::Note();
      stateChange = true;
    }
  }

  // if note is pressed or release, want to repaint only. Avoid painting every
  // keyboard input
  if (stateChange) {
    repaint();
  }
}