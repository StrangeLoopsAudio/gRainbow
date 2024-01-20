#pragma once
#include <juce_graphics/juce_graphics.h>

namespace Utils {

namespace Colour {
static const juce::Colour GLOBAL = juce::Colours::white;
static const juce::Colour BACKGROUND = juce::Colour(0xff212121);
static const juce::Colour PANEL = juce::Colour(0xff353535);
}  // namespace Colour

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
  value = juce::jlimit(0, 11, value);
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
  return juce::Colour::fromFloatRGBA(r, g, b, 1.0f).withSaturation(0.55f);
}

}  // namespace Utils
