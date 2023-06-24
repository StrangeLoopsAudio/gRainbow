/*
  ==============================================================================

    RainbowLookAndFeel.h
    Created: 10 Apr 2021 3:07:42pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class RainbowLookAndFeel : public juce::LookAndFeel_V4 {
 public:
  RainbowLookAndFeel() {}

 private:
  void drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPosProportional, float, float,
                        juce::Slider& slider) override;
  void drawLinearSlider(juce::Graphics& g, int, int, int width, int height, float sliderPos, float, float,
                        const juce::Slider::SliderStyle, juce::Slider& slider) override;
  void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted, bool) override;
  void drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour&, bool shouldDrawButtonAsHighlighted,
                            bool) override;
  void drawComboBox(juce::Graphics& g, int, int, bool, int, int, int, int, juce::ComboBox& box) override;
  void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool, bool, bool isHighlighted, bool, bool,
                         const juce::String& text, const juce::String&, const juce::Drawable*, const juce::Colour*) override;
  void positionComboBoxText(juce::ComboBox& box, juce::Label& labelToPosition) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowLookAndFeel)
};
