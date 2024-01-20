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
int RainbowLookAndFeel::getTabButtonBestWidth(juce::TabBarButton& btn, int) {
  int numTabs = btn.getTabbedButtonBar().getNumTabs();
  // Strange hack to perfectly align tabs with content if only 2 tabs
  bool shouldIncrement = (btn.getIndex() == numTabs - 1) && numTabs == 2;
  return (btn.getTabbedButtonBar().getWidth() / numTabs) + (int)shouldIncrement;
}

juce::Rectangle<int> RainbowLookAndFeel::getTabButtonExtraComponentBounds(const juce::TabBarButton& btn,
                                                                          juce::Rectangle<int>& textArea, juce::Component&) {
  return textArea.withSize(btn.getHeight(), btn.getHeight()).translated(Utils::PADDING, 0);
}

void RainbowLookAndFeel::drawTabButton(juce::TabBarButton& btn, juce::Graphics& g, bool mouseOver, bool mouseDown) {
  auto r = btn.getLocalBounds();
  auto tabColour = btn.isFrontTab() ? Utils::Colour::PANEL : Utils::Colour::PANEL.darker(0.15);
  if (mouseOver && !btn.isFrontTab()) tabColour = tabColour.brighter(0.1);
  g.setColour(tabColour);
  g.fillRoundedRectangle(r.withHeight(50).toFloat(), 10);

  drawTabButtonText(btn, g, mouseOver, mouseDown);
}

void RainbowLookAndFeel::drawTabAreaBehindFrontButton(juce::TabbedButtonBar&, juce::Graphics&, int, int) {}  // Do nothing

void RainbowLookAndFeel::drawTabButtonText(juce::TabBarButton& btn, juce::Graphics& g, bool mouseOver, bool) {
  auto textColour = Utils::Colour::GLOBAL;
  if (btn.isColourSpecified(juce::TextButton::ColourIds::textColourOnId)) {
    textColour = btn.findColour(juce::TextButton::ColourIds::textColourOnId);
  }
  g.setColour((btn.isFrontTab() || mouseOver) ? textColour : textColour.darker());
  int trim = btn.getExtraComponent() ? btn.getExtraComponent()->getRight() : 0;
  g.setFont(Utils::getFont());
  g.drawText(btn.getButtonText(), btn.getLocalBounds().withTrimmedLeft(trim), juce::Justification::centred);
}

// Sliders
void RainbowLookAndFeel::drawRotarySlider(juce::Graphics& g, int, int, int, int, float sliderPosProportional, float, float,
                                          juce::Slider& slider) {
  juce::Colour rainbowCol = slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);
  if (!slider.isEnabled()) rainbowCol = rainbowCol.darker();

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
  }

  ParamSlider* paramSlider = dynamic_cast<ParamSlider*>(&slider);
  if (paramSlider && paramSlider->getParameter()) {
    // Check for modulations on this param and visualize them
    const int idx = paramSlider->getParameter()->getParameterIndex();
    if (paramSlider->parameters.modulations.contains(idx)) {
      Modulation& mod = paramSlider->parameters.modulations.getReference(idx);
      // Draw inner arc representing modulation range
      juce::Range<float> modRange = mod.source->getRange();
      float lowerVal = juce::jlimit(0.0f, 1.0f, sliderPosProportional + (mod.depth * modRange.getStart()));
      float upperVal = juce::jlimit(0.0f, 1.0f, sliderPosProportional + (mod.depth * modRange.getEnd()));
      const float lowerPosRadians = startRadians + lowerVal * (endRadians - startRadians);
      const float upperPosRadians = startRadians + upperVal * (endRadians - startRadians);
      g.setColour(mod.source->colour);
      juce::Path modArc;
      auto modRect = r.reduced(Utils::PADDING);
      const int modRectSize = juce::jmin(modRect.getWidth(), modRect.getHeight());
      modArc.addArc(modRect.getX() + ((modRect.getWidth() - modRectSize) / 2), modRect.getY() + ((modRect.getHeight() - modRectSize) / 2), modRectSize, modRectSize, lowerPosRadians, upperPosRadians, true);
      g.strokePath(modArc, juce::PathStrokeType(3, juce::PathStrokeType::JointStyle::curved));
    }
  }

  juce::Path path;

  // position arc
  g.setColour(rainbowCol);
  path.addArc(r.getX() + ((r.getWidth() - size) / 2), r.getY() + ((r.getHeight() - size) / 2), size, size, startRadians, posRadians, true);
  path.lineTo(center);
  g.strokePath(path.createPathWithRoundedCorners(3), juce::PathStrokeType(3, juce::PathStrokeType::JointStyle::curved));
}

// Buttons
juce::Font RainbowLookAndFeel::getTextButtonFont(juce::TextButton&, int) {
  return Utils::getFont();
}

void RainbowLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted, bool) {
  const bool enabled = btn.isEnabled();  // not being used
  const bool on = btn.getToggleState();  // on/off

  juce::Colour fillColour = enabled ? btn.findColour(juce::ToggleButton::ColourIds::tickColourId) : Utils::Colour::BACKGROUND;
  g.setColour(fillColour);

  auto bounds = btn.getLocalBounds().toFloat();
  const int height = btn.getHeight();
  if (on) {
    g.setColour(fillColour.withAlpha(0.1f));
    g.fillRoundedRectangle(bounds.reduced(1), height / 2.0f);
  }
  g.setColour(fillColour);
  g.drawRoundedRectangle(bounds.reduced(1), height / 2.0f, 2.0f);

  float tickHeight = height - Utils::PADDING * 2;
  int tickX = on ? btn.getWidth() - tickHeight - Utils::PADDING : Utils::PADDING;
  auto tickArea = juce::Rectangle<float>(tickHeight, tickHeight);
  if (!on) {
    fillColour = shouldDrawButtonAsHighlighted ? fillColour.brighter(0.15f) : fillColour.darker();
  }
  g.setColour(fillColour);
  g.fillEllipse(tickArea.translated(tickX, Utils::PADDING));
}

void RainbowLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour&, bool, bool) {
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
  g.drawRoundedRectangle(box.getLocalBounds().toFloat().reduced(1), Utils::ROUNDED_AMOUNT, 2.0f);
}

void RainbowLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& labelToPosition) {
  labelToPosition.setFont(Utils::getFont());
  labelToPosition.setJustificationType(juce::Justification::centred);
  labelToPosition.setBounds(box.getLocalBounds());
}


void RainbowLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool, bool, bool isHighlighted,
                                           bool, bool, const juce::String& text, const juce::String&, const juce::Drawable*,
                                           const juce::Colour*) {
  // Fill background
  juce::Colour backgroundColour = findColour(juce::PopupMenu::ColourIds::backgroundColourId);
  if (isHighlighted) {
    g.setColour(backgroundColour.darker());
    g.fillRect(area);
  }
  g.setColour(backgroundColour.contrasting(0.8f));
  g.setFont(Utils::getFont());
  g.drawText(text, area, juce::Justification::centred);
}

//void RainbowLookAndFeel::drawProgressBar(juce::Graphics& g, juce::ProgressBar& progressBar, int width, int height, double progress,
//                                         const juce::String& textToShow) {
//  juce::Colour background = juce::Colours::white.withAlpha(0.5f);
//  juce::Colour foreground = Utils::getRainbow12Colour(static_cast<int>(progress * 11.0));
//  auto barBounds = progressBar.getLocalBounds().toFloat();
//  const float halfRectHeight = static_cast<float>(progressBar.getHeight()) * 0.5f;
//  g.setColour(background);
//  g.fillRoundedRectangle(barBounds, halfRectHeight);
//
//  if (progress >= 0.0f && progress <= 1.0f) {
//    juce::Path p;
//    p.addRoundedRectangle(barBounds, halfRectHeight);
//    g.reduceClipRegion(p);
//
//    barBounds.setWidth(barBounds.getWidth() * (float)progress);
//    g.setColour(foreground);
//    g.fillRoundedRectangle(barBounds, halfRectHeight);
//  }
//
//  if (textToShow.isNotEmpty()) {
//    g.setColour(foreground.contrasting(0.5f));
//    g.setFont((float)height * 0.4f);
//
//    g.drawText(textToShow, progressBar.getBounds().reduced(10), juce::Justification::centred, false);
//  }
//}
