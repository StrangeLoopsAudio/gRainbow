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
#include "Parameters.h"
#include "Utils/Utils.h"
#include "Utils/MidiNote.h"
#include "Utils/PitchClass.h"

/**
  @brief The RainbowKeyboard is for displaying the keys being pressed, but all the actual listening of midi inputs will come from
  editor.

  The RainbowKeyboard is able to detect mouse input and inject midi inputs
*/
class RainbowKeyboard : public juce::Component, juce::AudioProcessorParameter::Listener {
 public:
  RainbowKeyboard(juce::MidiKeyboardState& state, Parameters& parameters);
  ~RainbowKeyboard() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int, bool) override {}

  void mouseMove(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseEnter(const juce::MouseEvent&) override;
  void mouseExit(const juce::MouseEvent&) override;

  void setMidiNotes(const juce::Array<Utils::MidiNote>& midiNotes);

 private:
  static constexpr int MIDI_CHANNEL = 1;
  static constexpr float BLACK_NOTE_SIZE_RATIO = 0.7f;
  static constexpr int GLOBAL_RECT_HEIGHT = 18;
  
  // If a key is black or white, have to know this to paint the UI
  Utils::PitchClass WHITE_KEYS_PITCH_CLASS[7] = {Utils::PitchClass::C, Utils::PitchClass::D, Utils::PitchClass::E,
    Utils::PitchClass::F, Utils::PitchClass::G, Utils::PitchClass::A,
    Utils::PitchClass::B};
  
  Utils::PitchClass BLACK_KEYS_PITCH_CLASS[5] = {Utils::PitchClass::Cs, Utils::PitchClass::Ds, Utils::PitchClass::Fs,
    Utils::PitchClass::Gs, Utils::PitchClass::As};

  static inline bool isBlackKey(Utils::PitchClass pitchClass) { return ((1 << (pitchClass)) & 0x054a) != 0; }
  // Returns a value between 0.0-1.0 representing the note's x position on the keyboard
  float getPitchXRatio(Utils::PitchClass pitchClass);
  
  // These allow using the mouse to click a key
  void updateMouseState(const juce::MouseEvent& e, bool isDown, bool isClick);
  void generatorOnClick(ParamGenerator* gen);
  Utils::MidiNote xyMouseToNote(juce::Point<float> pos);
  
  // Preparing and drawing piano note areas
  void fillNoteRectangleMap();
  void drawKey(juce::Graphics& g, Utils::PitchClass pitchClass);

  // Bookkeeping
  juce::MidiKeyboardState& mState;
  Parameters& mParameters;
  // holds the velocity of each pitch class, if zero, then note is not played
  std::array<float, Utils::PitchClass::COUNT> mNoteVelocity;

  Utils::MidiNote mHoverNote; // Note being currently hovered by the mouse
  Utils::MidiNote mMouseNote; // The note being played because of the mouse injected input

  // Notes rectangle are recreated on resize and then just become a LUT
  juce::Rectangle<float> mNoteRectMap[Utils::PitchClass::COUNT];
  juce::Rectangle<float> mGlobalRect;
  bool mHoverGlobal = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowKeyboard)
};
