#pragma once

#include <juce_core/juce_core.h>

namespace Utils {

// All util logic around the notes/pitchClasses
enum PitchClass { NONE = -1, C = 0, Cs, D, Ds, E, F, Fs, G, Gs, A, As, B, COUNT };
// Use initializer_list to do "for (PitchClass key : ALL_PITCH_CLASS)" logic
static constexpr std::initializer_list<PitchClass> ALL_PITCH_CLASS = {
    PitchClass::C,  PitchClass::Cs, PitchClass::D,  PitchClass::Ds, PitchClass::E,  PitchClass::F,
    PitchClass::Fs, PitchClass::G,  PitchClass::Gs, PitchClass::A,  PitchClass::As, PitchClass::B};

// Slightly different from Parameters::PITCH_CLASS_NAMES for displaying to user, (e.g, replaces Cs with C#)
static juce::Array<juce::String> PITCH_CLASS_DISP_NAMES{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

static inline PitchClass getPitchClass(int midiNoteNumber) { return (PitchClass)(midiNoteNumber % PitchClass::COUNT); }

}  // namespace Utils
