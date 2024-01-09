/*
  ==============================================================================

    RainbowLookAndFeel.h
    Created: 10 Apr 2021 3:07:42pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

class RainbowLookAndFeel : public juce::LookAndFeel_V4 {
 public:
  static const juce::Typeface::Ptr getCustomTypeface()
  {
    static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::LeagueSpartanSemiBold_ttf, BinaryData::LeagueSpartanSemiBold_ttfSize);
    return typeface;
  }
  
  RainbowLookAndFeel() {
  }

 private:
  juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;

  // Tabs
  int getTabButtonBestWidth(juce::TabBarButton& btn, int tabDepth) override;
  juce::Rectangle<int> getTabButtonExtraComponentBounds(const juce::TabBarButton& btn, juce::Rectangle<int>& textArea, juce::Component& extraComp) override;
  void drawTabButton(juce::TabBarButton& btn, juce::Graphics& path, bool mouseOver, bool mouseDown) override;
  void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, int w, int h) override;
  void drawTabButtonText(juce::TabBarButton& btn, juce::Graphics&, bool mouseOver, bool mouseDown) override;
  
  // Sliders
  void drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPosProportional, float, float,
                        juce::Slider& slider) override;
  
  // Buttons
  juce::Font getTextButtonFont(juce::TextButton& btn, int buttonHeight) override;
  void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
  void drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour&, bool shouldDrawButtonAsHighlighted,
                            bool) override;
  
  // Combo boxes
  void drawComboBox(juce::Graphics& g, int, int, bool, int, int, int, int, juce::ComboBox& box) override;
  void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool, bool, bool isHighlighted, bool, bool,
                         const juce::String& text, const juce::String&, const juce::Drawable*, const juce::Colour*) override;
  void positionComboBoxText(juce::ComboBox& box, juce::Label& labelToPosition) override;
  
  // Progress bars
  void drawProgressBar(juce::Graphics& g, juce::ProgressBar& progressBar, int width, int height, double progress,
                       const juce::String& textToShow) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RainbowLookAndFeel)
};
