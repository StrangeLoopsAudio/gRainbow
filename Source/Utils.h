/*
  ==============================================================================

    Utils.h
    Created: 10 Apr 2021 3:27:51pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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

  typedef struct SpecRanges {
    std::vector<float> frameMax;
    float globalMax;
  } FftRanges;

  typedef struct HpsPitch {
    float freq;
    float amplitude;
    HpsPitch(float freq, float amplitude) : freq(freq), amplitude(amplitude) {}
  } HpsPitch;
};
