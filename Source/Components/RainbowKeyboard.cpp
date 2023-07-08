/*
  ==============================================================================

    RainbowKeyboard.cpp
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#include "RainbowKeyboard.h"
#include "Settings.h"
#include "BinaryData.h"
#include "Utils/Colour.h"

//==============================================================================
RainbowKeyboard::RainbowKeyboard(juce::MidiKeyboardState& state, Parameters& parameters) : mState(state), mParameters(parameters) {
  mNoteVelocity.fill(0.0f);
  mRandom.setSeed(juce::Time::currentTimeMillis());
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
  //g.fillAll(juce::Colours::transparentBlack);

  // Draw each key and its active generators
  for (Utils::PitchClass key : Utils::ALL_PITCH_CLASS) {
    drawKey(g, key);
  }

  // Nothing else is needed while loading
  if (!mParameters.ui.specComplete) return;

  // Make return button if anything other than global is selected
  if (mParameters.selectedParams->type != ParamType::GLOBAL) {
    juce::Path btnReturnPath;
    const int width = getWidth();
    const float halfRound = Utils::ROUNDED_AMOUNT / 2.0f;
    btnReturnPath.startNewSubPath(0, BTN_RETURN_HEIGHT);
    btnReturnPath.lineTo(0, Utils::ROUNDED_AMOUNT);
    btnReturnPath.cubicTo(0, halfRound, halfRound, 0, Utils::ROUNDED_AMOUNT, 0);
    btnReturnPath.lineTo(width - Utils::ROUNDED_AMOUNT, 0);
    btnReturnPath.cubicTo(width - halfRound, 0, width, halfRound, width, Utils::ROUNDED_AMOUNT);
    btnReturnPath.lineTo(width, BTN_RETURN_HEIGHT);
    btnReturnPath.closeSubPath();
    g.setColour(mIsHoverBtnReturn ? Utils::GLOBAL_COLOUR.brighter() : Utils::GLOBAL_COLOUR);
    g.fillPath(btnReturnPath);
    // Draw return text over the top
    g.setColour(juce::Colours::white);
    auto returnBarRect = getLocalBounds().removeFromTop(BTN_RETURN_HEIGHT);
    g.drawText(TEXT_RETURN, returnBarRect, juce::Justification::centred);
    g.setColour(mParameters.getSelectedParamColour().interpolatedWith(juce::Colours::white, 0.6f));
    g.drawText(TEXT_EDITING_PRE + PARAM_TYPE_NAMES[mParameters.selectedParams->type] + TEXT_EDITING_POST, returnBarRect.withTrimmedLeft(Utils::PADDING), juce::Justification::left);
    g.drawText(TEXT_EDITING_PRE + PARAM_TYPE_NAMES[mParameters.selectedParams->type] + TEXT_EDITING_POST, returnBarRect.withTrimmedRight(Utils::PADDING), juce::Justification::right);
  }
}

void RainbowKeyboard::fillNoteRectangleMap() {
  // Keep everything in float to match Rectangle type
  const float componentHeight = static_cast<float>(getHeight());

  juce::Rectangle<float> r = getLocalBounds().toFloat();

  r.removeFromTop(BTN_RETURN_HEIGHT);

  r = r.reduced(Utils::PADDING_F, Utils::PADDING_F).withCentre(r.getCentre());

  // key width = leftover width after padding / num pitch classes * component width
  const float keyWidth =
      (r.getWidth() - (Utils::PADDING_F * (Utils::PitchClass::COUNT - 1))) / static_cast<float>(Utils::PitchClass::COUNT);
  const float keyHeight = componentHeight * NOTE_BODY_HEIGHT;
  int genHeight = r.getHeight() * (1.0f - NOTE_BODY_HEIGHT) / 5;

  for (Utils::PitchClass key : Utils::ALL_PITCH_CLASS) {
    juce::Rectangle<float> keyRect = r.removeFromLeft(keyWidth);
    // Note body
    mNoteRectMap[key] = keyRect.removeFromBottom(keyHeight);

    // Generators on top of the note
    for (int i = 0; i < NUM_GENERATORS; ++i) {
      const auto genRect = keyRect.removeFromBottom(genHeight);
      mNoteGenRectMap[key][i] = genRect;
      const float offset = genRect.getWidth() * .8f;
      mNoteGenRectIncreaseMap[key][i] = genRect.withTrimmedLeft(offset);
      mNoteGenRectDecreaseMap[key][i] = genRect.withTrimmedRight(offset);
    }

    // Make add generator button for each note
    mNoteAddGenRectMap[key] = mNoteRectMap[key]
                                  .withSizeKeepingCentre(ADD_GEN_SIZE, ADD_GEN_SIZE)
                                  .withBottomY(mNoteRectMap[key].getY() -
                                               (genHeight * mParameters.note.notes[key]->getNumEnabledGens()) - Utils::PADDING_F);

    r.removeFromLeft(Utils::PADDING_F);
  }
}

void RainbowKeyboard::drawKey(juce::Graphics& g, Utils::PitchClass pitchClass) {
  const float velocity = mNoteVelocity[pitchClass];
  const bool isDown = (velocity > 0.0f);
  const bool isPitchSelected = mParameters.selectedParams == mParameters.note.notes[pitchClass].get();
  const ParamNote& paramNote = *mParameters.note.notes[pitchClass];
  const bool noCandidates = paramNote.candidates.empty();

  const juce::Colour keyColour = Utils::getRainbow12Colour(pitchClass);
  juce::Colour bodyColour = keyColour.withSaturation(NOTE_BODY_SATURATION);
  const juce::Colour borderColour = keyColour.darker();

  // if down, extra dark
  // if no note is down, lightly darken if mouse is hovering it
  if (noCandidates) {
    bodyColour = juce::Colours::grey;
  } else if (isDown) {
    bodyColour = bodyColour.darker().darker();
  } else if (pitchClass == mHoverNote.pitch) {
    bodyColour = bodyColour.darker();
  }

  g.setColour(noCandidates ? juce::Colours::grey : (isPitchSelected ? keyColour.darker() : bodyColour));

  juce::Rectangle<float> area = mNoteRectMap[pitchClass];
  g.fillRoundedRectangle(area, Utils::ROUNDED_AMOUNT);
  g.setColour(borderColour);
  g.drawRoundedRectangle(area, Utils::ROUNDED_AMOUNT, 2.0f);

  // Pitch class label
  juce::Rectangle<float> labelRect = area.withTrimmedTop(5).withTrimmedLeft(5).withSize(NOTE_LABEL_SIZE, NOTE_LABEL_SIZE);
  g.setColour(noCandidates ? juce::Colours::grey : keyColour.withSaturation(0.4f));
  g.fillRoundedRectangle(labelRect, 5.0f);
  g.setColour(borderColour);
  g.drawRoundedRectangle(labelRect, 5.0f, 2.0f);
  g.setColour(juce::Colours::black);
  g.drawFittedText(Utils::PITCH_CLASS_DISP_NAMES[pitchClass], labelRect.toNearestInt(), juce::Justification::horizontallyCentred,
                   1);

  // If there are no candidates, don't show anything else as it will be confusing that nothing is working
  if (noCandidates) {
    return;
  }

  // Draw the active generators on top of each key
  const int numEnabledGens = paramNote.getNumEnabledGens();
  int genHeight = getHeight() * (1.0f - NOTE_BODY_HEIGHT) / 4;
  juce::Rectangle<int> genRect = juce::Rectangle<int>(area.getWidth(), genHeight);

  for (int i = 0; i < numEnabledGens; ++i) {
    ParamGenerator* gen = paramNote.getEnabledGenByIdx(i);
    jassert(gen != nullptr);
    const bool isGenSelected = mParameters.selectedParams == gen;
    const auto& noteGenRect = mNoteGenRectMap[pitchClass][i];
    const bool isGenHovering = (mHoverGenRect == noteGenRect);

    juce::Colour genColour = keyColour.withSaturation(NOTE_BODY_SATURATION).brighter((i+1) * Utils::GENERATOR_BRIGHTNESS_ADD);
    if (isGenHovering) genColour = keyColour.withSaturation(NOTE_BODY_SATURATION).darker();
    if (isGenSelected) genColour = keyColour.darker();

    g.setColour(genColour);
    g.fillRoundedRectangle(noteGenRect, Utils::ROUNDED_AMOUNT);
    g.setColour(borderColour);
    g.drawRoundedRectangle(noteGenRect, Utils::ROUNDED_AMOUNT, 2.0f);
    g.setColour(juce::Colours::black);
    g.drawFittedText(juce::String(gen->candidate->get() + 1), noteGenRect.toNearestInt(), juce::Justification::centred, 1);

    if (isGenHovering) {
      const auto rightBox = mNoteGenRectIncreaseMap[pitchClass][i];
      const auto leftBox = mNoteGenRectDecreaseMap[pitchClass][i];
      g.setColour(mHoverGenIncrease ? keyColour : keyColour.withAlpha(0.5f));
      g.fillRoundedRectangle(rightBox, Utils::ROUNDED_AMOUNT);
      g.setColour(mHoverGenDecrease ? keyColour : keyColour.withAlpha(0.5f));
      g.fillRoundedRectangle(leftBox, Utils::ROUNDED_AMOUNT);
      g.setColour(juce::Colours::black);
      g.drawFittedText({">"}, rightBox.toNearestInt(), juce::Justification::centred, 1);
      g.drawFittedText({"<"}, leftBox.toNearestInt(), juce::Justification::centred, 1);
    }
  }

  // Draw the add generator button if more can still be added
  // Also disable add button if note or gen is selected (for neatness)
  if (numEnabledGens < NUM_GENERATORS) {
    const auto& addGenRect = mNoteAddGenRectMap[pitchClass];
    juce::Colour addColour =
        (mHoverGenRect == addGenRect) ? keyColour.withSaturation(0.4f).darker() : keyColour.withSaturation(0.4f);
    g.setColour(addColour);
    g.fillRoundedRectangle(addGenRect, 5.0f);
    g.setColour(borderColour);
    g.drawRoundedRectangle(addGenRect, 5.0f, 2.0f);
    g.setColour(juce::Colours::black);
    g.drawText("+", addGenRect, juce::Justification::centred);
  }

  // display animation to show the velocity level of the note
  if (isDown && PowerUserSettings::get().getAnimated()) {
    AnimationLUT& lut = mAnimationLUT[pitchClass];
    juce::Rectangle<float> block = juce::Rectangle<float>(lut.noteWidthSplit / 2.0f, lut.noteWidthSplit);
    g.setColour(juce::Colours::white);
    g.drawLine(area.getX(), lut.velocityLine, area.getRight(), lut.velocityLine, 2.0f);
    for (int i = 0; i < lut.blockCount; i++) {
      // (i * 2) to spread out the blocks across if at a low velocity
      // Tried using random() to get X pos, but looked a little too random IMO
      const float x = area.getX() + (static_cast<float>((i * 2) % ANIMATION_BLOCK_DIVISOR) * lut.noteWidthSplit);
      const float y = lut.yDelta + (lut.velocityLine * mRandom.nextFloat());
      block.setPosition(x, y);
      g.drawRect(block);
    }
  }
}

void RainbowKeyboard::resized() { fillNoteRectangleMap(); }

void RainbowKeyboard::setMidiNotes(const juce::Array<Utils::MidiNote>& midiNotes) {
  mNoteVelocity.fill(0.0f);
  for (const Utils::MidiNote& midiNote : midiNotes) {
    mNoteVelocity[midiNote.pitch] = midiNote.velocity;

    // if note is pressed, update animation LUT
    if (midiNote.velocity > 0.0f) {
      juce::Rectangle<float> area = mNoteRectMap[midiNote.pitch];
      AnimationLUT& lut = mAnimationLUT[midiNote.pitch];
      lut.noteWidthSplit = area.getWidth() / static_cast<float>(ANIMATION_BLOCK_DIVISOR);
      lut.inverseVelocityLine = (area.getHeight() * midiNote.velocity);
      const float bottom = area.getHeight() + area.getY();
      lut.velocityLine = bottom - lut.inverseVelocityLine;
      // velocity^2 to give non linear feel
      // if velocity is low, will be 1 row of blocks
      lut.blockCount = (ANIMATION_BLOCK_DIVISOR / 2) + static_cast<int>(20.0f * midiNote.velocity * midiNote.velocity);
      // substract have a box so it doesn't clip bottom of key
      lut.yDelta = lut.velocityLine - (lut.noteWidthSplit / 2.0f);
    }
  }
}

void RainbowKeyboard::mouseMove(const juce::MouseEvent& e) { updateMouseState(e, false, false); }

void RainbowKeyboard::mouseDrag(const juce::MouseEvent& e) { updateMouseState(e, true, false); }

void RainbowKeyboard::mouseDown(const juce::MouseEvent& e) { updateMouseState(e, true, false); }

void RainbowKeyboard::mouseUp(const juce::MouseEvent& e) {
  updateMouseState(e, false, true);
  if (mHoverGenRect != juce::Rectangle<float>() && e.mods.isRightButtonDown()) {
    juce::PopupMenu menu;
    menu.addItem(1, "Delete generator", true);
    menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
      if (result == 1) {
        // Disable the currently selected generator
        ParamGenerator* gen = dynamic_cast<ParamGenerator*>(mParameters.selectedParams);
        jassert(gen != nullptr);
        ParamHelper::setParam(gen->enable, false);
        mParameters.selectedParams = &mParameters.global;
        if (mParameters.onSelectedChange != nullptr) mParameters.onSelectedChange();
      }
    });
  }
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

  // reset each update
  mHoverGenRect = juce::Rectangle<float>();
  mHoverGenIncrease = false;
  mHoverGenDecrease = false;

  mHoverNote = xyMouseToNote(pos, isClick);

  // Test for return button hover/click
  if (mParameters.selectedParams->type != ParamType::GLOBAL &&
      getLocalBounds().removeFromTop(BTN_RETURN_HEIGHT).contains(pos.toInt())) {
    mIsHoverBtnReturn = true;
    if (isClick) {
      mParameters.selectedParams = &mParameters.global;
      if (mParameters.onSelectedChange != nullptr) mParameters.onSelectedChange();
      mIsHoverBtnReturn = false;
    }
  } else {
    mIsHoverBtnReturn = false;
  }

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
  repaint();
}

void RainbowKeyboard::generatorOnClick(ParamGenerator* gen) {
  if (mHoverGenIncrease || mHoverGenDecrease) {
    const size_t numCandidates = mParameters.note.notes[gen->noteIdx]->candidates.size();
    if (numCandidates == 0) return;
    const int pos = gen->candidate->get();
    int newPos = mHoverGenIncrease ? pos + 1 : pos - 1;
    newPos = (newPos + numCandidates) % numCandidates;
    ParamHelper::setParam(gen->candidate, newPos);
  }
  // If you have selected Gen "X" and then Increase on gen "Y", it will swap to "Y" on top of increasing the candidate value
  if (mParameters.selectedParams != gen) {
    mParameters.selectedParams = gen;
    if (mParameters.onSelectedChange != nullptr) {
      mParameters.onSelectedChange();
    }
  }
}

Utils::MidiNote RainbowKeyboard::xyMouseToNote(juce::Point<float> pos, bool isClick) {
  if (!reallyContains(pos.toInt(), false)) return Utils::MidiNote();

  for (Utils::PitchClass pitchClass : Utils::ALL_PITCH_CLASS) {
    ParamNote* note = mParameters.note.notes[pitchClass].get();
    // First check general note area
    const auto& noteRect = mNoteRectMap[pitchClass];
    if (noteRect.contains(pos.withY(noteRect.getCentreY()))) {
      // Now check note body and generators
      if (noteRect.contains(pos)) {
        float velocity = juce::jmax(0.0f, 1.0f - ((pos.y - noteRect.getY()) / noteRect.getHeight()));
        return Utils::MidiNote(pitchClass, velocity);
      } else {
        // Check generators and add gen button for hover
        for (int i = 0; i < note->getNumEnabledGens(); ++i) {
          if (mNoteGenRectMap[pitchClass][i].contains(pos)) {
            mHoverGenIncrease = mNoteGenRectIncreaseMap[pitchClass][i].contains(pos);
            mHoverGenDecrease = mNoteGenRectDecreaseMap[pitchClass][i].contains(pos);
            mHoverGenRect = mNoteGenRectMap[pitchClass][i];
            if (isClick) {
              generatorOnClick(note->getEnabledGenByIdx(i));
            }
            repaint();
            return Utils::MidiNote();
          }
        }
        if (mNoteAddGenRectMap[pitchClass].contains(pos) && note->getNumEnabledGens() < NUM_GENERATORS) {
          if (isClick) {
            // Add another generator
            note->enableNextAvailableGen();
            resized();
          }
          mHoverGenRect = mNoteAddGenRectMap[pitchClass];
          return Utils::MidiNote();
        }
      }
    }
  }

  // note not found
  return Utils::MidiNote();
}
