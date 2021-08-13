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
GeneratorsBox::GeneratorsBox(NoteParams& noteParams, UIParams& uiParams)
    : mNoteParams(noteParams), mUIParams(uiParams) {
  mCurPitchClass = (Utils::PitchClass)uiParams.pitchClass;
  mCurSelectedGenerator = (Utils::GeneratorColour)uiParams.generatorTab;
  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    GeneratorParams* gen =
        mNoteParams.notes[mCurPitchClass]->generators[i].get();
    mBtnsEnabled[i].setToggleState(gen->enable->get(),
                                   juce::dontSendNotification);
    juce::Colour tabColour = gen->enable->get()
                                 ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[i])
                                 : juce::Colours::darkgrey;
    mBtnsEnabled[i].setColour(juce::ToggleButton::ColourIds::tickColourId,
                              tabColour);
    mBtnsEnabled[i].onClick = [this, i] {
      if (!mNoteParams.notes[mCurPitchClass]->generators[i]->enable->get()) {
        changeGenerator((Utils::GeneratorColour)i);
      }
      ParamHelper::setParam(
          mNoteParams.notes[mCurPitchClass]->generators[i]->enable,
          !mNoteParams.notes[mCurPitchClass]->generators[i]->enable->get());
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
    ParamHelper::setParam(mNoteParams.notes[mCurPitchClass]->soloIdx,
                          isSolo ? mCurSelectedGenerator : SOLO_NONE);
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
  mSliderPitch.setRange(ParamRanges::PITCH_ADJUST.start,
                        ParamRanges::PITCH_ADJUST.end);
  mSliderPitch.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->pitchAdjust,
                          mSliderPitch.getValue());
  };
  addAndMakeVisible(mSliderPitch);

  mLabelPitch.setText("Pitch Adjust", juce::dontSendNotification);
  mLabelPitch.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPitch);

  /* Adjust position */
  mSliderPosition.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderPosition.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderPosition.setRotaryParameters(rotaryParams);
  mSliderPosition.setRange(ParamRanges::POSITION_ADJUST.start,
                           ParamRanges::POSITION_ADJUST.end);
  mSliderPosition.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->positionAdjust,
                          mSliderPosition.getValue());
  };
  addAndMakeVisible(mSliderPosition);

  mLabelPosition.setText("Position Adjust", juce::dontSendNotification);
  mLabelPosition.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelPosition);

  /* Shape */
  mSliderShape.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderShape.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderShape.setRotaryParameters(rotaryParams);
  mSliderShape.setRange(0.0, 1.0);
  mSliderShape.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->grainShape,
                          mSliderShape.getValue());
  };
  addAndMakeVisible(mSliderShape);

  mLabelShape.setText("Shape", juce::dontSendNotification);
  mLabelShape.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelShape);

  /* Tilt */
  mSliderTilt.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderTilt.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderTilt.setRotaryParameters(rotaryParams);
  mSliderTilt.setRange(0.0, 1.0);
  mSliderTilt.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->grainTilt,
                          mSliderTilt.getValue());
  };
  addAndMakeVisible(mSliderTilt);

  mLabelTilt.setText("Tilt", juce::dontSendNotification);
  mLabelTilt.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelTilt);

  /* Rate */
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRate.setRotaryParameters(rotaryParams);
  mSliderRate.setRange(ParamRanges::GRAIN_RATE.start,
                       ParamRanges::GRAIN_RATE.end);
  mSliderRate.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->grainRate,
                          mSliderRate.getValue());
  };
  addAndMakeVisible(mSliderRate);

  mLabelRate.setText("Rate", juce::dontSendNotification);
  mLabelRate.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRate);

  /* Duration */
  mSliderDuration.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDuration.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDuration.setRotaryParameters(rotaryParams);
  mSliderDuration.setRange(ParamRanges::GRAIN_DURATION.start,
                           ParamRanges::GRAIN_DURATION.end);
  mSliderDuration.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->grainDuration,
                          mSliderDuration.getValue());
  };
  addAndMakeVisible(mSliderDuration);

  mLabelDuration.setText("Duration", juce::dontSendNotification);
  mLabelDuration.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDuration);

  /* Gain */
  mSliderGain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderGain.setRotaryParameters(rotaryParams);
  mSliderGain.setRange(0.0, 1.0);
  mSliderGain.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->grainGain,
                          mSliderGain.getValue());
  };
  addAndMakeVisible(mSliderGain);

  mLabelGain.setText("Gain", juce::dontSendNotification);
  mLabelGain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelGain);

  /* Grain envelope viz */
  addAndMakeVisible(mEnvelopeGrain);

  /* Amp envelope viz */
  addAndMakeVisible(mEnvelopeAmp);

  /* Attack */
  mSliderAttack.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderAttack.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderAttack.setRotaryParameters(rotaryParams);
  mSliderAttack.setRange(ParamRanges::ATTACK.start, ParamRanges::ATTACK.end);
  mSliderAttack.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->attack,
                          mSliderAttack.getValue());
  };
  addAndMakeVisible(mSliderAttack);

  mLabelAttack.setText("Attack", juce::dontSendNotification);
  mLabelAttack.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelAttack);

  /* Decay */
  mSliderDecay.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDecay.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDecay.setRotaryParameters(rotaryParams);
  mSliderDecay.setRange(ParamRanges::DECAY.start, ParamRanges::DECAY.end);
  mSliderDecay.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->decay,
                          mSliderDecay.getValue());
  };
  addAndMakeVisible(mSliderDecay);

  mLabelDecay.setText("Decay", juce::dontSendNotification);
  mLabelDecay.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDecay);

  /* Sustain */
  mSliderSustain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderSustain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderSustain.setRotaryParameters(rotaryParams);
  mSliderSustain.setRange(0.0, 1.0);
  mSliderSustain.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->sustain,
                          mSliderSustain.getValue());
  };
  addAndMakeVisible(mSliderSustain);

  mLabelSustain.setText("Sustain", juce::dontSendNotification);
  mLabelSustain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelSustain);

  /* Release */
  mSliderRelease.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRelease.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRelease.setRotaryParameters(rotaryParams);
  mSliderRelease.setRange(ParamRanges::RELEASE.start, ParamRanges::RELEASE.end);
  mSliderRelease.onValueChange = [this] {
    ParamHelper::setParam(getCurrentGenerator()->release,
                          mSliderRelease.getValue());
  };
  addAndMakeVisible(mSliderRelease);

  mLabelRelease.setText("Release", juce::dontSendNotification);
  mLabelRelease.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRelease);

  // set default generator for initialization
  mNoteParams.notes[mCurPitchClass]->soloIdx->addListener(this);
  changeGenerator(mCurSelectedGenerator);
  startTimer(33);  // 30 fps
}

GeneratorsBox::~GeneratorsBox() { stopTimer(); }

void GeneratorsBox::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  bool borderLit = mNoteParams.notes[mCurPitchClass]->shouldPlayGenerator(
      mCurSelectedGenerator);
  juce::Colour fillCol =
      borderLit
          ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[mCurSelectedGenerator])
          : juce::Colours::darkgrey;
  g.setColour(fillCol);
  g.drawRoundedRectangle(getLocalBounds()
                             .withHeight(getHeight() + 10)
                             .translated(0, -11)
                             .toFloat()
                             .reduced(1.0f),
                         10.0f, 2.0f);

  g.setColour(juce::Colours::black);
  g.fillRect(0, 0, getWidth(), TABS_HEIGHT);

  float tabWidth = getWidth() / Utils::GeneratorColour::NUM_GEN;
  float curStart = 1.0f;
  for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
    GeneratorParams* gen =
        mNoteParams.notes[mCurPitchClass]->generators[i].get();
    juce::Colour tabColour =
        mNoteParams.notes[mCurPitchClass]->shouldPlayGenerator(i)
            ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[i])
            : juce::Colours::darkgrey;
    float tabHeight =
        (mCurSelectedGenerator == i) ? TABS_HEIGHT + 20.0f : TABS_HEIGHT - 2.0f;
    juce::Rectangle<float> tabRect =
        juce::Rectangle<float>(curStart, 1.0f, tabWidth - 2.0f, tabHeight);
    if (mCurHoverGenerator == i && mCurSelectedGenerator != i) {
      g.setColour(tabColour.withAlpha(0.3f));
      g.fillRoundedRectangle(tabRect, 10.0f);
    }
    g.setColour(tabColour);
    g.drawRoundedRectangle(tabRect, 10.0f, 2.0f);

    juce::Rectangle<int> textRect = juce::Rectangle<int>(
        mBtnsEnabled[i].getRight(), 0,
        tabRect.getRight() - mBtnsEnabled[i].getRight() - 6, TABS_HEIGHT);
    if (mNoteParams.notes[mCurPitchClass]->soloIdx->get() == i) {
      g.setColour(juce::Colours::blue);
      g.fillRect(textRect.withSizeKeepingCentre(textRect.getWidth() / 2.0f,
                                                textRect.getHeight() / 2.0f));
    }
    g.setColour(juce::Colours::white);
    g.drawFittedText(juce::String("g") + juce::String(i + 1), textRect,
                     juce::Justification::centred, 1);
    curStart += tabWidth;
  }

  // Black out extended tabs with black rect
  g.setColour(juce::Colours::black);
  g.fillRect(2.0f, (float)TABS_HEIGHT, (float)getWidth() - 4.0f, 30.0f);

  // Lines to connect to tab
  g.setColour(fillCol);
  if (mCurSelectedGenerator > 0) {
    g.drawLine(1.0f, TABS_HEIGHT, mCurSelectedGenerator * tabWidth + 2.0f,
               TABS_HEIGHT, 2.0f);
  }
  if (mCurSelectedGenerator < Utils::GeneratorColour::NUM_GEN - 1) {
    g.drawLine((mCurSelectedGenerator + 1) * tabWidth - 2.0f, TABS_HEIGHT,
               getWidth(), TABS_HEIGHT, 2.0f);
  }

  // Adjustments section title
  juce::Rectangle<int> adjustTitleRect =
      juce::Rectangle<int>(
          0, mSliderPitch.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f),
          getWidth(), SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(adjustTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_ADJUST_TITLE), adjustTitleRect,
             juce::Justification::centred);

  // Amp env section title
  juce::Rectangle<int> ampEnvTitleRect =
      juce::Rectangle<int>(
          0, mEnvelopeAmp.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f),
          getWidth(), SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(ampEnvTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_AMP_ENV_TITLE), ampEnvTitleRect,
             juce::Justification::centred);

  // Grain env section title
  juce::Rectangle<int> grainEnvTitleRect =
      juce::Rectangle<int>(
          0,
          mEnvelopeGrain.getY() - SECTION_TITLE_HEIGHT - (PADDING_SIZE / 2.0f),
          getWidth(), SECTION_TITLE_HEIGHT)
          .reduced(PADDING_SIZE, PADDING_SIZE / 2);
  g.setColour(fillCol);
  g.fillRect(grainEnvTitleRect);
  g.setColour(juce::Colours::black);
  g.drawText(juce::String(SECTION_GRAIN_ENV_TITLE), grainEnvTitleRect,
             juce::Justification::centred);
}

void GeneratorsBox::resized() {
  auto r = getLocalBounds();

  // Enable/disable buttons
  juce::Rectangle<int> btnRect = juce::Rectangle<int>(TOGGLE_SIZE, TOGGLE_SIZE);
  float tabWidth = getWidth() / Utils::GeneratorColour::NUM_GEN - 2.0f;
  float curStart = 1.0f;
  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    mBtnsEnabled[i].setBounds(btnRect.withCentre(
        juce::Point<int>(curStart + (TOGGLE_SIZE / 2) + 5, TABS_HEIGHT / 2)));
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
  mSliderPitch.setBounds(pitchKnobPanel.withSizeKeepingCentre(
      pitchKnobPanel.getHeight() * 2, pitchKnobPanel.getHeight()));
  auto posKnobPanel = adjustmentsPanel.removeFromRight(adjustKnobWidth);
  mLabelPosition.setBounds(posKnobPanel.removeFromBottom(LABEL_HEIGHT));
  mSliderPosition.setBounds(posKnobPanel.withSizeKeepingCentre(
      posKnobPanel.getHeight() * 2, posKnobPanel.getHeight()));
  mPositionChanger.setBounds(adjustmentsPanel.withSizeKeepingCentre(
      adjustmentsPanel.getWidth(), adjustmentsPanel.getHeight() / 1.5f));

  r.removeFromTop(PADDING_SIZE);

  // Amp envelope
  r.removeFromTop(SECTION_TITLE_HEIGHT);
  mEnvelopeAmp.setBounds(r.removeFromTop(ENVELOPE_HEIGHT));
  r.removeFromTop(PADDING_SIZE);

  // Amp env knobs
  auto knobWidth = r.getWidth() / NUM_AMP_ENV_PARAMS;
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
  mEnvelopeGrain.setBounds(r.removeFromTop(ENVELOPE_HEIGHT));
  r.removeFromTop(PADDING_SIZE);

  // Grain env knobs
  knobWidth = r.getWidth() / NUM_GRAIN_ENV_PARAMS;
  knobPanel = r.removeFromTop(knobWidth / 2);
  mSliderShape.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderTilt.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDuration.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderGain.setBounds(knobPanel.removeFromLeft(knobWidth));

  labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelShape.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelTilt.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelRate.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDuration.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelGain.setBounds(labelPanel.removeFromLeft(knobWidth));
}

void GeneratorsBox::setPitchClass(Utils::PitchClass pitchClass) {
  mNoteParams.notes[mCurPitchClass]->soloIdx->removeListener(this);
  mNoteParams.notes[pitchClass]->soloIdx->addListener(this);
  mCurPitchClass = pitchClass;
  mUIParams.pitchClass = pitchClass;
  changeGenerator(mCurSelectedGenerator);
}

void GeneratorsBox::mouseMove(const juce::MouseEvent& event) {
  if (event.y > TABS_HEIGHT) {
    mCurHoverGenerator = -1;
  } else {
    int tabHover = (event.getEventRelativeTo(this).getPosition().getX() /
                    (float)getWidth()) *
                   Utils::GeneratorColour::NUM_GEN;
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
  int tabClick = (event.getEventRelativeTo(this).getPosition().getX() /
                  (float)getWidth()) *
                 Utils::GeneratorColour::NUM_GEN;
  changeGenerator((Utils::GeneratorColour)tabClick);
}

void GeneratorsBox::parameterValueChanged(int idx, float value) {
  mParamHasChanged.store(true);
}

void GeneratorsBox::timerCallback() {
  if (mParamHasChanged.load()) {
    GeneratorParams* gen = getCurrentGenerator();
    for (int i = 0; i < NUM_GENERATORS; ++i) {
      mBtnsEnabled[i].setToggleState(
          mNoteParams.notes[mCurPitchClass]->generators[i]->enable->get(),
          juce::dontSendNotification);
    }
    mPositionChanger.setSolo(
        mNoteParams.notes[mCurPitchClass]->soloIdx->get() ==
        mCurSelectedGenerator);
    mPositionChanger.setPositionNumber(gen->candidate->get());
    mSliderPitch.setValue(gen->pitchAdjust->get(), juce::dontSendNotification);
    mSliderPosition.setValue(gen->positionAdjust->get(),
                             juce::dontSendNotification);
    mSliderShape.setValue(gen->grainShape->get(), juce::dontSendNotification);
    mEnvelopeGrain.setShape(gen->grainShape->get());
    mSliderTilt.setValue(gen->grainTilt->get(), juce::dontSendNotification);
    mEnvelopeGrain.setTilt(gen->grainTilt->get());
    mSliderRate.setValue(gen->grainRate->get(), juce::dontSendNotification);
    mEnvelopeGrain.setRate(
        ParamRanges::GRAIN_RATE.convertTo0to1(gen->grainRate->get()));
    mSliderDuration.setValue(gen->grainDuration->get(),
                             juce::dontSendNotification);
    mEnvelopeGrain.setDuration(
        ParamRanges::GRAIN_DURATION.convertTo0to1(gen->grainDuration->get()));
    mSliderGain.setValue(gen->grainGain->get(), juce::dontSendNotification);
    mEnvelopeGrain.setGain(gen->grainGain->get());
    mSliderAttack.setValue(gen->attack->get(), juce::dontSendNotification);
    mEnvelopeAmp.setAttack(
        ParamRanges::ATTACK.convertTo0to1(gen->attack->get()));
    mSliderDecay.setValue(gen->decay->get(), juce::dontSendNotification);
    mEnvelopeAmp.setDecay(ParamRanges::DECAY.convertTo0to1(gen->decay->get()));
    mSliderSustain.setValue(gen->sustain->get(), juce::dontSendNotification);
    mEnvelopeAmp.setSustain(gen->sustain->get());
    mSliderRelease.setValue(gen->release->get(), juce::dontSendNotification);
    mEnvelopeAmp.setRelease(
        ParamRanges::RELEASE.convertTo0to1(gen->release->get()));
    refreshState();
    mParamHasChanged.store(false);
  }
}

void GeneratorsBox::changeGenerator(Utils::GeneratorColour newGenerator) {
  // Replace parameter listener with new generator
  getCurrentGenerator()->removeListener(this);
  mNoteParams.notes[mCurPitchClass]->generators[newGenerator]->addListener(
      this);
  mCurSelectedGenerator = newGenerator;
  mUIParams.generatorTab = newGenerator;
  // Update UI Components
  mParamHasChanged.store(true);
}

void GeneratorsBox::refreshState() {
  mPositionChanger.setSolo(mNoteParams.notes[mCurPitchClass]->soloIdx->get() ==
                           mCurSelectedGenerator);

  juce::Colour newColour =
      juce::Colour(Utils::GENERATOR_COLOURS_HEX[mCurSelectedGenerator]);
  mPositionChanger.setColour(newColour);
  mEnvelopeGrain.setColour(newColour);
  mEnvelopeAmp.setColour(newColour);

  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    juce::Colour tabColour =
        mNoteParams.notes[mCurPitchClass]->generators[i]->enable->get()
            ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[i])
            : juce::Colours::darkgrey;
    mBtnsEnabled[i].setColour(juce::ToggleButton::ColourIds::tickColourId,
                              tabColour);
  }

  bool componentsLit = mNoteParams.notes[mCurPitchClass]->shouldPlayGenerator(
      mCurSelectedGenerator);
  juce::Colour knobColour =
      componentsLit
          ? juce::Colour(Utils::GENERATOR_COLOURS_HEX[mCurSelectedGenerator])
          : juce::Colours::darkgrey;
  mSliderPitch.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         knobColour);
  mSliderPitch.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mLabelPitch.setEnabled(componentsLit);
  mSliderPosition.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                            knobColour);
  mSliderPosition.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mLabelPosition.setEnabled(componentsLit);
  mPositionChanger.setActive(componentsLit);

  mEnvelopeAmp.setActive(componentsLit);
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                          knobColour);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         knobColour);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                           knobColour);
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                           knobColour);
  mSliderAttack.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit ? knobColour.brighter() : juce::Colours::darkgrey);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         componentsLit ? knobColour.brighter().brighter()
                                       : juce::Colours::darkgrey);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                           componentsLit
                               ? knobColour.brighter().brighter().brighter()
                               : juce::Colours::darkgrey);
  mSliderRelease.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit ? knobColour.brighter().brighter().brighter().brighter()
                    : juce::Colours::darkgrey);
  mLabelAttack.setEnabled(componentsLit);
  mLabelDecay.setEnabled(componentsLit);
  mLabelSustain.setEnabled(componentsLit);
  mLabelRelease.setEnabled(componentsLit);

  mEnvelopeGrain.setActive(componentsLit);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         knobColour);
  mSliderTilt.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                            knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderShape.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                         knobColour);
  mSliderTilt.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mSliderDuration.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mLabelShape.setEnabled(componentsLit);
  mLabelTilt.setEnabled(componentsLit);
  mLabelRate.setEnabled(componentsLit);
  mLabelDuration.setEnabled(componentsLit);
  mLabelGain.setEnabled(componentsLit);
  repaint();
}