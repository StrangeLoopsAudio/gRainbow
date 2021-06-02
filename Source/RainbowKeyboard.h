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
class RainbowKeyboard : public juce::Component,
                        juce::Timer,
                        juce::MidiKeyboardState::Listener,
                        juce::ChangeBroadcaster {
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
  void timerCallback() override;
  void focusLost(FocusChangeType) override;
  void handleNoteOn(juce::MidiKeyboardState*, int midiChannel,
                    int midiNoteNumber, float velocity) override;
  void handleNoteOff(juce::MidiKeyboardState*, int midiChannel,
                     int midiNoteNumber, float velocity) override;

 private:
  static constexpr auto NUM_KEYS = 12;
  static constexpr auto BLACK_INDICES = {1, 4, 6, 9, 11};
  static constexpr auto BLACK_NOTE_SIZE_RATIO = 0.7f;
  static constexpr auto KEY_WIDTH = 16.0f;
  static constexpr auto MIDI_CHANNEL = 1;
  static constexpr auto MIDI_CHANNEL_MASK = 0xffff;

  juce::MidiKeyboardState& mState;
  bool shouldCheckState = false;
  int mMouseOverNote = -1;
  float velocity = 1.0f;

  juce::Rectangle<float> getKeyRectangle(int pitchClass);
  void drawKey(juce::Graphics& g, int pitchClass, juce::Rectangle<float> area);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowKeyboard)
};
