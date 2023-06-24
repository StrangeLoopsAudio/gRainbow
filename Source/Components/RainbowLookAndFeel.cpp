#include "RainbowLookAndFeel.h"
#include "RainbowSlider.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/Utils.h"
#include "Utils/Colour.h"
#include "Utils/PitchClass.h"
#include "Parameters.h"

void RainbowLookAndFeel::drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPosProportional, float,
                                          float, juce::Slider& slider) {
  // Get RainbowSlider version of the slider
  RainbowSlider& rbSlider = dynamic_cast<RainbowSlider&>(slider);

  const float pos = juce::jmax(0.02f, sliderPosProportional);
  const float startRadians = 1.5f * juce::MathConstants<float>::pi;
  const float endRadians = startRadians + (pos * juce::MathConstants<float>::pi);
  const float startRadius = width / 4.0f;
  const float endRadius = width / 2.0f - 2;
  const float noteStripeInterval = (endRadius - startRadius) / static_cast<float>(Utils::PitchClass::COUNT);

  const juce::Point<float> center = juce::Point<float>(width / 2.0f, height);

  juce::Colour rainbowCol = slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);

  // Draw main arc
  g.setFillType(juce::ColourGradient(rainbowCol, slider.getLocalBounds().getBottomLeft().toFloat(), rainbowCol.withAlpha(0.4f),
                                     slider.getLocalBounds().getTopLeft().toFloat(), false));
  juce::Path rainbowPath = juce::Path();
  rainbowPath.addCentredArc(center.x, center.y, width / 2.7f, width / 2.7f, 0, startRadians, endRadians, true);
  g.strokePath(rainbowPath, juce::PathStrokeType(width / 4.0f));

  // Draw note-level arcs
  float curStripeRadius = startRadius;
  for (int i = 0; i < Utils::PitchClass::COUNT; ++i) {
    if (auto arc = rbSlider.getArcValue((Utils::PitchClass)i)) {
      float stripeEndRadians = startRadians + ((*arc) * juce::MathConstants<float>::pi);
      g.setColour(Utils::getRainbow12Colour(i));
      rainbowPath.clear();
      rainbowPath.addCentredArc(center.x, center.y, curStripeRadius, curStripeRadius, 0, startRadians, stripeEndRadians, true);
      g.strokePath(rainbowPath, juce::PathStrokeType(noteStripeInterval));
    }
    curStripeRadius += noteStripeInterval;
  }

  // Draw current value line on end of arc
  g.setColour(rainbowCol);
  g.drawLine(juce::Line<float>(center.getPointOnCircumference(startRadius, startRadius, endRadians),
                               center.getPointOnCircumference(endRadius, endRadius, endRadians)),
             3.0f);

  // Draw outline arcs
  g.setColour(rainbowCol);
  rainbowPath.clear();
  rainbowPath.addCentredArc(center.x, center.y, endRadius, endRadius, 0, startRadians, 2.5f * juce::MathConstants<float>::pi, true);
  g.strokePath(rainbowPath, juce::PathStrokeType(3));
}

void RainbowLookAndFeel::drawLinearSlider(juce::Graphics& g, int, int, int width, int height, float sliderPos, float, float,
                                          const juce::Slider::SliderStyle, juce::Slider& slider) {
  // Get RainbowSlider version of the slider
  RainbowSlider& rbSlider = dynamic_cast<RainbowSlider&>(slider);
  juce::Colour rainbowCol = slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);

  // Draw Current value bar
  g.setColour(rainbowCol.withAlpha(0.4f));
  g.fillRect(0, 0, (int)sliderPos, height);

  // Draw note-level lines
  const float noteStripeInterval = (float)(height - 4) / (float)Utils::PitchClass::COUNT;
  float curStripeY = static_cast<float>(height - 2);
  for (int i = 0; i < Utils::PitchClass::COUNT; ++i) {
    if (auto arc = rbSlider.getArcValue((Utils::PitchClass)i)) {
      float stripeX = (*arc) * width;
      g.setColour(Utils::getRainbow12Colour(i));
      g.drawLine(0, curStripeY, stripeX, curStripeY, noteStripeInterval);
    }
    curStripeY -= noteStripeInterval;
  }

  // Vertical line at position
  g.setColour(rainbowCol);
  g.drawLine(sliderPos, 0, sliderPos, (float)height, 2);

  // Outline
  g.setColour(rainbowCol);
  g.drawRect(1, 1, width, height, 2);
}

void RainbowLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted, bool) {
  juce::Colour fillColour = btn.findColour(juce::ToggleButton::ColourIds::tickColourId);
  g.setColour(fillColour);
  g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT, 2.0f);

  juce::Colour btnColour = btn.getToggleState() ? fillColour : juce::Colours::black;
  if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
    btnColour = fillColour.interpolatedWith(juce::Colours::black, 0.8);
  }
  g.setColour(btnColour);
  g.fillRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT);
}

void RainbowLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour&,
                                              bool shouldDrawButtonAsHighlighted, bool) {
  juce::Colour btnColour = btn.getToggleState() ? btn.findColour(juce::TextButton::ColourIds::buttonOnColourId)
                                                : btn.findColour(juce::TextButton::ColourIds::buttonColourId);
  if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
    btnColour = btnColour.darker();
  }
  g.setColour(btnColour);
  g.fillRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT);
  g.setColour(juce::Colours::black);
  g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT, 2.0f);
}

void RainbowLookAndFeel::drawComboBox(juce::Graphics& g, int, int, bool, int, int, int, int, juce::ComboBox& box) {
  // Draw background
  g.setColour(box.findColour(juce::ComboBox::ColourIds::backgroundColourId));
  g.fillRoundedRectangle(box.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT);
  // Draw arrow
}

void RainbowLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool, bool, bool isHighlighted,
                                           bool, bool, const juce::String& text, const juce::String&, const juce::Drawable*,
                                           const juce::Colour*) {
  // Fill background
  if (isHighlighted) {
    g.setColour(findColour(juce::PopupMenu::ColourIds::backgroundColourId).darker());
    g.fillRect(area);
  }
  g.setColour(juce::Colours::white);
  g.drawFittedText(text, area, juce::Justification::centred, 1);
}

void RainbowLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& labelToPosition) {
  labelToPosition.setBounds(box.getLocalBounds());
}

void RainbowLookAndFeel::drawProgressBar(juce::Graphics& g, juce::ProgressBar& progressBar, int width, int height, double progress,
                                         const juce::String& textToShow) {
  juce::Colour background = juce::Colours::white.withAlpha(0.5f);
  juce::Colour foreground = Utils::getRainbow12Colour(static_cast<int>(progress * 11.0));
  auto barBounds = progressBar.getLocalBounds().toFloat();
  const float halfRectHeight = static_cast<float>(progressBar.getHeight()) * 0.5f;
  g.setColour(background);
  g.fillRoundedRectangle(barBounds, halfRectHeight);

  if (progress >= 0.0f && progress <= 1.0f) {
    juce::Path p;
    p.addRoundedRectangle(barBounds, halfRectHeight);
    g.reduceClipRegion(p);

    barBounds.setWidth(barBounds.getWidth() * (float)progress);
    g.setColour(foreground);
    g.fillRoundedRectangle(barBounds, halfRectHeight);
  }

  if (textToShow.isNotEmpty()) {
    g.setColour(juce::Colour::contrasting(background, foreground));
    g.setFont((float)height * 0.4f);

    g.drawText(textToShow, 0, 0, width, height, juce::Justification::centred, false);
  }
}
