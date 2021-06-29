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
  static inline juce::Colour getRainbowColour(float value) {
    int rStripe = (int)std::floor(value * 7);
    float r, g, b;
    switch (rStripe) {
      case 0:
        r = 1;
        g = 0;
        b = 0;
        break;
      case 1:
        r = 1;
        g = 0.49;
        b = 0;
        break;
      case 2:
        r = 1;
        g = 1;
        b = 0;
        break;
      case 3:
        r = 0;
        g = 1;
        b = 0;
        break;
      case 4:
        r = 0;
        g = 0;
        b = 1;
        break;
      case 5:
        r = 0.29;
        g = 0;
        b = 0.5;
        break;
      case 6:
        r = 0.58;
        g = 0;
        b = 0.83;
        break;
      default: {
        r = 0;
        g = 0;
        b = 0;
        break;
      }
    }
    return juce::Colour(r * 255.0f, g * 255.0f, b * 255.0f);
  }

  static inline juce::Colour getRainbow12Colour(float value) {
    int rStripe = (int)std::floor(value * 12);
    float r, g, b;
    switch (rStripe) {
      case 0: // red
        r = 1;
        g = 0;
        b = 0;
        break;
      case 1: // orange
        r = 1;
        g = 0.5;
        b = 0;
        break;
      case 2: // yellow
        r = 1;
        g = 1;
        b = 0;
        break;
      case 3: // chartreuse
        r = 0.5;
        g = 1;
        b = 0;
        break;
      case 4: // green
        r = 0;
        g = 1;
        b = 0;
        break;
      case 5: // spring green
        r = 0;
        g = 1;
        b = 0.5;
        break;
      case 6: // cyan
        r = 0;
        g = 1;
        b = 1;
        break;
      case 7: // dodger blue
        r = 0;
        g = 0.5;
        b = 1;
        break;
      case 8: // blue
        r = 0;
        g = 0;
        b = 1;
        break;
      case 9: // purple
        r = 0.5;
        g = 0;
        b = 1;
        break;
      case 10: // violet
        r = 1;
        g = 0;
        b = 1;
        break;
      case 11: // magenta
        r = 1;
        g = 0;
        b = 0.5;
        break;
      default: {
        r = 0;
        g = 0;
        b = 0;
        break;
      }
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
