/*
  ==============================================================================

    RainbowKeyboard.h
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Utils.h"

//==============================================================================
/*
 */
class RainbowKeyboard : public juce::Component {
 public:
  RainbowKeyboard(juce::MidiKeyboardState& state);
  ~RainbowKeyboard() override;

  void paint(juce::Graphics&) override;
  void resized() override;
  void mouseMove(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseEnter(const juce::MouseEvent&) override;
  void mouseExit(const juce::MouseEvent&) override;

  // Takes input of Component::keyStateChanged from parent component due to lack
  // of always having focus on this component
  void updateKeyState(const juce::KeyPress* pKey, bool isKeyDown);

  // Returns a value between 0.0-1.0 representing the note's x position on the
  // keyboard
  float getPitchXRatio(Utils::PitchClass pitchClass);

 private:
  static constexpr float BLACK_NOTE_SIZE_RATIO = 0.7f;
  static constexpr int MIDI_CHANNEL = 1;

  juce::MidiKeyboardState& mState;
  juce::KeyPress mKeyPresses[Utils::PitchClass::COUNT];
  Utils::PitchClass mLastPressedKey = Utils::PitchClass::NONE;

  // Since only one note can be pressed at once, only need on instance of Note
  Utils::Note mCurrentNote;
  // Note being currently hovered by the mouse
  Utils::Note mHoverNote;

  // Notes rectangle are recreated on resize and then just become a LUT
  juce::Rectangle<float> mNoteRectangleMap[Utils::PitchClass::COUNT];
  void fillNoteRectangleMap();

  void drawKey(juce::Graphics& g, Utils::PitchClass pitchClass);
  void updateMouseState(const juce::MouseEvent& e, bool isDown);
  Utils::Note xyMouseToNote(juce::Point<float> pos);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowKeyboard)
};
