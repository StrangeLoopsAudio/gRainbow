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
  enum PositionColour { BLUE = 0, PURPLE, ORANGE, GREEN, NUM_BOXES };
  enum EnvelopeState { ATTACK, DECAY, SUSTAIN, RELEASE };
  static constexpr juce::int64 POSITION_COLOURS[4] = {0xFF52C4FF, 0xFFE352FF,
                                                      0xFFFF8D52, 0xFF6EFF52};
  static constexpr juce::int64 SECONDARY_POSITION_COLOURS[4][4] = {
      {0xFF526EFF, 0xFF8D52FF, 0xFFE352FF, 0xFFFF52C4},
      {0xFFFF52C5, 0xFFFF526E, 0xFFFF8C52, 0xFFFFE352},
      {0xFFFFE352, 0xFFC4FF52, 0xFF6DFF52, 0xFF52FF8D},
      {0xFF52FF8C, 0xFF52FFE3, 0xFF52C5FF, 0xFF526EFF}};

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
};
