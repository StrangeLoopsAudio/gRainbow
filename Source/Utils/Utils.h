/*
  ==============================================================================

    Utils.h
    Created: 10 Apr 2021 3:27:51pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <chrono>

namespace Utils {
typedef std::vector<std::vector<float>> SpecBuffer;

// UI spacing and colours
static constexpr int EDITOR_WIDTH = 1000;
static constexpr int EDITOR_HEIGHT = 550;
static constexpr int PANEL_WIDTH = 270;
static constexpr int KEYBOARD_HEIGHT = 200;
static constexpr int PADDING = 6;
static constexpr float PADDING_F = 6.0f;
static constexpr int TITLE_HEIGHT = 17;
static constexpr int LABEL_HEIGHT = TITLE_HEIGHT;
static constexpr int KNOB_WIDTH = (PANEL_WIDTH - (PADDING * 2)) / 4;
static constexpr int KNOB_HEIGHT = KNOB_WIDTH / 2;
static constexpr float ROUNDED_AMOUNT = 6.0f;
static const juce::Colour GLOBAL_COLOUR = juce::Colours::black;
static constexpr float GENERATOR_BRIGHTNESS_ADD = 0.2f;  // Amount to make brighter per generator

static const juce::ColourGradient BG_GRADIENT = juce::ColourGradient(juce::Colours::lightcyan, EDITOR_WIDTH / 2, 0, juce::Colours::skyblue, EDITOR_WIDTH / 2, EDITOR_HEIGHT, false);

static constexpr auto ENV_LUT_SIZE = 64;  // grain env lookup table size

static inline juce::ColourGradient getBgGradient(juce::Rectangle<int> boundsRelativeToEditor) {
  juce::ColourGradient grad = BG_GRADIENT;
  grad.point1.y = grad.point1.y - static_cast<float>(boundsRelativeToEditor.getY());
  grad.point2.y = grad.point2.y - static_cast<float>(boundsRelativeToEditor.getY());
  return grad;
}

typedef struct Result {
  bool success;
  juce::String message;
} Result;

// Number of generators available
static constexpr int NUM_GEN = 4;
// Constant used for pitch shifting by semitones
static constexpr auto TIMESTRETCH_RATIO = 1.0594f;

// All util logic around the notes/pitchClasses
enum PitchClass { NONE = -1, C = 0, Cs, D, Ds, E, F, Fs, G, Gs, A, As, B, COUNT };
// Use initializer_list to do "for (PitchClass key : ALL_PITCH_CLASS)" logic
static constexpr std::initializer_list<PitchClass> ALL_PITCH_CLASS = {
    PitchClass::C,  PitchClass::Cs, PitchClass::D,  PitchClass::Ds, PitchClass::E,  PitchClass::F,
    PitchClass::Fs, PitchClass::G,  PitchClass::Gs, PitchClass::A,  PitchClass::As, PitchClass::B};

// Slightly different from Parameters::PITCH_CLASS_NAMES for displaying to user, (e.g, replaces Cs with C#)
static juce::Array<juce::String> PITCH_CLASS_DISP_NAMES{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

static juce::Array<juce::String> SpecTypeNames{"Spectrogram", "Harmonic Profile", "Detected Pitches", "Audio Waveform"};

enum EnvelopeState { ATTACK, DECAY, SUSTAIN, RELEASE };
enum FilterType { NO_FILTER, LOWPASS, HIGHPASS, BANDPASS };

static inline PitchClass getPitchClass(int midiNoteNumber) { return (PitchClass)(midiNoteNumber % PitchClass::COUNT); }
// A "Note" is a wrapper to hold all the information about notes from a MidiMessage we care about sharing around classes
struct MidiNote {
  PitchClass pitch;
  float velocity;

  MidiNote() : pitch(PitchClass::NONE), velocity(0.0f) {}
  MidiNote(PitchClass pitch_, float velocity_) : pitch(pitch_), velocity(velocity_) {}

  bool operator==(const MidiNote& other) const { return pitch == other.pitch; }
  bool operator!=(const MidiNote& other) const { return pitch != other.pitch; }
};

typedef struct EnvelopeADSR {
  // All adsr params are in samples (except for sustain amp)
  EnvelopeState state = EnvelopeState::ATTACK;
  float amplitude = 0.0f;
  float noteOffAmplitude = 0.0f;
  int noteOnTs = -1;
  int noteOffTs = -1;
  EnvelopeADSR() {}
  EnvelopeADSR(int ts) { noteOn(ts); }
  void noteOn(int ts) {
    noteOnTs = ts;
    noteOffTs = -1;
    state = EnvelopeState::ATTACK;
    amplitude = 0.0f;
    noteOffAmplitude = 0.0f;
  }
  void noteOff(int ts) {
    noteOnTs = -1;
    noteOffTs = ts;
    noteOffAmplitude = amplitude;
    state = EnvelopeState::RELEASE;
  }
  /* ADSR params (except sustain) should be in samples */
  float getAmplitude(long curTs, float attack, float decay, float sustain, float release) {
    float newAmp = 0.0f;
    switch (state) {
      case Utils::EnvelopeState::ATTACK: {
        if (noteOnTs < 0) return 0.0f;
        newAmp = (curTs - noteOnTs) / attack;
        if ((curTs - noteOnTs) >= attack) {
          state = Utils::EnvelopeState::DECAY;
        }
        break;
      }
      case Utils::EnvelopeState::DECAY: {
        if (noteOnTs < 0) return 0.0f;
        newAmp = 1.0f - ((curTs - noteOnTs - attack) / (float)decay) * (1.0f - sustain);
        if (curTs - noteOnTs - attack >= decay) {
          state = Utils::EnvelopeState::SUSTAIN;
        }
        break;
      }
      case Utils::EnvelopeState::SUSTAIN: {
        newAmp = sustain;
        // Note: setting note off sets state to release, we don't need to
        // here
        break;
      }
      case Utils::EnvelopeState::RELEASE: {
        if (noteOffTs < 0) return 0.0f;
        newAmp = noteOffAmplitude - (((curTs - noteOffTs) / (float)release) * noteOffAmplitude);
        if ((curTs - noteOffTs) > release) {
          noteOffTs = -1;
        }
        break;
      }
    }
    amplitude = juce::jlimit(0.0f, 1.0f, newAmp);
    return amplitude;
  }
} EnvelopeADSR;

template <typename CompType, typename CompAttachment>
class AttachedComponent {
 public:
  AttachedComponent<CompType, CompAttachment>(juce::RangedAudioParameter& param, juce::Component& parent,
                                              std::function<void(CompType&)> init = nullptr) {
    attachment.reset(new CompAttachment(param, component));
    parent.addAndMakeVisible(component);
    if (init != nullptr) init(component);
    attachment->sendInitialUpdate();
  }
  CompType component;

 private:
  std::unique_ptr<CompAttachment> attachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AttachedComponent)
};

static inline juce::Colour getRainbow6Colour(int value) {
  jassert(value >= 0 && value <= 6);
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  switch (value) {
    case 0:  // red
      r = 1.0f;
      g = 0.0f;
      b = 0.0f;
      break;
    case 1:  // orange
      r = 1.0f;
      g = 0.49f;
      b = 0.f;
      break;
    case 2:  // yellow
      r = 1.0f;
      g = 1.0f;
      b = 0.0f;
      break;
    case 3:  // green
      r = 0.0f;
      g = 1.0f;
      b = 0.0f;
      break;
    case 4:  // blue
      r = 0.0f;
      g = 0.0f;
      b = 1.0f;
      break;
    case 5:  // indigo
      r = 0.29f;
      g = 0.0f;
      b = 0.5f;
      break;
    case 6:  // violet
      r = 0.58f;
      g = 0.0f;
      b = 0.83f;
      break;
    default:
      break;
  }
  return juce::Colour::fromFloatRGBA(r, g, b, 1.0f);
}

static inline juce::Colour getRainbow12Colour(int value) {
  jassert(value >= 0 && value <= 11);
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  switch (value) {
    case 0:  // red
      r = 1.0f;
      g = 0.0f;
      b = 0.0f;
      break;
    case 1:  // orange
      r = 1.0f;
      g = 0.5f;
      b = 0.0f;
      break;
    case 2:  // yellow
      r = 1.0f;
      g = 1.0f;
      b = 0.0f;
      break;
    case 3:  // chartreuse
      r = 0.5f;
      g = 1.0f;
      b = 0.0f;
      break;
    case 4:  // green
      r = 0.0f;
      g = 1.0f;
      b = 0.0f;
      break;
    case 5:  // spring green
      r = 0.0f;
      g = 1.0f;
      b = 0.5f;
      break;
    case 6:  // cyan
      r = 0.0f;
      g = 1.0f;
      b = 1.0f;
      break;
    case 7:  // dodger blue
      r = 0.0f;
      g = 0.5f;
      b = 1.0f;
      break;
    case 8:  // blue
      r = 0.0f;
      g = 0.0f;
      b = 1.0f;
      break;
    case 9:  // purple
      r = 0.5f;
      g = 0.0f;
      b = 1.0f;
      break;
    case 10:  // violet
      r = 1.0f;
      g = 0.0f;
      b = 1.0f;
      break;
    case 11:  // magenta
      r = 1.0f;
      g = 0.0f;
      b = 0.5f;
      break;
    default:
      break;
  }
  return juce::Colour::fromFloatRGBA(r, g, b, 1.0f);
}

[[maybe_unused]] static const std::vector<float> getGrainEnvelopeLUT(const float shape, const float tilt) {
  std::vector<float> lut;
  /* LUT divided into 3 parts

               1.0
              -----
     rampUp  /     \  rampDown
            /       \
  */
  float scaledShape = (shape * ENV_LUT_SIZE) / 2.0f;
  float scaledTilt = tilt * ENV_LUT_SIZE;
  int rampUpEndSample = juce::jmax(0.0f, scaledTilt - scaledShape);
  int rampDownStartSample = juce::jmin((float)ENV_LUT_SIZE, scaledTilt + scaledShape);
  for (int i = 0; i < ENV_LUT_SIZE; i++) {
    if (i < rampUpEndSample) {
      lut.push_back(static_cast<float>(i / rampUpEndSample));
    } else if (i > rampDownStartSample) {
      lut.push_back(1.0f - (float)(i - rampDownStartSample) / (ENV_LUT_SIZE - rampDownStartSample));
    } else {
      lut.push_back(1.0f);
    }
  }
  juce::FloatVectorOperations::clip(lut.data(), lut.data(), 0.0f, 1.0f, lut.size());
  return lut;
}

template <class TimeT = std::chrono::milliseconds, class ClockT = std::chrono::steady_clock>
class Timer {
  using timep_t = typename ClockT::time_point;
  timep_t _start = ClockT::now(), _end = {};

 public:
  void tick() {
    _end = timep_t{};
    _start = ClockT::now();
  }

  void tock() { _end = ClockT::now(); }

  template <class TT = TimeT>
  TT duration() const {
    return std::chrono::duration_cast<TT>(_end - _start);
  }
};

static inline float db2lin(float value) { return std::pow(10.0f, value / 10.0f); }
static inline float lin2db(float value) { return value < 1e-10 ? -100 : 10.0f * std::log10(value); }

// Ripped from essentia utils
class BPF {
 protected:
  std::vector<float> _xPoints;
  std::vector<float> _yPoints;
  std::vector<float> _slopes;

 public:
  BPF() {}
  BPF(std::vector<float> xPoints, std::vector<float> yPoints) { init(xPoints, yPoints); }
  void init(std::vector<float> xPoints, std::vector<float> yPoints) {
    _xPoints = xPoints;
    _yPoints = yPoints;

    jassert(_xPoints.size() == _yPoints.size());
    jassert(_xPoints.size() >= 2);
    for (size_t i = 1; i < _xPoints.size(); ++i) {
      jassert(_xPoints[i - 1] < _xPoints[i]);
    }

    _slopes.resize(_xPoints.size() - 1);

    for (size_t j = 1; j < _xPoints.size(); ++j) {
      // this never gives a division by zero as we checked just before that
      // x[i-1] < x[i]
      _slopes[j - 1] = (_yPoints[j] - _yPoints[j - 1]) / (_xPoints[j] - _xPoints[j - 1]);
    }
  }

  inline float operator()(float x) {
    jassert(x >= _xPoints[0]);
    jassert(x <= _xPoints.back());

    std::vector<float>::size_type j = 0;
    while (x > _xPoints[j + 1]) {
      j += 1;
    }

    return (x - _xPoints[j]) * _slopes[j] + _yPoints[j];
  }
};

}  // namespace Utils
