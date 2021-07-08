/*
  ==============================================================================

    RainbowKeyboard.h
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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

  // Returns a value between 0.0-1.0 representing the note's x position on the
  // keyboard
  float getPitchXRatio(int pitchClass);

 private:
  static constexpr int NUM_KEYS = 12;
  static constexpr int INVALID_NOTE = -1;
  static constexpr float BLACK_NOTE_SIZE_RATIO = 0.7f;
  static constexpr int MIDI_CHANNEL = 1;
  const int WHITE_KEY_INDICES[7] = {0, 2, 4, 5, 7, 9, 11};
  const int BLACK_KEY_INDICES[5] = {1, 3, 6, 8, 10};

  inline bool isBlackKey(int pitchClass) {
    return ((1 << (pitchClass)) & 0x054a) != 0;
  }

  juce::MidiKeyboardState& mState;
  int mMouseOverNote = INVALID_NOTE;
  int mPressedNote = INVALID_NOTE;
  bool mIsNotePressed = false;
  float mNoteVelocity = 1.0f;

  // Notes rectangle are recreated on resize and then just become a LUT
  juce::Rectangle<float> mNoteRectangleMap[NUM_KEYS];
  void fillNoteRectangleMap();

  void drawKey(juce::Graphics& g, int pitchClass);
  void updateNoteOver(const juce::MouseEvent& e, bool isDown);
  int xyToNote(juce::Point<float> pos);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowKeyboard)
};
