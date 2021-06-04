/*
  ==============================================================================

    RainbowKeyboard.cpp
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#include "RainbowKeyboard.h"

#include <JuceHeader.h>

#include "Utils.h"

//==============================================================================
RainbowKeyboard::RainbowKeyboard(juce::MidiKeyboardState& state)
    : mState(state) {}

RainbowKeyboard::~RainbowKeyboard() {}

void RainbowKeyboard::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw each white key but rainbowy
  for (int key : {0, 2, 4, 5, 7, 9, 11}) {
    drawKey(g, key);
  }
  // Draw each black key but rainbowy
  for (int key : {1, 3, 6, 8, 10}) {
    drawKey(g, key);
  }
}

juce::Rectangle<float> RainbowKeyboard::getKeyRectangle(int pitchClass) {
  auto keyWidth = getWidth() / 7;
  auto blackKeyOffset = BLACK_NOTE_SIZE_RATIO * keyWidth / 2;
  static const float notePos[] = {0.0f,
                                  keyWidth - blackKeyOffset,
                                  keyWidth,
                                  2 * keyWidth - blackKeyOffset,
                                  2 * keyWidth,                              
                                  3 * keyWidth,
                                  4 * keyWidth - blackKeyOffset,
                                  4 * keyWidth,
                                  5 * keyWidth - blackKeyOffset,
                                  5 * keyWidth,                                  
                                  6 * keyWidth - blackKeyOffset,
                                  6 * keyWidth
                                  };
  
  auto isBlackKey = ((1 << (pitchClass)) & 0x054a) != 0;
  auto width = isBlackKey ? BLACK_NOTE_SIZE_RATIO * keyWidth : keyWidth;
  auto height = isBlackKey ? getHeight() / 2 : getHeight();

  return juce::Rectangle<float>(notePos[pitchClass], 0, width,
                                height);
}

void RainbowKeyboard::drawKey(juce::Graphics& g,
                              int pitchClass) {
  bool isDown = mState.isNoteOnForChannels(MIDI_CHANNEL_MASK, pitchClass);
  bool isOver = pitchClass == mMouseOverNote;
  float normPc = (pitchClass + 0.5) / NUM_KEYS;
  auto keyColor = Utils::getRainbow12Colour(normPc);

  juce::Rectangle<float> area = getKeyRectangle(pitchClass);
  g.setColour(keyColor);
  g.fillRect(area);
  g.setColour(juce::Colours::black);
  auto isBlackKey = ((1 << (pitchClass)) & 0x054a) != 0;
  g.drawRect(area, isBlackKey ? 2 : 1);
}

void RainbowKeyboard::resized() { 
  //repaint(); 
}
