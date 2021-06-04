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

 private:
  static constexpr auto NUM_KEYS = 12;
  static constexpr auto BLACK_NOTE_SIZE_RATIO = 0.7f;
  static constexpr auto MIDI_CHANNEL = 1;
  static constexpr auto MIDI_CHANNEL_MASK = 0xffff;
  const int WHITE_KEY_INDICES[7] = {0, 2, 4, 5, 7, 9, 11};
  const int BLACK_KEY_INDICES[5] = {1, 3, 6, 8, 10};

  juce::MidiKeyboardState& mState;
  bool shouldCheckState = false;
  int mMouseOverNote = -1;
  int mPressedNote = -1;
  bool mIsNotePressed = false;
  float mNoteVelocity = 1.0f;

  juce::Rectangle<float> getKeyRectangle(int pitchClass);
  void drawKey(juce::Graphics& g, int pitchClass);
  void updateNoteOver(const juce::MouseEvent& e, bool isDown);
  int xyToNote(juce::Point<float> pos);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowKeyboard)
};
