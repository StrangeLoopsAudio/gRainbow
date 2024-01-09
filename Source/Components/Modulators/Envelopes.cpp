/*
  ==============================================================================

    Envelopes.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "Envelopes.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

Envelopes::Envelopes(Parameters& parameters)
    : mParameters(parameters),
mSliderAttack(mParameters, mParameters.global.env1.attack),
mSliderDecay(mParameters, mParameters.global.env1.decay),
mSliderSustain(mParameters, mParameters.global.env1.sustain),
mSliderRelease(mParameters, mParameters.global.env1.release) {
  
  // Default slider settings
  std::vector<std::reference_wrapper<juce::Slider>> sliders = { mSliderAttack, mSliderDecay, mSliderSustain, mSliderRelease };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(slider.get());
  }
  mSliderAttack.setRange(ParamRanges::ATTACK.start, ParamRanges::ATTACK.end, 0.01);
  mSliderAttack.setDoubleClickReturnValue(true, ParamDefaults::ATTACK_DEFAULT_SEC);
  mSliderAttack.setTextValueSuffix("s");
  mSliderDecay.setRange(ParamRanges::DECAY.start, ParamRanges::DECAY.end, 0.01);
  mSliderDecay.setDoubleClickReturnValue(true, ParamDefaults::DECAY_DEFAULT_SEC);
  mSliderDecay.setTextValueSuffix("s");
  mSliderSustain.setRange(ParamRanges::SUSTAIN.start, ParamRanges::SUSTAIN.end, 0.01);
  mSliderSustain.setDoubleClickReturnValue(true, ParamDefaults::SUSTAIN_DEFAULT);
  mSliderRelease.setRange(ParamRanges::RELEASE.start, ParamRanges::RELEASE.end, 0.01);
  mSliderRelease.setDoubleClickReturnValue(true, ParamDefaults::RELEASE_DEFAULT_SEC);
  mSliderRelease.setTextValueSuffix("s");

  mParameters.global.env1.attack->addListener(this);
  mParameters.global.env1.decay->addListener(this);
  mParameters.global.env1.sustain->addListener(this);
  mParameters.global.env1.release->addListener(this);
  mParamHasChanged.store(true); // Init param values
  
  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelAttack, mLabelDecay, mLabelSustain, mLabelRelease };
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
    label.get().setJustificationType(juce::Justification::centredTop);
    label.get().setFont(juce::Font(14));
    addAndMakeVisible(label.get());
  }
  mLabelAttack.setText("attack", juce::dontSendNotification);
  mLabelDecay.setText("decay", juce::dontSendNotification);
  mLabelSustain.setText("sustain", juce::dontSendNotification);
  mLabelRelease.setText("release", juce::dontSendNotification);

  startTimer(Utils::UI_REFRESH_INTERVAL);
}

Envelopes::~Envelopes() {
  mParameters.global.env1.attack->removeListener(this);
  mParameters.global.env1.decay->removeListener(this);
  mParameters.global.env1.sustain->removeListener(this);
  mParameters.global.env1.release->removeListener(this);
  stopTimer();
}

void Envelopes::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void Envelopes::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderAttack.setValue(mParameters.global.env1.attack->get(), juce::dontSendNotification);
    mSliderDecay.setValue(mParameters.global.env1.decay->get(), juce::dontSendNotification);
    mSliderSustain.setValue(mParameters.global.env1.sustain->get(), juce::dontSendNotification);
    mSliderRelease.setValue(mParameters.global.env1.release->get(), juce::dontSendNotification);
  }
}

void Envelopes::paint(juce::Graphics& g) {
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
  
  g.setColour(Utils::BG_COLOUR);
  g.fillRect(mVizRect);
  
  float attack = ParamRanges::ATTACK.convertTo0to1(mSliderAttack.getValue());
  float decay = ParamRanges::DECAY.convertTo0to1(mSliderDecay.getValue());
  float sustain = ParamRanges::SUSTAIN.convertTo0to1(mSliderSustain.getValue());
  float release = ParamRanges::RELEASE.convertTo0to1(mSliderRelease.getValue());
  
  // Draw ADSR path
  juce::Colour colour = Utils::GLOBAL_COLOUR;
  g.setFillType(juce::ColourGradient(colour.withAlpha(0.35f), mVizRect.getTopLeft(), colour.withAlpha(0.05f), mVizRect.getBottomLeft(), false));
  
  auto adsrRect = mVizRect.reduced(Utils::PADDING * 2, Utils::PADDING * 2);
  
  juce::Path adsrPath;
  juce::Point<float> startPt = adsrRect.getBottomLeft();
  juce::Point<float> attackPt = startPt.translated(attack * adsrRect.getWidth() * 0.375f, -adsrRect.getHeight());
  juce::Point<float> decayPt = attackPt.translated(decay * adsrRect.getWidth() * 0.375f, (1.0f - sustain) * (adsrRect.getHeight()));
  juce::Point<float> sustainPt = adsrRect.getBottomLeft().translated(adsrRect.getWidth() * 0.75f, -sustain * (adsrRect.getHeight()));
  juce::Point<float> endPt = sustainPt.translated(release * adsrRect.getWidth() * 0.25f, 0.0f).withY(adsrRect.getBottom());
  
  adsrPath.startNewSubPath(startPt);
  adsrPath.lineTo(attackPt);
  adsrPath.lineTo(decayPt);
  adsrPath.lineTo(sustainPt);
  adsrPath.lineTo(endPt);
  adsrPath.closeSubPath();
  g.fillPath(adsrPath.createPathWithRoundedCorners(5));
  g.setColour(colour);
  g.strokePath(adsrPath.createPathWithRoundedCorners(5), juce::PathStrokeType(3));
}

void Envelopes::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING);

  // Labels
  auto labelPanel = r.removeFromBottom(Utils::LABEL_HEIGHT);
  const int labelWidth = labelPanel.getWidth() / 4;
  mLabelAttack.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelDecay.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelSustain.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelRelease.setBounds(labelPanel.removeFromLeft(labelWidth));
  
  // Sliders
  auto knobPanel = r.removeFromBottom(Utils::KNOB_HEIGHT);
  const int knobWidth = knobPanel.getWidth() / 4;
  mSliderAttack.setBounds(knobPanel.removeFromLeft(knobWidth).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mSliderDecay.setBounds(knobPanel.removeFromLeft(knobWidth).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mSliderSustain.setBounds(knobPanel.removeFromLeft(knobWidth).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  mSliderRelease.setBounds(knobPanel.removeFromLeft(knobWidth).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));
  
  auto rightPanel = r.removeFromRight(r.getWidth() * 0.25f);
  
  r.removeFromRight(Utils::PADDING);
  r.removeFromBottom(Utils::PADDING);
  
  mVizRect = r.toFloat();
}
