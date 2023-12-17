#include "RainbowLookAndFeel.h"
#include "RainbowSlider.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/Utils.h"
#include "Utils/Colour.h"
#include "Utils/PitchClass.h"

juce::Typeface::Ptr RainbowLookAndFeel::getTypefaceForFont(const juce::Font&) {
  return getCustomTypeface();
}

// Tabs
int RainbowLookAndFeel::getTabButtonBestWidth(juce::TabBarButton& btn, int tabDepth) {
  int numTabs = btn.getTabbedButtonBar().getNumTabs();
  // Strange hack to perfectly align tabs with content if only 2 tabs
  bool shouldIncrement = (btn.getIndex() == numTabs - 1) && numTabs == 2;
  return (btn.getTabbedButtonBar().getWidth() / numTabs) + (int)shouldIncrement;
}

juce::Rectangle<int> RainbowLookAndFeel::getTabButtonExtraComponentBounds(const juce::TabBarButton& btn, juce::Rectangle<int>& textArea, juce::Component& extraComp) {
  return textArea.withSize(btn.getHeight(), btn.getHeight()).translated(Utils::PADDING, 0);
}

void RainbowLookAndFeel::drawTabButton(juce::TabBarButton& btn, juce::Graphics& g, bool mouseOver, bool mouseDown) {
  auto r = btn.getLocalBounds();
  auto tabColour = btn.isFrontTab() ? Utils::PANEL_COLOUR : Utils::PANEL_COLOUR.darker(0.15);
  if (mouseOver && !btn.isFrontTab()) tabColour = tabColour.brighter(0.1);
  g.setColour(tabColour);
  g.fillRoundedRectangle(r.withHeight(50).toFloat(), 10);
  
  g.setColour(Utils::GLOBAL_COLOUR);
  drawTabButtonText(btn, g, mouseOver, mouseDown);
}

void RainbowLookAndFeel::drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, int w, int h) {} // Do nothing

void RainbowLookAndFeel::drawTabButtonText(juce::TabBarButton& btn, juce::Graphics& g, bool mouseOver, bool mouseDown) {
  auto textColour = (btn.isFrontTab() || mouseOver) ? Utils::GLOBAL_COLOUR : Utils::GLOBAL_COLOUR.darker();
  g.setColour(textColour);
  int trim = btn.getExtraComponent() ? btn.getExtraComponent()->getRight() : 0;
  g.drawText(btn.getButtonText(), btn.getLocalBounds().withTrimmedLeft(trim), juce::Justification::centred);
}

// Sliders
void RainbowLookAndFeel::drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPosProportional, float,
                                          float, juce::Slider& slider) {
  // Get RainbowSlider version of the slider
  RainbowSlider& rbSlider = dynamic_cast<RainbowSlider&>(slider);

  auto r = slider.getLocalBounds().reduced(2, 2);
  
  const float pos = juce::jmax(0.02f, sliderPosProportional);
  const float startRadians = juce::MathConstants<float>::pi + 0.6f;
  const float endRadians = juce::MathConstants<float>::pi + (2.0f * juce::MathConstants<float>::pi) - 0.6f;
  
  const float posRadians = startRadians + pos * (endRadians - startRadians);
  juce::Colour rainbowCol = slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);


  const int size = juce::jmin(r.getWidth(), r.getHeight());
  auto center = r.getCentre().toFloat();
  
  juce::Path path;
  
  // position arc
  g.setColour(rainbowCol);
  path.addArc(r.getX() + ((r.getWidth() - size) / 2), r.getY() + ((r.getHeight() - size) / 2), size, size, startRadians, posRadians, true);
  path.lineTo(center);
  g.strokePath(path.createPathWithRoundedCorners(3), juce::PathStrokeType(3, juce::PathStrokeType::JointStyle::curved));
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

  juce::Colour btnColour = btn.getToggleState() ? fillColour : Utils::BG_COLOUR;
  if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
    btnColour = btnColour.brighter(0.15f);
  }
  g.setColour(btnColour);
  g.fillRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT);
}

void RainbowLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour&,
                                              bool shouldDrawButtonAsHighlighted, bool) {
  juce::Colour btnColour = btn.getToggleState() ? btn.findColour(juce::TextButton::ColourIds::buttonOnColourId)
  : btn.findColour(juce::TextButton::ColourIds::buttonColourId);
  //  if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
  //    btnColour = btnColour.darker();
  //  }
  g.setColour(btnColour);
  if (btn.getToggleState()) {
    g.fillRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT);
  } else {
    g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT, 2.0f);
  }
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
    g.setColour(foreground.contrasting(0.5f));
    g.setFont((float)height * 0.4f);

    g.drawText(textToShow, 0, 0, width, height, juce::Justification::centred, false);
  }
}
