/*
  ==============================================================================

    GrainControl.cpp
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#include "GrainControl.h"
#include "../Utils.h"

GrainControl::GrainControl(Parameters& parameters, foleys::LevelMeterSource& meterSource)
    : mParameters(parameters),
      mCurSelectedParams(parameters.selectedParams),
      mSliderPitchAdjust(parameters, ParamCommon::Type::PITCH_ADJUST),
      mSliderPitchSpray(parameters, ParamCommon::Type::PITCH_SPRAY),
      mSliderPosAdjust(parameters, ParamCommon::Type::POS_ADJUST),
      mSliderPosSpray(parameters, ParamCommon::Type::POS_SPRAY),
      mSliderPanAdjust(parameters, ParamCommon::Type::PAN_ADJUST),
      mSliderPanSpray(parameters, ParamCommon::Type::PAN_SPRAY),
      mSliderGain(parameters, ParamCommon::Type::GAIN) {

  juce::Colour colour = Utils::GLOBAL_COLOUR;

  // Gain and meter
  mMeter.setMeterSource(&meterSource);
  mMeter.setLookAndFeel(&mMeterLookAndFeel);
  addAndMakeVisible(mMeter);

  mSliderGain.setNumDecimalPlacesToDisplay(2);
  mSliderGain.setRange(ParamRanges::GAIN.start, ParamRanges::GAIN.end, 0.01);
  addAndMakeVisible(mSliderGain);

  mLabelGain.setText("Gain", juce::dontSendNotification);
  mLabelGain.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelGain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelGain);

  // Adjust pitch
  mSliderPitchAdjust.setNumDecimalPlacesToDisplay(2);
  mSliderPitchAdjust.setRange(ParamRanges::PITCH_ADJUST.start, ParamRanges::PITCH_ADJUST.end, 0.01);
  addAndMakeVisible(mSliderPitchAdjust);

  mLabelPitchAdjust.setText("Pitch Adjust", juce::dontSendNotification);
  mLabelPitchAdjust.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelPitchAdjust.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPitchAdjust);

  // Pitch spray
  mSliderPitchSpray.setTextValueSuffix("cents");
  mSliderPitchSpray.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
  mSliderPitchSpray.setNumDecimalPlacesToDisplay(3);
  mSliderPitchSpray.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, colour);
  mSliderPitchSpray.setColour(juce::Slider::ColourIds::textBoxTextColourId, colour);
  mSliderPitchSpray.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::white);
  mSliderPitchSpray.setRange(ParamRanges::PITCH_SPRAY.start, ParamRanges::PITCH_SPRAY.end, 0.005);
  addAndMakeVisible(mSliderPitchSpray);

  mLabelPitchSpray.setText("Pitch Spray", juce::dontSendNotification);
  mLabelPitchSpray.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelPitchSpray.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPitchSpray);

  // Adjust position
  mSliderPosAdjust.setNumDecimalPlacesToDisplay(2);
  mSliderPosAdjust.setRange(ParamRanges::POSITION_ADJUST.start, ParamRanges::POSITION_ADJUST.end, 0.01);
  addAndMakeVisible(mSliderPosAdjust);

  mLabelPosAdjust.setText("Position Adjust", juce::dontSendNotification);
  mLabelPosAdjust.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelPosAdjust.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPosAdjust);

  // Position spray
  mSliderPosSpray.setTextValueSuffix("s");
  mSliderPosSpray.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
  mSliderPosSpray.setNumDecimalPlacesToDisplay(3);
  mSliderPosSpray.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, colour);
  mSliderPosSpray.setColour(juce::Slider::ColourIds::textBoxTextColourId, colour);
  mSliderPosSpray.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::white);
  mSliderPosSpray.setRange(ParamRanges::POSITION_SPRAY.start, ParamRanges::POSITION_SPRAY.end, 0.005);
  addAndMakeVisible(mSliderPosSpray);

  mLabelPosSpray.setText("Position Spray", juce::dontSendNotification);
  mLabelPosSpray.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelPosSpray.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPosSpray);

  // Adjust pan
  mSliderPanAdjust.setNumDecimalPlacesToDisplay(2);
  mSliderPanAdjust.setRange(ParamRanges::PAN_ADJUST.start, ParamRanges::PAN_ADJUST.end, 0.01);
  addAndMakeVisible(mSliderPanAdjust);

  mLabelPanAdjust.setText("Pan Adjust", juce::dontSendNotification);
  mLabelPanAdjust.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelPanAdjust.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPanAdjust);

  // Pan spray
  mSliderPanSpray.setTextValueSuffix("s");
  mSliderPanSpray.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
  mSliderPanSpray.setNumDecimalPlacesToDisplay(3);
  mSliderPanSpray.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, colour);
  mSliderPanSpray.setColour(juce::Slider::ColourIds::textBoxTextColourId, colour);
  mSliderPanSpray.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::white);
  mSliderPanSpray.setRange(ParamRanges::PAN_SPRAY.start, ParamRanges::PAN_SPRAY.end, 0.005);
  addAndMakeVisible(mSliderPanSpray);

  mLabelPanSpray.setText("Pan Spray", juce::dontSendNotification);
  mLabelPanSpray.setColour(juce::Label::ColourIds::textColourId, colour);
  mLabelPanSpray.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPanSpray);

  mPositionChanger.onPositionChanged = [this](bool isRight) {
    ParamGenerator* gen = dynamic_cast<ParamGenerator*>(mParameters.selectedParams);
    jassert(gen != nullptr);
    int numCandidates = mParameters.note.notes[gen->noteIdx]->candidates.size();
    int pos = gen->candidate->get();
    if (numCandidates == 0) return pos;
    int newPos = isRight ? pos + 1 : pos - 1;
    newPos = (newPos + numCandidates) % numCandidates;
    ParamHelper::setParam(gen->candidate, newPos);
    mPositionChanger.setPositionNumber(newPos);
  };
  mPositionChanger.onSoloChanged = [this](bool isSolo) {
    if (mCurSelectedParams->type == ParamType::GENERATOR) {
      ParamGenerator* gen = dynamic_cast<ParamGenerator*>(mCurSelectedParams);
      ParamHelper::setParam(mParameters.note.notes[gen->noteIdx]->soloIdx, isSolo ? gen->genIdx : SOLO_NONE);
    }
  };
  mPositionChanger.setColour(colour);
  addAndMakeVisible(mPositionChanger);

  mCurSelectedParams->addListener(this);
  updateSelectedParams();

  startTimer(100);
}

GrainControl::~GrainControl() { mMeter.setLookAndFeel(nullptr); }

void GrainControl::parameterValueChanged(int idx, float value) { mParamHasChanged.store(true); }

void GrainControl::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    mSliderGain.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::GAIN), juce::dontSendNotification);
    mSliderPitchAdjust.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::PITCH_ADJUST),
                                juce::dontSendNotification);
    mSliderPitchSpray.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::PITCH_SPRAY),
                               juce::dontSendNotification);
    mSliderPosAdjust.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::POS_ADJUST),
                              juce::dontSendNotification);
    mSliderPosSpray.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::POS_SPRAY),
                             juce::dontSendNotification);
    mSliderPanAdjust.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::PAN_ADJUST),
                              juce::dontSendNotification);
    mSliderPanSpray.setValue(mParameters.getFloatParam(mCurSelectedParams, ParamCommon::Type::PAN_SPRAY),
                             juce::dontSendNotification);
    if (mCurSelectedParams->type == ParamType::GENERATOR) {
      ParamGenerator* gen = dynamic_cast<ParamGenerator*>(mCurSelectedParams);
      mPositionChanger.setPositionNumber(gen->candidate->get());
      mPositionChanger.setSolo(mParameters.note.notes[gen->noteIdx]->soloIdx->get() == gen->genIdx);
    } else {
      mPositionChanger.setSolo(false);
    }
  }
}

void GrainControl::updateSelectedParams() { 
  if (mCurSelectedParams != nullptr) mCurSelectedParams->removeListener(this);
  mCurSelectedParams = mParameters.selectedParams;
  mCurSelectedParams->addListener(this);

  mParamColour = mParameters.getSelectedParamColour();
  mSliderGain.updateSelectedParams();
  mSliderPitchAdjust.updateSelectedParams();
  mSliderPitchSpray.updateSelectedParams();
  mSliderPosAdjust.updateSelectedParams();
  mSliderPosSpray.updateSelectedParams();
  mSliderPanAdjust.updateSelectedParams();
  mSliderPanSpray.updateSelectedParams();

  bool isGen = mCurSelectedParams->type == ParamType::GENERATOR;
  mPositionChanger.setActive(isGen);
  if (isGen) {
    ParamGenerator* gen = dynamic_cast<ParamGenerator*>(mCurSelectedParams);
    mPositionChanger.setNumPositions(mParameters.note.notes[gen->noteIdx]->candidates.size());
    mPositionChanger.setPositionNumber(gen->candidate->get());
    mPositionChanger.setColour(mParamColour);
  }
  
  mParamHasChanged.store(true);
  repaint();
}

void GrainControl::paint(juce::Graphics& g) {
  juce::Colour colour = mParamColour;

  // Section title
  g.setColour(Utils::GLOBAL_COLOUR);
  g.fillRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT);
  g.setColour(colour);
  g.drawRoundedRectangle(mTitleRect, Utils::ROUNDED_AMOUNT, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(juce::String(SECTION_TITLE), mTitleRect, juce::Justification::centred);
}

void GrainControl::resized() {
  juce::Rectangle<int> r = getLocalBounds();
  // Remove padding
  r = r.reduced(Utils::PADDING, Utils::PADDING).withCentre(r.getCentre());

  // Make title rect
  mTitleRect = r.removeFromTop(Utils::TITLE_HEIGHT).toFloat();

  r.removeFromTop(Utils::PADDING);
  
  int knobWidth = r.getWidth() / 3;

  r.removeFromLeft(Utils::PADDING);

  // Pitch spray and adjust
  juce::Rectangle<int> knobPanel = r.removeFromLeft(knobWidth);
  mLabelPitchSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPitchSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT).reduced(Utils::PADDING, 0));
  knobPanel.removeFromBottom(Utils::PADDING);
  mLabelPitchAdjust.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPitchAdjust.setBounds(
      knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));

  // Gain
  mLabelGain.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderGain.setBounds(
      knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));

  // Position spray and adjust
  knobPanel = r.removeFromRight(knobWidth);
  mLabelPosSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPosSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT).reduced(Utils::PADDING, 0));
  knobPanel.removeFromBottom(Utils::PADDING);
  mLabelPosAdjust.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPosAdjust.setBounds(
      knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));

  // Candidate changer
  mPositionChanger.setBounds(knobPanel.removeFromBottom(Utils::KNOB_HEIGHT + Utils::LABEL_HEIGHT));

  // Pan spray and adjust
  knobPanel = r;
  mLabelPanSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPanSpray.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT).reduced(Utils::PADDING, 0));
  knobPanel.removeFromBottom(Utils::PADDING);
  mLabelPanAdjust.setBounds(knobPanel.removeFromBottom(Utils::LABEL_HEIGHT));
  mSliderPanAdjust.setBounds(
      knobPanel.removeFromBottom(Utils::KNOB_HEIGHT).withSizeKeepingCentre(Utils::KNOB_HEIGHT * 2, Utils::KNOB_HEIGHT));

  // Meter
  mMeter.setBounds(knobPanel.removeFromBottom(Utils::KNOB_HEIGHT + Utils::LABEL_HEIGHT).reduced(Utils::PADDING));
}
