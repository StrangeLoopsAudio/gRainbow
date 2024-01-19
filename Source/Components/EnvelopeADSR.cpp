/*
  ==============================================================================

    EnvelopeADSR.cpp
    Created: 12 Jul 2021 12:02:12am
    Author:  brady

  ==============================================================================
*/

#include "EnvelopeADSR.h"
#include "Utils/Utils.h"

// Just like EnvelopePanel but for the amp env (no map button and global params)
// TODO: can probably consolidate the 2 classes in a clever way
//==============================================================================
EnvelopeADSR::EnvelopeADSR(Parameters& parameters)
    : mParameters(parameters),
      mSliderAttack(mParameters, mParameters.global.ampEnvAttack),
      mSliderDecay(mParameters, mParameters.global.ampEnvDecay),
      mSliderSustain(mParameters, mParameters.global.ampEnvSustain),
      mSliderRelease(mParameters, mParameters.global.ampEnvRelease)
{
  // Default slider settings
  std::vector<std::reference_wrapper<juce::Slider>> sliders = { mSliderAttack, mSliderDecay, mSliderSustain, mSliderRelease };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    slider.get().setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, Utils::GLOBAL_COLOUR);
    addAndMakeVisible(slider.get());
  }
  mSliderAttack.setRange(ParamRanges::ATTACK.start, ParamRanges::ATTACK.end, 0.01);
  mSliderAttack.setDoubleClickReturnValue(true, ParamDefaults::ATTACK_DEFAULT_SEC);
  mSliderAttack.setTextValueSuffix(" s");
  mSliderDecay.setRange(ParamRanges::DECAY.start, ParamRanges::DECAY.end, 0.01);
  mSliderDecay.setDoubleClickReturnValue(true, ParamDefaults::DECAY_DEFAULT_SEC);
  mSliderDecay.setTextValueSuffix(" s");
  mSliderSustain.setRange(ParamRanges::SUSTAIN.start, ParamRanges::SUSTAIN.end, 0.01);
  mSliderSustain.setDoubleClickReturnValue(true, ParamDefaults::SUSTAIN_DEFAULT);
  mSliderRelease.setRange(ParamRanges::RELEASE.start, ParamRanges::RELEASE.end, 0.01);
  mSliderRelease.setDoubleClickReturnValue(true, ParamDefaults::RELEASE_DEFAULT_SEC);
  mSliderRelease.setTextValueSuffix(" s");
  
  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelAttack, mLabelDecay, mLabelSustain, mLabelRelease };
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
    label.get().setJustificationType(juce::Justification::centredTop);
    label.get().setFont(Utils::getFont());
    addAndMakeVisible(label.get());
  }
  mLabelAttack.setText("attack", juce::dontSendNotification);
  mLabelDecay.setText("decay", juce::dontSendNotification);
  mLabelSustain.setText("sustain", juce::dontSendNotification);
  mLabelRelease.setText("release", juce::dontSendNotification);
  
  mParameters.global.ampEnvAttack->addListener(this);
  mParameters.global.ampEnvDecay->addListener(this);
  mParameters.global.ampEnvSustain->addListener(this);
  mParameters.global.ampEnvRelease->addListener(this);
  mParamHasChanged.store(true); // Init param values

  startTimer(Utils::UI_REFRESH_INTERVAL);
}

EnvelopeADSR::~EnvelopeADSR() {
  mParameters.global.ampEnvAttack->removeListener(this);
  mParameters.global.ampEnvDecay->removeListener(this);
  mParameters.global.ampEnvSustain->removeListener(this);
  mParameters.global.ampEnvRelease->removeListener(this);
  stopTimer();
}

void EnvelopeADSR::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void EnvelopeADSR::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderAttack.setValue(mParameters.global.ampEnvAttack->get(), juce::dontSendNotification);
    mSliderDecay.setValue(mParameters.global.ampEnvDecay->get(), juce::dontSendNotification);
    mSliderSustain.setValue(mParameters.global.ampEnvSustain->get(), juce::dontSendNotification);
    mSliderRelease.setValue(mParameters.global.ampEnvRelease->get(), juce::dontSendNotification);
  }
}

void EnvelopeADSR::paint(juce::Graphics& g) {
  juce::Colour colour = Utils::GLOBAL_COLOUR;

  // Panel rectangle
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
  
  // Visualization rect
  g.setColour(Utils::BG_COLOUR);
  g.fillRect(mVizRect);

  const float attack = ParamRanges::ATTACK.convertTo0to1(mSliderAttack.getValue());
  const float decay = ParamRanges::DECAY.convertTo0to1(mSliderDecay.getValue());
  const float sustain = ParamRanges::SUSTAIN.convertTo0to1(mSliderSustain.getValue());
  const float release = ParamRanges::RELEASE.convertTo0to1(mSliderRelease.getValue());

  // Draw ADSR path
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
  g.setColour(colour);
  g.strokePath(adsrPath.createPathWithRoundedCorners(5), juce::PathStrokeType(3));
}

void EnvelopeADSR::resized() {
  auto r = getLocalBounds().reduced(Utils::PADDING);

  // Place labels
  juce::Rectangle<int> labelPanel = r.removeFromBottom(Utils::LABEL_HEIGHT);
  int labelWidth = labelPanel.getWidth() / 4;
  mLabelAttack.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelDecay.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelSustain.setBounds(labelPanel.removeFromLeft(labelWidth));
  mLabelRelease.setBounds(labelPanel.removeFromLeft(labelWidth));

  // Place sliders
  juce::Rectangle<int> knobPanel = r.removeFromBottom(Utils::KNOB_HEIGHT);
  int knobWidth = knobPanel.getWidth() / 4;
  mSliderAttack.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDecay.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderSustain.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRelease.setBounds(knobPanel.removeFromLeft(knobWidth));

  r.removeFromBottom(Utils::PADDING);

  mVizRect = r.toFloat();
}
