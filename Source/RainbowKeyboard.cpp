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

  // Draw "white" keys but rainbowy
  for (int i = 0; i < NUM_KEYS; ++i) {
    float normPc = (i + 0.5) / NUM_KEYS;
    auto keyColor = Utils::getRainbow12Colour(normPc);
    // TODO: get rectangle
    drawKey(g, i, getKeyRectangle(i));
  }
}

juce::Rectangle<float> RainbowKeyboard::getKeyRectangle(int pitchClass) {
  auto isBlackKey = std::find(BLACK_INDICES.begin(), BLACK_INDICES.end(),
                              pitchClass) != BLACK_INDICES.end();

}

void RainbowKeyboard::drawKey(juce::Graphics& g,
                              int pitchClass, juce::Rectangle<float> area) {
  bool isDown = mState.isNoteOnForChannels(MIDI_CHANNEL_MASK, pitchClass);
  bool isOver = pitchClass == mMouseOverNote;

}

void RainbowKeyboard::resized() {}
