/*
  ==============================================================================

    FilterControl.cpp
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#include "FilterControl.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

//==============================================================================
FilterControl::FilterControl(Parameters& parameters)
    : mParameters(parameters),
      mSliderCutoff(parameters, mParameters.global.filterCutoff),
      mSliderResonance(parameters, mParameters.global.filterRes),
      mPathStroke(3, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::rounded) {
  juce::Colour colour = Utils::Colour::GLOBAL;

  // Default slider settings
  std::vector<std::reference_wrapper<juce::Slider>> sliders = { mSliderCutoff, mSliderResonance };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    slider.get().setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::Colour::GLOBAL);
    addAndMakeVisible(slider.get());
  }
  mSliderCutoff.setRange(ParamRanges::FILT_CUTOFF.start, ParamRanges::FILT_CUTOFF.end, 0.01);
  mSliderCutoff.setTextValueSuffix("hz");
  mSliderCutoff.setDoubleClickReturnValue(true, ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
  mSliderResonance.setRange(ParamRanges::FILT_RESONANCE.start, ParamRanges::FILT_RESONANCE.end, 0.01);
  mSliderResonance.setDoubleClickReturnValue(true, ParamDefaults::FILTER_RESONANCE_DEFAULT);

  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelCutoff, mLabelResonance };
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, Utils::Colour::GLOBAL);
    label.get().setJustificationType(juce::Justification::centredTop);
    label.get().setFont(juce::Font(14));
    addAndMakeVisible(label.get());
  }
  mLabelCutoff.setText("cutoff", juce::dontSendNotification);
  mLabelResonance.setText("resonance", juce::dontSendNotification);

  mFilterType.setJustificationType(juce::Justification::centred);
  mFilterType.setColour(juce::ComboBox::ColourIds::backgroundColourId, colour);
  for (int i = 0; i < FILTER_TYPE_NAMES.size(); ++i) {
    mFilterType.addItem(FILTER_TYPE_NAMES[i], i + 1);
  }
  mFilterType.onChange = [this]() {
    int type = mFilterType.getSelectedId() - 1;
    ParamHelper::setParam(mParameters.global.filterType, type);
  };
  addAndMakeVisible(mFilterType);

  mParameters.global.filterType->addListener(this);
  mParameters.global.filterCutoff->addListener(this);
  mParameters.global.filterRes->addListener(this);

  mParamHasChanged.store(true); // Init param values

  startTimer(Utils::UI_REFRESH_INTERVAL);
}

FilterControl::~FilterControl() {
  mParameters.global.filterType->removeListener(this);
  mParameters.global.filterCutoff->removeListener(this);
  mParameters.global.filterRes->removeListener(this);
  stopTimer();
}

void FilterControl::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void FilterControl::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderCutoff.setValue(mParameters.global.filterCutoff->get(), juce::dontSendNotification);
    mSliderResonance.setValue(mParameters.global.filterRes->get(),
                              juce::dontSendNotification);
    mFilterType.setSelectedId(mParameters.global.filterType->getIndex() + 1,
                              juce::dontSendNotification);
    updateFilterPath();
    repaint();
  }
}

void FilterControl::paint(juce::Graphics& g) {
  juce::Colour colour = Utils::Colour::GLOBAL;

  g.setColour(Utils::Colour::PANEL);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);

  g.setColour(Utils::Colour::BACKGROUND);
  g.fillRect(mVizRect);

  // Set gradient
  g.setFillType(juce::ColourGradient(colour.withAlpha(0.35f), mVizRect.getTopLeft(), colour.withAlpha(0.05f), mVizRect.getBottomLeft(), false));
  g.fillPath(mFilterPath);

  // Stroke highlight on path
  g.setColour(colour);
  g.strokePath(mFilterPath, mPathStroke);

}

void FilterControl::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING);

  int panelWidth = r.getWidth() / 3.0f;
  juce::Rectangle<int> knobPanel = r.removeFromBottom(Utils::LABEL_HEIGHT + Utils::KNOB_HEIGHT);
  juce::Rectangle<int> panel = knobPanel.removeFromRight(panelWidth);
  mLabelResonance.setBounds(panel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderResonance.setBounds(
                             panel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  panel = knobPanel.removeFromRight(panelWidth);
  mLabelCutoff.setBounds(panel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderCutoff.setBounds(panel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));

  mFilterType.setBounds(knobPanel);

  r.removeFromBottom(Utils::PADDING);

  mVizRect = r.toFloat();
}

void FilterControl::updateFilterPath() {
  auto drawRect = mVizRect.reduced(Utils::PADDING * 2, Utils::PADDING * 2);

  // Draw selected filter path
  mFilterPath.clear();
  int resPadding = drawRect.getHeight() * 0.3f; // both width and height max size of resonance peak
  float cutoffWidth = drawRect.getWidth() *
  ParamRanges::FILT_CUTOFF.convertTo0to1(mSliderCutoff.getValue());
  float resHeight =
  drawRect.getHeight() * 0.3f * ParamRanges::FILT_RESONANCE.convertTo0to1(mSliderResonance.getValue());

  juce::Point<float> startPt = drawRect.getBottomLeft();
  juce::Point<float> midPt1, midPt2, midPt3, midPt4;
  juce::Point<float> endPt = drawRect.getBottomRight();

  switch (mFilterType.getSelectedId() - 1) {
    case (Utils::FilterType::LOWPASS):
      midPt1 = juce::Point<float>(drawRect.getX(), drawRect.getY() + resPadding);
      midPt2 = juce::Point<float>(drawRect.getX() + cutoffWidth - resPadding / 2, drawRect.getY() + resPadding);
      midPt3 = juce::Point<float>(drawRect.getX() + cutoffWidth, drawRect.getY() + (resPadding - resHeight));
      midPt4 = juce::Point<float>(drawRect.getX() + cutoffWidth + resPadding / 2, drawRect.getBottom());
      break;
    case (Utils::FilterType::HIGHPASS):
      cutoffWidth = drawRect.getWidth() - cutoffWidth; // Reverse for HP
      midPt1 = juce::Point<float>(drawRect.getRight() - cutoffWidth - resPadding / 2, drawRect.getBottom());
      midPt2 = juce::Point<float>(drawRect.getRight() - cutoffWidth, drawRect.getY() + (resPadding - resHeight));
      midPt3 = juce::Point<float>(drawRect.getRight() - cutoffWidth + resPadding / 2, drawRect.getY() + resPadding);
      midPt4 = juce::Point<float>(drawRect.getRight(), drawRect.getY() + resPadding);
      break;
    case (Utils::FilterType::BANDPASS):
      midPt1 = juce::Point<float>(drawRect.getCentreX() - cutoffWidth / 2 - resPadding / 2,
                                  drawRect.getBottom());
      midPt2 = juce::Point<float>(drawRect.getCentreX() - cutoffWidth / 2, drawRect.getY() + (resPadding - resHeight));
      midPt3 = juce::Point<float>(drawRect.getCentreX() + cutoffWidth / 2, drawRect.getY() + (resPadding - resHeight));
      midPt4 = juce::Point<float>(drawRect.getCentreX() + cutoffWidth / 2 + resPadding / 2,
                                  drawRect.getBottom());
      break;
    case (Utils::FilterType::NO_FILTER):
      midPt1 = juce::Point<float>(drawRect.getTopLeft().translated(0, resPadding));
      midPt2 = juce::Point<float>(drawRect.getTopLeft().translated(10, resPadding));
      midPt3 = juce::Point<float>(drawRect.getTopRight().translated(-10, resPadding));
      midPt4 = juce::Point<float>(drawRect.getTopRight().translated(0, resPadding));
      break;
  }

  // I'm not happy with this either, clean it up if possible
  midPt1.setXY(juce::jlimit(drawRect.getX(), drawRect.getRight(), midPt1.x), juce::jlimit(drawRect.getY(), drawRect.getBottom(), midPt1.y));
  midPt2.setXY(juce::jlimit(drawRect.getX(), drawRect.getRight(), midPt2.x), juce::jlimit(drawRect.getY(), drawRect.getBottom(), midPt2.y));
  midPt3.setXY(juce::jlimit(drawRect.getX(), drawRect.getRight(), midPt3.x), juce::jlimit(drawRect.getY(), drawRect.getBottom(), midPt3.y));
  midPt4.setXY(juce::jlimit(drawRect.getX(), drawRect.getRight(), midPt4.x), juce::jlimit(drawRect.getY(), drawRect.getBottom(), midPt4.y));

  // Make path from points
  if (startPt.x < midPt1.x) startPt.x = midPt1.x + 5;
  if (endPt.x > midPt4.x) endPt.x = midPt4.x - 5;
  mFilterPath.startNewSubPath(startPt);
  mFilterPath.lineTo(midPt1);
  mFilterPath.lineTo(midPt2);
  mFilterPath.lineTo(midPt3);
  mFilterPath.lineTo(midPt4);
  mFilterPath.lineTo(endPt);
  mFilterPath.closeSubPath();
  mFilterPath = mFilterPath.createPathWithRoundedCorners(5);
}
