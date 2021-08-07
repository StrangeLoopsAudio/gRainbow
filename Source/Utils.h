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

class Utils {
 public:
  // Tetradic colours
  enum GeneratorColour { BLUE = 0, PURPLE, ORANGE, GREEN, NUM_GEN };
  enum EnvelopeState { ATTACK, DECAY, SUSTAIN, RELEASE };
  enum SpecType { LOGO, SPECTROGRAM, HPCP, NOTES, NUM_TYPES };
  // All util logic around the notes/pitchClasses
  enum PitchClass {
    NONE = -1,
    C = 0,
    Cs,
    D,
    Ds,
    E,
    F,
    Fs,
    G,
    Gs,
    A,
    As,
    B,
    COUNT
  };
  // Use initializer_list to do "for (PitchClass key : ALL_PITCH_CLASS)" logic
  static constexpr std::initializer_list<PitchClass> ALL_PITCH_CLASS = {
      PitchClass::C,  PitchClass::Cs, PitchClass::D,  PitchClass::Ds,
      PitchClass::E,  PitchClass::F,  PitchClass::Fs, PitchClass::G,
      PitchClass::Gs, PitchClass::A,  PitchClass::As, PitchClass::B};

  static constexpr PitchClass WHITE_KEYS_PITCH_CLASS[7] = {
      PitchClass::C, PitchClass::D, PitchClass::E, PitchClass::F,
      PitchClass::G, PitchClass::A, PitchClass::B};
  static constexpr PitchClass BLACK_KEYS_PITCH_CLASS[5] = {
      PitchClass::Cs, PitchClass::Ds, PitchClass::Fs, PitchClass::Gs,
      PitchClass::As};
  static constexpr auto MAX_POSITIONS = 6;
  static constexpr juce::int64 POSITION_COLOURS[4] = {0xFF52C4FF, 0xFFE352FF,
                                                      0xFFFF8D52, 0xFF6EFF52};
  static constexpr auto TIMESTRETCH_RATIO =
      1.0594f;  // Constant used for pitch shifting by semitones
  static constexpr auto FILE_RECORDING = "gRainbow_user_recording.wav";
  static constexpr auto FILE_SPECTROGRAM = "gRainbow_spec.png";
  static constexpr auto FILE_HPCP = "gRainbow_hpcp.png";
  static constexpr auto FILE_NOTES = "gRainbow_notes.png";
  static constexpr float INVALID_VELOCITY = 0.0f;
  struct Note {
    PitchClass pitch;
    float velocity;

    Note() : pitch(PitchClass::NONE), velocity(INVALID_VELOCITY) {}
    Note(PitchClass pitch, float velocity) : pitch(pitch), velocity(velocity) {}
  };

  typedef struct GeneratorState {
    bool isEnabled = false;
    bool isSolo = false;
    bool isWaiting = false; // Waiting for other gen to unsolo
    GeneratorState() {}
    GeneratorState(bool isEnabled, bool isSolo)
        : isEnabled(isEnabled), isSolo(isSolo) {}
    bool shouldPlay() { return (isEnabled && !isWaiting) || isSolo; }
  } GeneratorState;

  typedef struct GeneratorParams {
    GeneratorState state;
    int   position; // Position number to play
    float pitchAdjust;
    float posAdjust;
    float shape;
    float rate;
    float duration;
    float gain;
    float attack;
    float decay;
    float sustain;
    float release;
    std::vector<float> grainEnv;
    GeneratorParams() {}
    GeneratorParams(GeneratorState state, int position, float pitchAdjust,
                    float posAdjust, float shape, float rate, float duration,
                    float gain, float attack, float decay, float sustain,
                    float release, std::vector<float> grainEnv)
        : state(state),
          position(position),
          pitchAdjust(pitchAdjust),
          posAdjust(posAdjust),
          shape(shape),
          rate(rate),
          duration(duration),
          gain(gain),
          attack(attack),
          decay(decay),
          sustain(sustain),
          release(release),
          grainEnv(grainEnv) {}
  } GeneratorParams;

  typedef struct GlobalParams {
    float gain;
    float attack;
    float decay;
    float sustain;
    float release;
    GlobalParams() {}
    GlobalParams(float gain, float attack, float decay, float sustain,
                 float release)
        : gain(gain),
          attack(attack),
          decay(decay),
          sustain(sustain),
          release(release) {}
  } GlobalParams;

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
      case 0: // red
        r = 1.0f;
        g = 0.0f;
        b = 0.0f;
        break;
      case 1: // orange
        r = 1.0f;
        g = 0.5f;
        b = 0.0f;
        break;
      case 2: // yellow
        r = 1.0f;
        g = 1.0f;
        b = 0.0f;
        break;
      case 3: // chartreuse
        r = 0.5f;
        g = 1.0f;
        b = 0.0f;
        break;
      case 4: // green
        r = 0.0f;
        g = 1.0f;
        b = 0.0f;
        break;
      case 5: // spring green
        r = 0.0f;
        g = 1.0f;
        b = 0.5f;
        break;
      case 6: // cyan
        r = 0.0f;
        g = 1.0f;
        b = 1.0f;
        break;
      case 7: // dodger blue
        r = 0.0f;
        g = 0.5f;
        b = 1.0f;
        break;
      case 8: // blue
        r = 0.0f;
        g = 0.0f;
        b = 1.0f;
        break;
      case 9: // purple
        r = 0.5f;
        g = 0.0f;
        b = 1.0f;
        break;
      case 10: // violet
        r = 1.0f;
        g = 0.0f;
        b = 1.0f;
        break;
      case 11: // magenta
        r = 1.0f;
        g = 0.0f;
        b = 0.5f;
        break;
      default:
        break;
    }
    return juce::Colour(r * 255.0f, g * 255.0f, b * 255.0f);
  }

  template <class TimeT = std::chrono::milliseconds,
            class ClockT = std::chrono::steady_clock>
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
  static inline float lin2db(float value) {
    return value < 1e-10 ? -100 : 10.0f * std::log10(value);
  }

  // Ripped from essentia utils
  class BPF {
   protected:
    std::vector<float> _xPoints;
    std::vector<float> _yPoints;
    std::vector<float> _slopes;

   public:
    BPF() {}
    BPF(std::vector<float> xPoints, std::vector<float> yPoints) {
      init(xPoints, yPoints);
    }
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
        _slopes[j - 1] =
            (_yPoints[j] - _yPoints[j - 1]) / (_xPoints[j] - _xPoints[j - 1]);
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

  static inline bool isBlackKey(PitchClass pitchClass) {
    return ((1 << (pitchClass)) & 0x054a) != 0;
  }
};  // Utils
