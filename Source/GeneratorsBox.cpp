/*
  ==============================================================================

    GeneratorsBox.cpp
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#include "GeneratorsBox.h"

#include <JuceHeader.h>

//==============================================================================
GeneratorsBox::GeneratorsBox(ParamsNote& paramsNote, ParamUI& paramUI)
    : mParamsNote(paramsNote),
      mParamUI(paramUI),
      mSliderRate(ParamRanges::GRAIN_RATE),
      mSliderDuration(ParamRanges::GRAIN_DURATION) {
  mCurPitchClass = (Utils::PitchClass)paramUI.pitchClass;
  mCurSelectedGenerator = (Utils::GeneratorColour)paramUI.generatorTab;
  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    ParamGenerator* gen = mParamsNote.notes[mCurPitchClass]->generators[i].get();
    mBtnsEnabled[i].setToggleState(gen->enable->get(), juce::dontSendNotification);
    juce::Colour tabColour = gen->enable->get() ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[i]) : juce::Colours::darkgrey;
    mBtnsEnabled[i].setColour(juce::ToggleButton::ColourIds::tickColourId, tabColour);
    mBtnsEnabled[i].onClick = [this, i] {
      if (!mParamsNote.notes[mCurPitchClass]->generators[i]->enable->get()) {
        changeGenerator((Utils::GeneratorColour)i);
      }
      ParamHelper::setParam(mParamsNote.notes[mCurPitchClass]->generators[i]->enable,
                            !mParamsNote.notes[mCurPitchClass]->generators[i]->enable->get());
      refreshState();
    };
    mBtnsEnabled[i].addMouseListener(this, false);
    addAndMakeVisible(mBtnsEnabled[i]);
  }

  mPositionChanger.onPositionChanged = [this](bool isRight) {
    if (onPositionChanged != nullptr) {
      onPositionChanged(mCurSelectedGenerator, isRight);
    }
  };
  mPositionChanger.onSoloChanged = [this](bool isSolo) {
    ParamHelper::setParam(mParamsNote.notes[mCurPitchClass]->soloIdx, isSolo ? mCurSelectedGenerator : SOLO_NONE);
  };
  addAndMakeVisible(mPositionChanger);

  /* Knob params */
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi;
  rotaryParams.stopAtEnd = true;

  bool enabled = getCurrentGenerator()->enable->get();
  mLabelShape.setEnabled(enabled);
  mLabelTilt.setEnabled(enabled);
  mLabelRate.setEnabled(enabled);
  mLabelDuration.setEnabled(enabled);
  mLabelGain.setEnabled(enabled);

  /* Adjust pitch */
  mSliderPitch.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderPitch.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderPitch.setRotaryParameters(rotaryParams);
  mSliderPitch.setNumDecimalPlacesToDisplay(2);
  mSliderPitch.setRange(ParamRanges::PITCH_ADJUST.start, ParamRanges::PITCH_ADJUST.end, 0.01);
  mSliderPitch.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->pitchAdjust, mSliderPitch.getValue()); };
  addAndMakeVisible(mSliderPitch);

  mLabelPitch.setText("Pitch Adjust", juce::dontSendNotification);
  mLabelPitch.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPitch);

  /* Adjust position */
  mSliderPosition.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderPosition.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderPosition.setRotaryParameters(rotaryParams);
  mSliderPosition.setNumDecimalPlacesToDisplay(2);
  mSliderPosition.setRange(ParamRanges::POSITION_ADJUST.start, ParamRanges::POSITION_ADJUST.end, 0.01);
  mSliderPosition.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->positionAdjust, mSliderPosition.getValue());
  };
  addAndMakeVisible(mSliderPosition);

  mLabelPosition.setText("Position Adjust", juce::dontSendNotification);
  mLabelPosition.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPosition);

  /* Shape */
  mSliderShape.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderShape.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderShape.setRotaryParameters(rotaryParams);
  mSliderShape.setNumDecimalPlacesToDisplay(2);
  mSliderShape.setRange(0.0, 1.0, 0.01);
  mSliderShape.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->grainShape, mSliderShape.getValue()); };
  addAndMakeVisible(mSliderShape);

  mLabelShape.setText("Shape", juce::dontSendNotification);
  mLabelShape.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelShape);

  /* Tilt */
  mSliderTilt.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderTilt.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderTilt.setRotaryParameters(rotaryParams);
  mSliderTilt.setNumDecimalPlacesToDisplay(2);
  mSliderTilt.setRange(0.0, 1.0, 0.01);
  mSliderTilt.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->grainTilt, mSliderTilt.getValue()); };
  addAndMakeVisible(mSliderTilt);

  mLabelTilt.setText("Tilt", juce::dontSendNotification);
  mLabelTilt.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelTilt);

  /* Sync */
  mBtnSync.setButtonText("free");
  mBtnSync.setColour(juce::TextButton::buttonColourId, juce::Colour(GRAIN_SYNC_COLOURS_HEX[0]));
  mBtnSync.setColour(juce::TextButton::buttonOnColourId, juce::Colour(GRAIN_SYNC_COLOURS_HEX[1]));
  mBtnSync.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
  mBtnSync.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
  mBtnSync.onClick = [this]() { ParamHelper::setParam(getCurrentGenerator()->grainSync, !mBtnSync.getToggleState()); };
  addAndMakeVisible(mBtnSync);

  /* Rate */
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRate.setRotaryParameters(rotaryParams);
  mSliderRate.setNumDecimalPlacesToDisplay(2);
  mSliderRate.setRange(ParamRanges::GRAIN_RATE.start, ParamRanges::GRAIN_RATE.end, 0.01);
  mSliderRate.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->grainRate, mSliderRate.getValue()); };
  addAndMakeVisible(mSliderRate);

  mLabelRate.setText("Rate", juce::dontSendNotification);
  mLabelRate.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRate);

  /* Duration */
  mSliderDuration.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDuration.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDuration.setRotaryParameters(rotaryParams);
  mSliderDuration.setNumDecimalPlacesToDisplay(2);
  mSliderDuration.setRange(ParamRanges::GRAIN_DURATION.start, ParamRanges::GRAIN_DURATION.end, 0.01);
  mSliderDuration.setTextValueSuffix("s");
  mSliderDuration.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->grainDuration, mSliderDuration.getValue());
  };
  addAndMakeVisible(mSliderDuration);

  mLabelDuration.setText("Duration", juce::dontSendNotification);
  mLabelDuration.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDuration);

  /* Grain envelope viz */
  addAndMakeVisible(mEnvelopeGrain);

  /* Amp envelope viz */
  addAndMakeVisible(mEnvelopeAmp);

  /* Attack */
  mSliderAttack.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderAttack.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderAttack.setRotaryParameters(rotaryParams);
  mSliderAttack.setNumDecimalPlacesToDisplay(2);
  mSliderAttack.setRange(ParamRanges::ATTACK.start, ParamRanges::ATTACK.end, 0.01);
  mSliderAttack.setTextValueSuffix("s");
  mSliderAttack.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->attack, mSliderAttack.getValue()); };
  addAndMakeVisible(mSliderAttack);

  mLabelAttack.setText("Attack", juce::dontSendNotification);
  mLabelAttack.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelAttack);

  /* Decay */
  mSliderDecay.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDecay.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDecay.setRotaryParameters(rotaryParams);
  mSliderDecay.setNumDecimalPlacesToDisplay(2);
  mSliderDecay.setRange(ParamRanges::DECAY.start, ParamRanges::DECAY.end, 0.01);
  mSliderDecay.setTextValueSuffix("s");
  mSliderDecay.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->decay, mSliderDecay.getValue()); };
  addAndMakeVisible(mSliderDecay);

  mLabelDecay.setText("Decay", juce::dontSendNotification);
  mLabelDecay.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDecay);

  /* Sustain */
  mSliderSustain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderSustain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderSustain.setRotaryParameters(rotaryParams);
  mSliderSustain.setNumDecimalPlacesToDisplay(2);
  mSliderSustain.setRange(0.0, 1.0, 0.01);
  mSliderSustain.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->sustain, mSliderSustain.getValue()); };
  addAndMakeVisible(mSliderSustain);

  mLabelSustain.setText("Sustain", juce::dontSendNotification);
  mLabelSustain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelSustain);

  /* Release */
  mSliderRelease.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRelease.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRelease.setRotaryParameters(rotaryParams);
  mSliderRelease.setNumDecimalPlacesToDisplay(2);
  mSliderRelease.setRange(ParamRanges::RELEASE.start, ParamRanges::RELEASE.end, 0.01);
  mSliderRelease.setTextValueSuffix("s");
  mSliderRelease.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->release, mSliderRelease.getValue()); };
  addAndMakeVisible(mSliderRelease);

  mLabelRelease.setText("Release", juce::dontSendNotification);
  mLabelRelease.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRelease);

  /* Gain */
  mSliderGain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderGain.setRotaryParameters(rotaryParams);
  mSliderGain.setNumDecimalPlacesToDisplay(2);
  mSliderGain.setRange(0.0, 1.0, 0.01);
  mSliderGain.onValueChange = [this] { ParamHelper::setParam(getCurrentGenerator()->gain, mSliderGain.getValue()); };
  addAndMakeVisible(mSliderGain);

  mLabelGain.setText("Gain", juce::dontSendNotification);
  mLabelGain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelGain);

  // set default generator for initialization
  setPitchClass(mCurPitchClass);
  startTimer(33);  // 30 fps
}

GeneratorsBox::~GeneratorsBox() {
  mParamsNote.notes[mCurPitchClass]->removeListener(mCurSelectedGenerator, this);
  stopTimer();
}

void GeneratorsBox::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  bool borderLit = mParamsNote.notes[mCurPitchClass]->shouldPlayGenerator(mCurSelectedGenerator);
  juce::Colour fillCol = borderLit ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[mCurSelectedGenerator]) : juce::Colours::darkgrey;
  g.setColour(fillCol);
  g.drawRoundedRectangle(getLocalBounds().withHeight(getHeight() + 10).translated(0, -11).toFloat().reduced(1.0f), 10.0f, 2.0f);

  g.setColour(juce::Colours::black);
  g.fillRect(0, 0, getWidth(), TABS_HEIGHT);

  float tabWidth = getWidth() / Utils::GeneratorColour::NUM_GEN;
  float curStart = 1.0f;
  for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
    ParamGenerator* gen = mParamsNote.notes[mCurPitchClass]->generators[i].get();
    juce::Colour tabColour = mParamsNote.notes[mCurPitchClass]->shouldPlayGenerator(i)
                                 ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[i])
                                 : juce::Colours::darkgrey;
    float tabHeight = (mCurSelectedGenerator == i) ? TABS_HEIGHT + 20.0f : TABS_HEIGHT - 2.0f;
    juce::Rectangle<float> tabRect = juce::Rectangle<float>(curStart, 1.0f, tabWidth - 2.0f, tabHeight);
    if (mCurHoverGenerator == i && mCurSelectedGenerator != i) {
      g.setColour(tabColour.withAlpha(0.3f));
      g.fillRoundedRectangle(tabRect, 10.0f);
    }
    g.setColour(tabColour);
    g.drawRoundedRectangle(tabRect, 10.0f, 2.0f);

    juce::Rectangle<int> textRect =
        juce::Rectangle<int>(mBtnsEnabled[i].getRight(), 0, tabRect.getRight() - mBtnsEnabled[i].getRight() - 6, TABS_HEIGHT);
    if (mParamsNote.notes[mCurPitchClass]->soloIdx->get() == i) {
      g.setColour(juce::Colours::blue);
      g.fillRect(textRect.withSizeKeepingCentre(textRect.getWidth() / 2.0f, textRect.getHeight() / 2.0f));
    }
    g.setColour(juce::Colours::white);
    g.drawFittedText(juce::String("g") + juce::String(i + 1), textRect, juce::Justification::centred, 1);
    curStart += tabWidth;
  }

  // Black out extended tabs with black rect
  g.setColour(juce::Colours::black);
  g.fillRect(2.0f, (float)TABS_HEIGHT, (float)getWidth() - 4.0f, 30.0f);

  // Lines to connect to tab
  g.setColour(fillCol);
  if (mCurSelectedGenerator > 0) {
    g.drawLine(1.0f, TABS_HEIGHT, mCurSelectedGenerator * tabWidth + 2.0f, TABS_HEIGHT, 2.0f);
  }
  if (mCurSelectedGenerator < Utils::GeneratorColour::NUM_GEN - 1) {
    g.drawLine((mCurSelectedGenerator + 1) * tabWidth - 2.0f, TABS_HEIGHT, getWidth(), TABS_HEIGHT, 2.0f);
  }

  // Adjustments section title
  juce::Rectangle<int> adjustTitleRect =
      juce::Rectangle<int>(0, mSliderPitch.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f), getWidth(), SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(adjustTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_ADJUST_TITLE), adjustTitleRect, juce::Justification::centred);

  // Amp env section title
  juce::Rectangle<int> ampEnvTitleRect =
      juce::Rectangle<int>(0, mEnvelopeAmp.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f), getWidth(), SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(ampEnvTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_AMP_ENV_TITLE), ampEnvTitleRect, juce::Justification::centred);

  // Grain env section title
  juce::Rectangle<int> grainEnvTitleRect =
      juce::Rectangle<int>(0, mEnvelopeGrain.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f), getWidth(),
                           SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(grainEnvTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_GRAIN_ENV_TITLE), grainEnvTitleRect, juce::Justification::centred);
}

void GeneratorsBox::resized() {
  auto r = getLocalBounds();

  // Enable/disable buttons
  juce::Rectangle<int> btnRect = juce::Rectangle<int>(TOGGLE_SIZE, TOGGLE_SIZE);
  float tabWidth = getWidth() / Utils::GeneratorColour::NUM_GEN - 2.0f;
  float curStart = 1.0f;
  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    mBtnsEnabled[i].setBounds(btnRect.withCentre(juce::Point<int>(curStart + (TOGGLE_SIZE / 2) + 5, TABS_HEIGHT / 2)));
    curStart += tabWidth + 2.0f;
  }
  r.removeFromTop(TABS_HEIGHT);

  // Add insets
  r.removeFromTop(PADDING_SIZE);
  r.removeFromLeft(PADDING_SIZE);
  r.removeFromRight(PADDING_SIZE);
  r.removeFromBottom(PADDING_SIZE);

  // Enable and solo buttons
  r.removeFromTop(SECTION_TITLE_HEIGHT);
  auto adjustmentsPanel = r.removeFromTop(ADJUSTMENT_HEIGHT + LABEL_HEIGHT);
  int adjustKnobWidth = adjustmentsPanel.getWidth() / 3;
  auto pitchKnobPanel = adjustmentsPanel.removeFromLeft(adjustKnobWidth);
  mLabelPitch.setBounds(pitchKnobPanel.removeFromBottom(LABEL_HEIGHT));
  mSliderPitch.setBounds(pitchKnobPanel.withSizeKeepingCentre(pitchKnobPanel.getHeight() * 2, pitchKnobPanel.getHeight()));
  auto posKnobPanel = adjustmentsPanel.removeFromRight(adjustKnobWidth);
  mLabelPosition.setBounds(posKnobPanel.removeFromBottom(LABEL_HEIGHT));
  mSliderPosition.setBounds(posKnobPanel.withSizeKeepingCentre(posKnobPanel.getHeight() * 2, posKnobPanel.getHeight()));
  mPositionChanger.setBounds(
      adjustmentsPanel.withSizeKeepingCentre(adjustmentsPanel.getWidth(), adjustmentsPanel.getHeight() / 1.5f));

  r.removeFromTop(PADDING_SIZE);

  // Amp envelope
  r.removeFromTop(SECTION_TITLE_HEIGHT);
  auto knobWidth = r.getWidth() / NUM_AMP_ENV_PARAMS;
  auto ampEnvPanel = r.removeFromTop(ENVELOPE_HEIGHT);
  auto gainPanel = ampEnvPanel.removeFromRight(knobWidth);
  mLabelGain.setBounds(gainPanel.removeFromBottom(LABEL_HEIGHT));
  mSliderGain.setBounds(gainPanel.withSizeKeepingCentre(gainPanel.getWidth(), gainPanel.getWidth() / 2.0f));
  mEnvelopeAmp.setBounds(ampEnvPanel.withTrimmedRight(PADDING_SIZE));
  r.removeFromTop(PADDING_SIZE);

  // Amp env knobs
  auto knobPanel = r.removeFromTop(knobWidth / 2);
  mSliderAttack.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDecay.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderSustain.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRelease.setBounds(knobPanel.removeFromLeft(knobWidth));

  auto labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelAttack.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDecay.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelSustain.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelRelease.setBounds(labelPanel.removeFromLeft(knobWidth));

  r.removeFromTop(PADDING_SIZE);

  // Grain envelopes
  r.removeFromTop(SECTION_TITLE_HEIGHT);
  knobWidth = r.getWidth() / NUM_GRAIN_ENV_PARAMS;
  auto grainPanel = r.removeFromTop(ENVELOPE_HEIGHT);
  auto syncPanel = grainPanel.removeFromRight(knobWidth);
  mBtnSync.changeWidthToFitText(LABEL_HEIGHT);
  mBtnSync.setCentrePosition(syncPanel.getCentre());
  mEnvelopeGrain.setBounds(grainPanel.withTrimmedRight(PADDING_SIZE));
  r.removeFromTop(PADDING_SIZE);

  // Grain env knobs
  knobPanel = r.removeFromTop(knobWidth / 2);
  mSliderShape.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderTilt.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDuration.setBounds(knobPanel.removeFromLeft(knobWidth));

  labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelShape.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelTilt.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelRate.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDuration.setBounds(labelPanel.removeFromLeft(knobWidth));
}

void GeneratorsBox::setPitchClass(Utils::PitchClass pitchClass) {
  // Remove listeners from old generator and note
  mParamsNote.notes[mCurPitchClass]->removeListener(mCurSelectedGenerator, this);

  // Update components and members
  mPositionChanger.setNumPositions(mParamsNote.notes[pitchClass]->candidates.size());
  mCurPitchClass = pitchClass;
  mParamUI.pitchClass = pitchClass;

  // Add listeners to new generator and note
  mParamsNote.notes[mCurPitchClass]->addListener(mCurSelectedGenerator, this);

  // Update UI Components
  mParamHasChanged.store(true);
}

void GeneratorsBox::mouseMove(const juce::MouseEvent& event) {
  if (event.y > TABS_HEIGHT) {
    mCurHoverGenerator = -1;
  } else {
    int tabHover = (event.getEventRelativeTo(this).getPosition().getX() / (float)getWidth()) * Utils::GeneratorColour::NUM_GEN;
    mCurHoverGenerator = tabHover;
  }
  repaint();
}

void GeneratorsBox::mouseExit(const juce::MouseEvent& event) {
  if (event.y > TABS_HEIGHT) return;
  mCurHoverGenerator = -1;
  repaint();
}

void GeneratorsBox::mouseUp(const juce::MouseEvent& event) {
  if (event.y > TABS_HEIGHT) return;
  if (event.eventComponent != this) return;
  int tabClick = (event.getEventRelativeTo(this).getPosition().getX() / (float)getWidth()) * Utils::GeneratorColour::NUM_GEN;
  changeGenerator((Utils::GeneratorColour)tabClick);
}

void GeneratorsBox::parameterValueChanged(int idx, float value) { mParamHasChanged.store(true); }

void GeneratorsBox::timerCallback() {
  if (mParamHasChanged.load()) {
    mParamHasChanged.store(false);
    ParamGenerator* gen = getCurrentGenerator();
    for (int i = 0; i < NUM_GENERATORS; ++i) {
      mBtnsEnabled[i].setToggleState(mParamsNote.notes[mCurPitchClass]->generators[i]->enable->get(), juce::dontSendNotification);
    }
    mPositionChanger.setSolo(mParamsNote.notes[mCurPitchClass]->soloIdx->get() == mCurSelectedGenerator);
    mPositionChanger.setPositionNumber(gen->candidate->get());
    mSliderPitch.setValue(gen->pitchAdjust->get(), juce::dontSendNotification);
    mSliderPosition.setValue(gen->positionAdjust->get(), juce::dontSendNotification);
    mSliderShape.setValue(gen->grainShape->get(), juce::dontSendNotification);
    mEnvelopeGrain.setShape(gen->grainShape->get());
    mSliderTilt.setValue(gen->grainTilt->get(), juce::dontSendNotification);
    mEnvelopeGrain.setTilt(gen->grainTilt->get());
    mBtnSync.setToggleState(gen->grainSync->get(), juce::dontSendNotification);
    mEnvelopeGrain.setSync(gen->grainSync->get());
    mBtnSync.setButtonText(mBtnSync.getToggleState() ? "sync" : "free");
    mSliderRate.setSync(gen->grainSync->get());
    mSliderRate.setValue(gen->grainRate->get(), juce::dontSendNotification);
    mEnvelopeGrain.setRate(ParamRanges::GRAIN_RATE.convertTo0to1(gen->grainRate->get()));
    mSliderDuration.setSync(gen->grainSync->get());
    mSliderDuration.setTextValueSuffix(gen->grainSync->get() ? "" : "s");
    mSliderDuration.setValue(gen->grainDuration->get(), juce::dontSendNotification);
    mEnvelopeGrain.setDuration(ParamRanges::GRAIN_DURATION.convertTo0to1(gen->grainDuration->get()));
    mSliderAttack.setValue(gen->attack->get(), juce::dontSendNotification);
    mEnvelopeAmp.setAttack(ParamRanges::ATTACK.convertTo0to1(gen->attack->get()));
    mSliderDecay.setValue(gen->decay->get(), juce::dontSendNotification);
    mEnvelopeAmp.setDecay(ParamRanges::DECAY.convertTo0to1(gen->decay->get()));
    mSliderSustain.setValue(gen->sustain->get(), juce::dontSendNotification);
    mEnvelopeAmp.setSustain(gen->sustain->get());
    mSliderRelease.setValue(gen->release->get(), juce::dontSendNotification);
    mEnvelopeAmp.setRelease(ParamRanges::RELEASE.convertTo0to1(gen->release->get()));
    mSliderGain.setValue(gen->gain->get(), juce::dontSendNotification);
    mEnvelopeAmp.setGain(gen->gain->get());
    refreshState();
  }
}

void GeneratorsBox::changeGenerator(Utils::GeneratorColour newGenerator) {
  if (newGenerator == mCurSelectedGenerator) return;
  // Remove listener from old generator
  getCurrentGenerator()->removeListener(this);
  // Update components and members
  mCurSelectedGenerator = newGenerator;
  mParamUI.generatorTab = newGenerator;

  // Add listener to new generator
  getCurrentGenerator()->addListener(this);

  // Update UI Components
  mParamHasChanged.store(true);
}

void GeneratorsBox::refreshState() {
  mPositionChanger.setSolo(mParamsNote.notes[mCurPitchClass]->soloIdx->get() == mCurSelectedGenerator);

  juce::Colour newColour = juce::Colour(Utils::GENERATOR_COLOURS_HEX[mCurSelectedGenerator]);
  mPositionChanger.setColour(newColour);
  mEnvelopeGrain.setColour(newColour);
  mEnvelopeAmp.setColour(newColour);

  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    juce::Colour tabColour = mParamsNote.notes[mCurPitchClass]->generators[i]->enable->get()
                                 ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[i])
                                 : juce::Colours::darkgrey;
    mBtnsEnabled[i].setColour(juce::ToggleButton::ColourIds::tickColourId, tabColour);
  }

  bool componentsLit = mParamsNote.notes[mCurPitchClass]->shouldPlayGenerator(mCurSelectedGenerator);
  juce::Colour knobColour =
      componentsLit ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[mCurSelectedGenerator]) : juce::Colours::darkgrey;
  mSliderPitch.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderPitch.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mLabelPitch.setEnabled(componentsLit);
  mSliderPosition.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderPosition.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                            componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mLabelPosition.setEnabled(componentsLit);
  mPositionChanger.setActive(componentsLit);

  mEnvelopeAmp.setActive(componentsLit);
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                          componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         componentsLit ? knobColour.brighter().brighter() : juce::Colours::darkgrey);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                           componentsLit ? knobColour.brighter().brighter().brighter() : juce::Colours::darkgrey);
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                           componentsLit ? knobColour.brighter().brighter().brighter().brighter() : juce::Colours::darkgrey);
  mLabelAttack.setEnabled(componentsLit);
  mLabelDecay.setEnabled(componentsLit);
  mLabelSustain.setEnabled(componentsLit);
  mLabelRelease.setEnabled(componentsLit);

  mEnvelopeGrain.setActive(componentsLit);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderTilt.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, knobColour);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mSliderTilt.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mLabelShape.setEnabled(componentsLit);
  mLabelTilt.setEnabled(componentsLit);
  mLabelRate.setEnabled(componentsLit);
  mLabelDuration.setEnabled(componentsLit);
  mLabelGain.setEnabled(componentsLit);
  repaint();
}