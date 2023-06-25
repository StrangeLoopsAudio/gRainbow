#pragma once

#include "PitchClass.h"

namespace Utils {

// A "Note" is a wrapper to hold all the information about notes from a MidiMessage we care about sharing around classes
struct MidiNote {
  PitchClass pitch;
  float velocity;

  MidiNote() : pitch(PitchClass::NONE), velocity(0.0f) {}
  MidiNote(PitchClass pitch_, float velocity_) : pitch(pitch_), velocity(velocity_) {}

  bool operator==(const MidiNote& other) const { return pitch == other.pitch; }
  bool operator!=(const MidiNote& other) const { return pitch != other.pitch; }
};

}  // namespace Utils
