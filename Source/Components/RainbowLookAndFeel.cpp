#include "RainbowLookAndFeel.h"
#include "Sliders.h"
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
  g.drawFittedText(btn.getButtonText(), btn.getLocalBounds().withTrimmedLeft(trim), juce::Justification::centred, 2, 1.0f);
}

// Sliders
void RainbowLookAndFeel::drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPosProportional, float,
                                          float, juce::Slider& slider) {
  
  juce::Colour rainbowCol = slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);
  
  auto r = slider.getLocalBounds().reduced(2, 2);
  
  const float startRadians = juce::MathConstants<float>::pi + 0.6f;
  const float endRadians = juce::MathConstants<float>::pi + (2.0f * juce::MathConstants<float>::pi) - 0.6f;
  
  const float posRadians = startRadians + sliderPosProportional * (endRadians - startRadians);

  const int size = juce::jmin(r.getWidth(), r.getHeight());
  auto center = r.getCentre().toFloat().translated(-0.5f, 0);
  
  // Get CommonSlider version of the slider
  CommonSlider* commonSlider = dynamic_cast<CommonSlider*>(&slider);
  if (commonSlider) {
    auto* param = commonSlider->getParam();
    if (param->type == ParamType::GENERATOR && commonSlider->getIsUsed()) {
      // Draw border around slider representing generator being used
      g.setColour(rainbowCol);
      g.drawRoundedRectangle(slider.getLocalBounds().reduced(1, 0).toFloat(), 5, 1);
    }
//    // Draw global tick
//    if (commonSlider->getGlobalValue() != sliderPosProportional) {
//      const float globRadians = startRadians + commonSlider->getGlobalValue() * (endRadians - startRadians);
//      g.setColour(Utils::GLOBAL_COLOUR);
//      g.drawLine(juce::Line<float>(center, center.getPointOnCircumference(size / 2.0f - 3, globRadians)), 2);
//    }
//    if (commonSlider->getParamLevel() == ParamType::GENERATOR && commonSlider->getNoteValue() != sliderPosProportional) {
//      // Draw note tick
//      const float globRadians = startRadians + commonSlider->getNoteValue() * (endRadians - startRadians);
//      g.setColour(rainbowCol);
//      g.drawLine(juce::Line<float>(center, center.getPointOnCircumference(size / 4.0f, globRadians)), 2);
//    }
  }
  
  juce::Path path;
  
  // position arc
  g.setColour(rainbowCol);
  path.addArc(r.getX() + ((r.getWidth() - size) / 2), r.getY() + ((r.getHeight() - size) / 2), size, size, startRadians, posRadians, true);
  path.lineTo(center);
  g.strokePath(path.createPathWithRoundedCorners(3), juce::PathStrokeType(3, juce::PathStrokeType::JointStyle::curved));
}

void RainbowLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
  juce::Colour fillColour = btn.findColour(juce::ToggleButton::ColourIds::tickColourId);
  if (!btn.isEnabled()) fillColour = Utils::BG_COLOUR;
  g.setColour(fillColour);
  g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(1), btn.getHeight() / 2.0f, 2.0f);

  float tickHeight = btn.getHeight() - Utils::PADDING * 2;
  int tickX = btn.getToggleState() ? btn.getWidth() - tickHeight - Utils::PADDING: Utils::PADDING;
  auto tickArea = juce::Rectangle<float>(tickHeight, tickHeight);
  if (shouldDrawButtonAsHighlighted && !btn.getToggleState()) {
    fillColour = fillColour.brighter(0.15f);
  }
  g.setColour(btn.getToggleState() ? fillColour : fillColour.darker());
  g.fillEllipse(tickArea.translated(tickX, Utils::PADDING));
}

void RainbowLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour&,
                                              bool shouldDrawButtonAsHighlighted, bool) {
  juce::Colour btnColour = btn.getToggleState() ? btn.findColour(juce::TextButton::ColourIds::buttonOnColourId)
  : btn.findColour(juce::TextButton::ColourIds::buttonColourId);
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
