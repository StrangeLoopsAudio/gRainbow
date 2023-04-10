/*
  ==============================================================================

    RainbowKeyboard.h
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Parameters.h"
#include "../Utils.h"

/**
  @brief The RainbowKeyboard is for displaying the keys being pressed, but all the actual listening of midi inputs will come from
  editor.

  The RainbowKeyboard is able to detect mouse input and inject midi inputs
*/
class RainbowKeyboard : public juce::Component {
 public:
  RainbowKeyboard(juce::MidiKeyboardState& state, Parameters& parameters);
  ~RainbowKeyboard() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void mouseMove(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseEnter(const juce::MouseEvent&) override;
  void mouseExit(const juce::MouseEvent&) override;

  void setMidiNotes(const juce::Array<Utils::MidiNote>& midiNotes);

  // Returns a value between 0.0-1.0 representing the note's x position on the
  // keyboard
  float getPitchXRatio(Utils::PitchClass pitchClass);

 private:
  static constexpr int MIDI_CHANNEL = 1;
  static constexpr float NOTE_BODY_HEIGHT = 0.5f;
  static constexpr float GEN_NODE_HEIGHT = 0.08f;
  static constexpr float NOTE_BODY_SATURATION = 0.5f;
  static constexpr float NOTE_LABEL_SIZE = 22;
  static constexpr float ADD_GEN_SIZE = 18;

  juce::Random mRandom;
  juce::MidiKeyboardState& mState;
  Parameters& mParameters;

  // holds the velocity of each pitch class, if zero, then note is not played
  std::array<float, Utils::PitchClass::COUNT> mNoteVelocity;

  // These allow using the mouse to click a key
  void updateMouseState(const juce::MouseEvent& e, bool isDown, bool isClick);
  Utils::MidiNote xyMouseToNote(juce::Point<float> pos, bool isClick);
  // Note being currently hovered by the mouse
  Utils::MidiNote mHoverNote;
  // Generator being currently hovered by the mouse
  juce::Rectangle<float> mHoverGenRect;
  // Keeps track of the note being played because of the mouse injected input
  Utils::MidiNote mMouseNote;

  // Notes rectangle are recreated on resize and then just become a LUT
  juce::Rectangle<float> mNoteRectMap[Utils::PitchClass::COUNT];
  juce::Rectangle<float> mNoteAddGenRectMap[Utils::PitchClass::COUNT];
  juce::Rectangle<float> mNoteGenRectMap[Utils::PitchClass::COUNT][Utils::NUM_GEN];
  void fillNoteRectangleMap();

  void drawKey(juce::Graphics& g, Utils::PitchClass pitchClass);

  // LUT for animation values to save from recomputing each frame
  static constexpr int ANIMATION_BLOCK_DIVISOR = 12;
  struct AnimationLUT {
    float velocityLine;
    float inverseVelocityLine;
    float noteWidthSplit;
    float blockCount;
    float yDelta;
  };
  std::array<AnimationLUT, Utils::PitchClass::COUNT> mAnimationLUT;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowKeyboard)
};
