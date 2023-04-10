/*
  ==============================================================================

    RainbowKeyboard.cpp
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#include "RainbowKeyboard.h"
#include "Settings.h"
#include "../Parameters.h"

//==============================================================================
RainbowKeyboard::RainbowKeyboard(juce::MidiKeyboardState& state) : mState(state) {
  mNoteVelocity.fill(0.0f);
  mRandom.setSeed(juce::Time::currentTimeMillis());
}

RainbowKeyboard::~RainbowKeyboard() {}

void RainbowKeyboard::paint(juce::Graphics& g) {
  //g.fillAll(juce::Colours::transparentBlack);

  // Draw each white key, but rainbowy, then black key, but rainbowy, so the
  // black keys rect are fully on top of white key rect visually
  for (Utils::PitchClass key : Utils::ALL_PITCH_CLASS) {
    drawKey(g, key);
  }

  g.setColour(Utils::GLOBAL_COLOUR);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
}

float RainbowKeyboard::getPitchXRatio(Utils::PitchClass pitchClass) {
  return mNoteRectangleMap[pitchClass].getCentreX() / static_cast<float>(getWidth());
}

void RainbowKeyboard::fillNoteRectangleMap() {
  // Keep everything in float to match Rectangle type
  const float componentWidth = static_cast<float>(getWidth());
  const float componentHeight = static_cast<float>(getHeight());

  juce::Rectangle<float> r = getLocalBounds().toFloat();
  
  r = r.reduced(Utils::PADDING, Utils::PADDING).withCentre(getLocalBounds().toFloat().getCentre());

  // key width = leftover width after padding / num pitch classes * component width
  const float keyWidth = (r.getWidth() - (Utils::PADDING * (Utils::PitchClass::COUNT - 1))) / Utils::PitchClass::COUNT;
  const float keyHeight = componentHeight * NOTE_BODY_HEIGHT;

  for (Utils::PitchClass key : Utils::ALL_PITCH_CLASS) {
    //float keyCenterY = 
    mNoteRectangleMap[key] = r.removeFromLeft(keyWidth).removeFromBottom(keyHeight);
    r.removeFromLeft(Utils::PADDING);
  }
}

void RainbowKeyboard::drawKey(juce::Graphics& g, Utils::PitchClass pitchClass) {
  juce::Colour keyColor = Utils::getRainbow12Colour(pitchClass).withSaturation(NOTE_BODY_SATURATION);
  const float velocity = mNoteVelocity[pitchClass];
  const bool isDown = (velocity > 0.0f);

  // if down, extra dark
  // if no note is down, lightly darken if mouse is hovering it
  if (isDown) {
    keyColor = keyColor.darker().darker();
  } else if (pitchClass == mHoverNote.pitch) {
    keyColor = keyColor.darker();
  }

  g.setColour(keyColor);
  juce::Colour color1 = (isDown) ? keyColor.brighter() : keyColor;
  juce::Colour color2 = (isDown) ? keyColor : keyColor.brighter();
  g.setFillType(juce::ColourGradient(color1, juce::Point<float>(0.0f, 0.0f), color2, juce::Point<float>(0, getHeight()), false));

  juce::Rectangle<float> area = mNoteRectangleMap[pitchClass];
  g.fillRoundedRectangle(area, 10.0f);
  g.setColour(keyColor.withSaturation(1.0f));
  g.drawRoundedRectangle(area, 10.0f, 1.0f);

  // display animation to show the velocity level of the note
  if (isDown && PowerUserSettings::get().getAnimated()) {
    AnimationLUT& lut = mAnimationLUT[pitchClass];
    juce::Rectangle<float> block = juce::Rectangle<float>(lut.noteWidthSplit, lut.noteWidthSplit);
    g.setColour(juce::Colours::white);
    g.drawLine(area.getX(), lut.velocityLine, area.getRight(), lut.velocityLine, 2.0f);
    for (int i = 0; i < lut.blockCount; i++) {
      // (i * 2) to spread out the blocks across if at a low velocity
      // Tried using random() to get X pos, but looked a little too random IMO
      const float x = area.getX() + (static_cast<float>((i * 2) % ANIMATION_BLOCK_DIVISOR) * lut.noteWidthSplit);
      const float y = lut.yDelta + (lut.inverseVelocityLine * mRandom.nextFloat());
      block.setPosition(x, y);
      g.drawRect(block);
    }
  }

  // Pitch class label
  juce::Rectangle<float> labelRect = area.withTrimmedTop(5).withTrimmedLeft(5).withSize(NOTE_LABEL_SIZE, NOTE_LABEL_SIZE);
  g.setColour(keyColor.withSaturation(0.7f));
  g.fillRoundedRectangle(labelRect, 5.0f);
  g.setColour(keyColor.withSaturation(1.0f));
  g.drawRoundedRectangle(labelRect, 5.0f, 1.0f);
  g.setColour(juce::Colours::black);
  g.drawFittedText(PITCH_CLASS_NAMES[pitchClass], labelRect.toNearestInt(), juce::Justification::horizontallyCentred, 1);
}

void RainbowKeyboard::resized() { fillNoteRectangleMap(); }

void RainbowKeyboard::setMidiNotes(const juce::Array<Utils::MidiNote>& midiNotes) {
  mNoteVelocity.fill(0.0f);
  for (const Utils::MidiNote& midiNote : midiNotes) {
    mNoteVelocity[midiNote.pitch] = midiNote.velocity;

    // if note is pressed, update animation LUT
    if (midiNote.velocity > 0.0f) {
      juce::Rectangle<float> area = mNoteRectangleMap[midiNote.pitch];
      AnimationLUT& lut = mAnimationLUT[midiNote.pitch];
      lut.noteWidthSplit = area.getWidth() / static_cast<float>(ANIMATION_BLOCK_DIVISOR);
      lut.inverseVelocityLine = (area.getHeight() * midiNote.velocity);
      lut.velocityLine = area.getHeight() - lut.inverseVelocityLine;
      // velocity^2 to give non linear feel
      // if velocity is low, will be 1 row of blocks
      lut.blockCount = (ANIMATION_BLOCK_DIVISOR / 2) + static_cast<int>(20.0f * midiNote.velocity * midiNote.velocity);
      // substract clip bottom of key
      lut.yDelta = lut.velocityLine - (lut.noteWidthSplit / 2.0f);
    }
  }
}

void RainbowKeyboard::mouseMove(const juce::MouseEvent& e) { updateMouseState(e, false); }

void RainbowKeyboard::mouseDrag(const juce::MouseEvent& e) { updateMouseState(e, true); }

void RainbowKeyboard::mouseDown(const juce::MouseEvent& e) { updateMouseState(e, true); }

void RainbowKeyboard::mouseUp(const juce::MouseEvent& e) { updateMouseState(e, false); }

void RainbowKeyboard::mouseEnter(const juce::MouseEvent& e) {
  // This is NOT called if mouseDrag() is still happening
  updateMouseState(e, false);
}

void RainbowKeyboard::mouseExit(const juce::MouseEvent& e) {
  // This is NOT called if mouseDrag() is still happening
  updateMouseState(e, false);
}

/**
  @brief Only used to inject a note into the midiKeyboard state. The listener
  will set the rainbowKeyboard state

  Takes input of Component::keyStateChanged from parent component due to lack
  of always having focus on this component
*/
void RainbowKeyboard::updateMouseState(const juce::MouseEvent& e, bool isDown) {
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
  repaint();
}

Utils::MidiNote RainbowKeyboard::xyMouseToNote(juce::Point<float> pos) {
  if (!reallyContains(pos.toInt(), false)) return Utils::MidiNote();
  // Since the Juce canvas is all in float, keep in float to prevent strange
  // int-vs-float rounding errors selecting the wrong key
  const float componentHeight = static_cast<float>(getHeight());

  for (Utils::PitchClass pitchClass : Utils::ALL_PITCH_CLASS) {
    if (mNoteRectangleMap[pitchClass].contains(pos)) {
      // TODO: recheck velocity calc for new key design
      float velocity = juce::jmax(0.0f, 1.0f - ((pos.y - mNoteRectangleMap[pitchClass].getY()) / mNoteRectangleMap[pitchClass].getHeight()));
      return Utils::MidiNote(pitchClass, velocity);
    }
  }

  // note not found
  return Utils::MidiNote();
}
