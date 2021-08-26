/*
  ==============================================================================

    Utils.h
    Created: 10 Apr 2021 3:27:51pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <chrono>


namespace Utils {
typedef std::vector<std::vector<float>> SpecBuffer;

// american gold, light silver, metallic bronze, rich purple
static constexpr juce::int64 GENERATOR_COLOURS_HEX[4] = {0xFFD6AF36, 0xFFD7D7D7, 0xFFA77044, 0xFFA34827};

enum FilterType { LOWPASS, HIGHPASS, BANDPASS };

// Tetradic colours
enum GeneratorColour { BLUE = 0, PURPLE, ORANGE, GREEN, NUM_GEN };



enum EnvelopeState { ATTACK, DECAY, SUSTAIN, RELEASE };

// All util logic around the notes/pitchClasses
enum PitchClass { NONE = -1, C = 0, Cs, D, Ds, E, F, Fs, G, Gs, A, As, B, COUNT };

enum FilterType { EMPTY, LOWPASS, HIGHPASS, BANDPASS };

// Use initializer_list to do "for (PitchClass key : ALL_PITCH_CLASS)" logic
static constexpr std::initializer_list<PitchClass> ALL_PITCH_CLASS = {
    PitchClass::C,  PitchClass::Cs, PitchClass::D,  PitchClass::Ds, PitchClass::E,  PitchClass::F,
    PitchClass::Fs, PitchClass::G,  PitchClass::Gs, PitchClass::A,  PitchClass::As, PitchClass::B};

// Constant used for pitch shifting by semitones
static constexpr auto TIMESTRETCH_RATIO = 1.0594f;

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
  float getAmplitude(int curTs, float attack, float decay, float sustain, float release) {
    float newAmp = 0.0f;
    switch (state) {
      case Utils::EnvelopeState::ATTACK: {
        if (noteOnTs < 0) return 0.0f;
        newAmp = (curTs - noteOnTs) / (float)attack;
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

static inline juce::Colour getRainbowColour(int value) {
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
  return juce::Colour(r * 255.0f, g * 255.0f, b * 255.0f);
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
  return juce::Colour(r * 255.0f, g * 255.0f, b * 255.0f);
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
    for (int i = 1; i < int(_xPoints.size()); ++i) {
      jassert(_xPoints[i - 1] < _xPoints[i]);
    }

    _slopes.resize(_xPoints.size() - 1);

    for (int j = 1; j < int(_xPoints.size()); ++j) {
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
};  // namespace Utils
