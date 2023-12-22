/*
  ==============================================================================

    RainbowKeyboard.cpp
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#include "RainbowKeyboard.h"
#include "../Settings.h"
#include "BinaryData.h"
#include "Utils/Colour.h"

//==============================================================================
RainbowKeyboard::RainbowKeyboard(juce::MidiKeyboardState& state, Parameters& parameters) : mState(state), mParameters(parameters) {
  mNoteVelocity.fill(0.0f);
  for (auto& note : mParameters.note.notes) {
    for (auto& gen : note->generators) {
      gen->enable->addListener(this);
    }
  }
}

RainbowKeyboard::~RainbowKeyboard() {
  for (auto& note : mParameters.note.notes) {
    for (auto& gen : note->generators) {
      gen->enable->removeListener(this);
    }
  }
}

void RainbowKeyboard::parameterValueChanged(int, float) { resized(); }

void RainbowKeyboard::paint(juce::Graphics& g) {
  // Draw each white key, then black key, so the
  // black keys rect are fully on top of white key rect visually
  for (Utils::PitchClass pitchClass : WHITE_KEYS_PITCH_CLASS) {
    drawKey(g, pitchClass);
  }
  for (Utils::PitchClass pitchClass : BLACK_KEYS_PITCH_CLASS) {
    drawKey(g, pitchClass);
  }
  
  // Global select rect
  const bool isGlobal = mParameters.selectedParams->type == ParamType::GLOBAL;
  auto globalFillColour = isGlobal ? Utils::GLOBAL_COLOUR : juce::Colours::transparentBlack;
  if (mHoverGlobal && !isGlobal) globalFillColour = Utils::GLOBAL_COLOUR.withAlpha(0.2f);
  g.setColour(globalFillColour);
  g.fillRoundedRectangle(mGlobalRect, 10);
  g.setColour(Utils::GLOBAL_COLOUR);
  g.drawRoundedRectangle(mGlobalRect, 10, 2);
  auto globalTextColour = isGlobal ? Utils::BG_COLOUR : Utils::GLOBAL_COLOUR;
  g.setColour(globalTextColour);
  g.drawText("global", mGlobalRect, juce::Justification::centred);
}

float RainbowKeyboard::getPitchXRatio(Utils::PitchClass pitchClass) {
  float noteMiddle = mNoteRectMap[pitchClass].getCentreX() / static_cast<float>(getWidth());
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
  const float width = static_cast<float>(getWidth());
  const float height = static_cast<float>(getHeight() - GLOBAL_RECT_HEIGHT - Utils::PADDING);
  
  const float keyWidth = width / 7.0f;
  const float blackKeyOffset = BLACK_NOTE_SIZE_RATIO * keyWidth / 2.0f;
  
  const float notePos[Utils::PitchClass::COUNT] = {0.0f * keyWidth,
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
  const float blackNoteHeight = height * 0.65f;
  const float whiteNoteHeight = height;
  
  for (Utils::PitchClass key : Utils::ALL_PITCH_CLASS) {
    const bool blackKey = isBlackKey(key);
    const auto keyRect = juce::Rectangle<float>(notePos[key], 0.0f, (blackKey) ? blackNoteWidth : whiteNoteWidth,
                                                (blackKey) ? blackNoteHeight : whiteNoteHeight);
    mNoteRectMap[key] = keyRect.reduced(1, 1);
  }
  
  mGlobalRect = getLocalBounds().toFloat().removeFromBottom(GLOBAL_RECT_HEIGHT);
}

void RainbowKeyboard::drawKey(juce::Graphics& g, Utils::PitchClass pitchClass) {
  const float velocity = mNoteVelocity[pitchClass];
  const bool isDown = (velocity > 0.0f);
  const auto* note = mParameters.note.notes[pitchClass].get();
  const bool isPitchSelected = mParameters.selectedParams == note;
  const bool isGenSelected = std::find_if(note->generators.begin(), note->generators.end(), [this](const std::unique_ptr<ParamGenerator>& gen) { return gen.get() == mParameters.selectedParams; }) != std::end(note->generators);
  const ParamNote& paramNote = *mParameters.note.notes[pitchClass];

  const bool isBlack = isBlackKey(pitchClass);
  const juce::Colour defaultKeyColour = isBlack ? juce::Colours::black : juce::Colours::white;
  const juce::Colour keyRbColour = Utils::getRainbow12Colour(pitchClass);
  juce::Colour keyColour = isPitchSelected ? keyRbColour : defaultKeyColour;

  // if down, extra dark
  // if no note is down, lightly darken if mouse is hovering it
  if (pitchClass == mHoverNote.pitch && keyColour == defaultKeyColour) {
    keyColour = isBlack ? keyColour.brighter() : keyColour.darker();
  }

  g.setColour(keyColour);

  juce::Rectangle<float> area = mNoteRectMap[pitchClass];
  // Main key outline
  g.setColour(keyColour);
  g.fillRoundedRectangle(area, 5);
  // Generator selected indicator
  g.setColour(keyRbColour);
  if (isGenSelected) g.drawRoundedRectangle(area.reduced(1, 1), 4, 2);
  
  // Key hint color
  g.setColour(Utils::getRainbow12Colour(pitchClass));
  g.fillRect(area.withHeight(5));
  
}

void RainbowKeyboard::resized() { fillNoteRectangleMap(); }

void RainbowKeyboard::setMidiNotes(const juce::Array<Utils::MidiNote>& midiNotes) {
  mNoteVelocity.fill(0.0f);
  for (const Utils::MidiNote& midiNote : midiNotes) {
    mNoteVelocity[midiNote.pitch] = midiNote.velocity;
  }
}

void RainbowKeyboard::mouseMove(const juce::MouseEvent& e) { updateMouseState(e, false, false); }

void RainbowKeyboard::mouseDrag(const juce::MouseEvent& e) { updateMouseState(e, true, false); }

void RainbowKeyboard::mouseDown(const juce::MouseEvent& e) { updateMouseState(e, true, false); }

void RainbowKeyboard::mouseUp(const juce::MouseEvent& e) {
  updateMouseState(e, false, true);
}

void RainbowKeyboard::mouseEnter(const juce::MouseEvent& e) {
  // This is NOT called if mouseDrag() is still happening
  updateMouseState(e, false, false);
}

void RainbowKeyboard::mouseExit(const juce::MouseEvent& e) {
  // This is NOT called if mouseDrag() is still happening
  updateMouseState(e, false, false);
}

/**
  @brief Only used to inject a note into the midiKeyboard state. The listener
  will set the rainbowKeyboard state

  Takes input of Component::keyStateChanged from parent component due to lack
  of always having focus on this component
*/
void RainbowKeyboard::updateMouseState(const juce::MouseEvent& e, bool isDown, bool isClick) {
  const juce::Point<float> pos = e.getEventRelativeTo(this).position;

  mHoverNote = xyMouseToNote(pos);

  // Will be invalid if mouse is dragged outside of keyboard
  bool isValidNote = mHoverNote.pitch != Utils::PitchClass::NONE;
  if (mHoverNote.pitch != mMouseNote.pitch) {
    // Hovering over new note, send note off for old note if necessary
    // Will turn off also if mouse exit keyboard
    if (mMouseNote.pitch != Utils::PitchClass::NONE) {
      mState.noteOff(MIDI_CHANNEL, mMouseNote.pitch, mMouseNote.velocity);
      mMouseNote = Utils::MidiNote();
    }
    if (isDown && isValidNote) {
      mState.noteOn(MIDI_CHANNEL, mHoverNote.pitch, mHoverNote.velocity);
      mMouseNote = mHoverNote;
      // Select current note for parameter edits and send update
      if (mParameters.selectedParams != mParameters.note.notes[mHoverNote.pitch].get()) {
        mParameters.selectedParams = mParameters.note.notes[mHoverNote.pitch].get();
        if (mParameters.onSelectedChange != nullptr) mParameters.onSelectedChange();
      }
    }
  } else {
    if (isDown && (mMouseNote.pitch == Utils::PitchClass::NONE) && isValidNote) {
      // Note on if pressing current note
      mState.noteOn(MIDI_CHANNEL, mHoverNote.pitch, mHoverNote.velocity);
      mMouseNote = mHoverNote;
    } else if ((mMouseNote.pitch != Utils::PitchClass::NONE) && !isDown) {
      // Note off if released current note
      mState.noteOff(MIDI_CHANNEL, mMouseNote.pitch, mMouseNote.velocity);
      mMouseNote = Utils::MidiNote();
    } else {
      // still update state
      mMouseNote = mHoverNote;
    }
  }
  
  // Check if global is hovered or clicked
  mHoverGlobal = false;
  if (mHoverNote.pitch == Utils::PitchClass::NONE) {
    if (mGlobalRect.contains(pos)) {
      mHoverGlobal = true;
      if (isDown && mParameters.selectedParams != &mParameters.global) {
        // Select global for parameter edits and send update
        mParameters.selectedParams = &mParameters.global;
        if (mParameters.onSelectedChange != nullptr) mParameters.onSelectedChange();
      }
    }
  }
  repaint();
}

void RainbowKeyboard::generatorOnClick(ParamGenerator* gen) {
  // If you have selected Gen "X" and then Increase on gen "Y", it will swap to "Y" on top of increasing the candidate value
  if (mParameters.selectedParams != gen) {
    mParameters.selectedParams = gen;
    if (mParameters.onSelectedChange != nullptr) {
      mParameters.onSelectedChange();
    }
  }
}

Utils::MidiNote RainbowKeyboard::xyMouseToNote(juce::Point<float> pos) {
  if (!reallyContains(pos.toInt(), false)) return Utils::MidiNote();
  
  // Check black keys then white keys
  if (pos.getY() < mNoteRectMap[BLACK_KEYS_PITCH_CLASS[0]].getBottom()) {
    for (Utils::PitchClass pitchClass : BLACK_KEYS_PITCH_CLASS) {
      const auto& noteRect = mNoteRectMap[pitchClass];
      if (noteRect.contains(pos)) {
        float velocity = juce::jmax(0.0f, 1.0f - ((pos.y - noteRect.getY()) / noteRect.getHeight()));
        return Utils::MidiNote(pitchClass, velocity);
      }
    }
  }
  
  for (Utils::PitchClass pitchClass : WHITE_KEYS_PITCH_CLASS) {
    const auto& noteRect = mNoteRectMap[pitchClass];
    if (noteRect.contains(pos)) {
      float velocity = juce::jmax(0.0f, 1.0f - ((pos.y - noteRect.getY()) / noteRect.getHeight()));
      return Utils::MidiNote(pitchClass, velocity);
    }
  }

  // note not found
  return Utils::MidiNote();
}
