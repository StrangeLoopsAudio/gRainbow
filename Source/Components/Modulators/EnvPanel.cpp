/*
  ==============================================================================

    EnvPanel.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "EnvPanel.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"

EnvPanel::EnvPanel(int modIdx, Parameters& parameters)
: mParameters(parameters),
mModEnv(mParameters.global.modEnvs[modIdx]),
mSliderAttack(mParameters, mModEnv.attack),
mSliderDecay(mParameters, mModEnv.decay),
mSliderSustain(mParameters, mModEnv.sustain),
mSliderRelease(mParameters, mModEnv.release),
mBtnMap(mParameters, mModEnv) {
  
  // Default slider settings
  std::vector<std::reference_wrapper<juce::Slider>> sliders = { mSliderAttack, mSliderDecay, mSliderSustain, mSliderRelease };
  for (auto& slider : sliders) {
    slider.get().setNumDecimalPlacesToDisplay(2);
    slider.get().setPopupDisplayEnabled(true, true, this);
    slider.get().setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, mModEnv.colour);
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

  mModEnv.attack->addListener(this);
  mModEnv.decay->addListener(this);
  mModEnv.sustain->addListener(this);
  mModEnv.release->addListener(this);
  mParamHasChanged.store(true); // Init param values
  
  // Default button settings
  std::vector<std::reference_wrapper<juce::TextButton>> buttons = { mBtnMap };
  for (auto& button : buttons) {
    button.get().setToggleable(true);
    button.get().setColour(juce::TextButton::textColourOffId, mModEnv.colour);
    button.get().setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    button.get().setColour(juce::TextButton::buttonColourId, mModEnv.colour);
    button.get().setColour(juce::TextButton::buttonOnColourId, mModEnv.colour);
    addAndMakeVisible(button.get());
  }
  
  // Default label settings
  std::vector<std::reference_wrapper<juce::Label>> labels = { mLabelAttack, mLabelDecay, mLabelSustain, mLabelRelease };
  for (auto& label : labels) {
    label.get().setColour(juce::Label::ColourIds::textColourId, mModEnv.colour);
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

EnvPanel::~EnvPanel() {
  mModEnv.attack->removeListener(this);
  mModEnv.decay->removeListener(this);
  mModEnv.sustain->removeListener(this);
  mModEnv.release->removeListener(this);
  stopTimer();
}

void EnvPanel::visibilityChanged() {
  if (!isVisible()) {
    mBtnMap.resetMappingStatus();
  }
}

void EnvPanel::parameterValueChanged(int, float) { mParamHasChanged.store(true); }

void EnvPanel::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderAttack.setValue(mModEnv.attack->get(), juce::dontSendNotification);
    mSliderDecay.setValue(mModEnv.decay->get(), juce::dontSendNotification);
    mSliderSustain.setValue(mModEnv.sustain->get(), juce::dontSendNotification);
    mSliderRelease.setValue(mModEnv.release->get(), juce::dontSendNotification);
  }
}

void EnvPanel::paint(juce::Graphics& g) {
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().expanded(0, 20).translated(0, -20).toFloat(), 10);
  
  g.setColour(Utils::BG_COLOUR);
  g.fillRect(mVizRect);
  
  float attack = ParamRanges::ATTACK.convertTo0to1(mSliderAttack.getValue());
  float decay = ParamRanges::DECAY.convertTo0to1(mSliderDecay.getValue());
  float sustain = ParamRanges::SUSTAIN.convertTo0to1(mSliderSustain.getValue());
  float release = ParamRanges::RELEASE.convertTo0to1(mSliderRelease.getValue());
  
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
  g.setColour(mModEnv.colour);
  g.strokePath(adsrPath.createPathWithRoundedCorners(5), juce::PathStrokeType(3));
}

void EnvPanel::resized() {
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
  
  mBtnMap.setBounds(rightPanel.removeFromTop(mVizRect.getHeight() / 2));

}
