/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
GRainbowAudioProcessorEditor::GRainbowAudioProcessorEditor(
    GranularSynth& synth, juce::AudioProcessorValueTreeState& apvts)
    : AudioProcessorEditor(&synth),
      mSynth(synth),
      apvts(apvts),
      mKeyboard(mSynth.getKeyboardState()),
      mProgressBar(mLoadingProgress) {

  mSynth.onNoteChanged = [this](Utils::PitchClass pitchClass,
                                    bool isNoteOn) {
    if (isNoteOn) {
      mCurPitchClass = pitchClass;
      mStartedPlayingTrig = true;
    } else {
      mArcSpec.setNoteOff();
    }
  };

  mSynth.onBufferProcessed = [this](std::vector<std::vector<float>>* buffer,
                                    Utils::SpecType type) {
    mArcSpec.loadBuffer(buffer, type);
  };

  mSynth.onProgressUpdated = [this](float progress) {
    mLoadingProgress = progress;
    juce::MessageManagerLock lock;
    if (progress >= 1.0) {
      mProgressBar.setVisible(false);
    } else if (!mProgressBar.isVisible()) {
      mProgressBar.setVisible(true);
    }
  };

  setLookAndFeel(&mRainbowLookAndFeel);

  /* Open file button */
  juce::Image openFileNormal = juce::PNGImageFormat::loadFrom(
      BinaryData::openFileNormal_png, BinaryData::openFileNormal_pngSize);
  juce::Image openFileOver = juce::PNGImageFormat::loadFrom(
      BinaryData::openFileOver_png, BinaryData::openFileOver_pngSize);
  mBtnOpenFile.setImages(false, true, true, openFileNormal, 1.0f,
                         juce::Colours::transparentBlack, openFileOver, 1.0f,
                         juce::Colours::transparentBlack, openFileOver, 1.0f,
                         juce::Colours::transparentBlack);
  mBtnOpenFile.onClick = [this] { openNewFile(); };
  addAndMakeVisible(mBtnOpenFile);

  /* Recording button */
  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(
      BinaryData::microphone_png, BinaryData::microphone_pngSize);
  juce::Image recordOver = juce::PNGImageFormat::loadFrom(
      BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack);
  mBtnRecord.onClick = [this] {
    if (mRecorder.isRecording()) {
      stopRecording();
    } else {
      startRecording();
    }
  };
  addAndMakeVisible(mBtnRecord);

  /* Position tabs */
  mGeneratorTabs.onTabChanged = [this](Utils::GeneratorColour tab,
                                       bool isSelected, bool isEnabled) {
    mGeneratorBoxes[tab].setVisible(isSelected);
    mGeneratorBoxes[tab].setActive(isEnabled);
    mSynth.updateGeneratorParameter(tab, GranularSynth::ParameterType::ENABLED,
                                     isEnabled);
    std::vector<bool> positionStates;
    for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
      bool canPlay =
          mGeneratorBoxes[i].getState() != GeneratorBox::BoxState::SOLO_WAIT;
      bool shouldPlay =
          mGeneratorBoxes[i].getActive() ||
          mGeneratorBoxes[i].getState() == GeneratorBox::BoxState::SOLO;
      positionStates.push_back(canPlay && shouldPlay);
    }
    mSynth.updateGeneratorStates(positionStates);
    mArcSpec.setPositions(mSynth.getCurrentPositions());
  };
  addAndMakeVisible(mGeneratorTabs);

  /* Position boxes */
  for (int i = 0; i < mGeneratorBoxes.size(); ++i) {
    mGeneratorBoxes[i].setColour((Utils::GeneratorColour)i);
    mGeneratorBoxes[i].setActive(i == 0);
    mGeneratorBoxes[i].setParams(
        mSynth.getGeneratorParams((Utils::GeneratorColour)i));
    mGeneratorBoxes[i].onPositionChanged = [this, i](bool isRight) {
      int newPosition = mSynth.incrementPosition(i, isRight);
      mGeneratorBoxes[i].setPositionNumber(newPosition);
      mArcSpec.setPositions(mSynth.getCurrentPositions());
    };
    mGeneratorBoxes[i].onParameterChanged =
        [this](Utils::GeneratorColour pos, GranularSynth::ParameterType param,
               float value) {
          if (param == GranularSynth::ParameterType::SOLO) {
            std::vector<bool> positionStates;
            for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
              if (i != pos) {
                mGeneratorBoxes[i].setState(
                    (value == true) ? GeneratorBox::BoxState::SOLO_WAIT
                                    : GeneratorBox::BoxState::READY);
              }
              bool canPlay = mGeneratorBoxes[i].getState() !=
                             GeneratorBox::BoxState::SOLO_WAIT;
              bool shouldPlay =
                  mGeneratorBoxes[i].getActive() ||
                  mGeneratorBoxes[i].getState() == GeneratorBox::BoxState::SOLO;
              positionStates.push_back(canPlay && shouldPlay);
            }
            mSynth.updateGeneratorStates(positionStates);
          }
          mSynth.updateGeneratorParameter(pos, param, value);
        };
    addChildComponent(mGeneratorBoxes[i]);
    if (i == 0) mGeneratorBoxes[i].setVisible(true);
  }

  /* Global parameter box */
  mGlobalParamBox.setParams(mSynth.getGlobalParams());
  mGlobalParamBox.onParameterChanged =
      [this](GranularSynth::ParameterType param, float value) {
        mSynth.updateGlobalParameter(param, value);
      };
  addAndMakeVisible(mGlobalParamBox);

  /* Arc spectrogram */
  addAndMakeVisible(mArcSpec);

  addChildComponent(mProgressBar);

  addAndMakeVisible(mKeyboard);

  mAudioDeviceManager.initialise(1, 2, nullptr, true, {}, nullptr);

  mAudioDeviceManager.addAudioCallback(&mRecorder);

  startTimer(50);  // Keyboard focus timer

  setSize(1200, 565);
}

GRainbowAudioProcessorEditor::~GRainbowAudioProcessorEditor() {
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  auto recordFile = parentDir.getChildFile(RECORDING_FILE);
  recordFile.deleteFile();
  mAudioDeviceManager.removeAudioCallback(&mRecorder);
  setLookAndFeel(nullptr);
}

void GRainbowAudioProcessorEditor::timerCallback() {
  if (mStartedPlayingTrig && mCurPitchClass != Utils::PitchClass::NONE) {
    int numFoundPositions = mSynth.getNumFoundPositions();
    std::vector<bool> tabStates;
    for (int i = 0; i < mGeneratorBoxes.size(); ++i) {
      mGeneratorBoxes[i].setParams(
          mSynth.getGeneratorParams((Utils::GeneratorColour)i));
      mGeneratorBoxes[i].setNumPositions(numFoundPositions);
      tabStates.push_back(
          mSynth.getGeneratorParams((Utils::GeneratorColour)i)
              .isActive);
    }
    mGeneratorTabs.setTabStates(tabStates);
    std::vector<GrainPositionFinder::GrainPosition> positions =
        mSynth.getCurrentPositions();
    mArcSpec.setNoteOn(mCurPitchClass, mSynth.getCurrentPositions());
    mStartedPlayingTrig = false;
    repaint();  // Update note display
  }
}

//==============================================================================
void GRainbowAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw background for open file button
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnOpenFile.getBounds().toFloat(), 14);

  // Draw background for record button
  g.setColour(mRecorder.isRecording() ? juce::Colours::red
                                      : juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnRecord.getBounds().toFloat(), 14);
}

void GRainbowAudioProcessorEditor::paintOverChildren(juce::Graphics& g) {
  // Draw note display
  if (mCurPitchClass != Utils::PitchClass::NONE) {
    std::vector<GrainPositionFinder::GrainPosition> positions =
        mSynth.getCurrentPositions();
    // Draw position arrows
    for (int i = 0; i < positions.size(); ++i) {
      if (positions[i].pitch.pitchClass == Utils::PitchClass::NONE) continue;
      g.setColour(juce::Colour(Utils::POSITION_COLOURS[i]));
      auto middlePos =
          positions[i].pitch.posRatio + (positions[i].pitch.duration / 2.0f);
      float angleRad = (juce::MathConstants<float>::pi * middlePos) -
                       (juce::MathConstants<float>::pi / 2.0f);
      juce::Point<float> startPoint = juce::Point<float>(
          mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
      juce::Point<float> endPoint = startPoint.getPointOnCircumference(
          mArcSpec.getHeight() / 4.5f, angleRad);
      g.drawArrow(juce::Line<float>(startPoint, endPoint), 4.0f, 10.0f, 6.0f);
    }
    // Draw path to positions
    float noteX =
        mKeyboard.getBounds().getX() +
        (mKeyboard.getWidth() * mKeyboard.getPitchXRatio(mCurPitchClass));
    juce::Path displayPath;
    displayPath.startNewSubPath(noteX, mNoteDisplayRect.getBottom());
    displayPath.lineTo(
        noteX, mNoteDisplayRect.getBottom() - (NOTE_DISPLAY_HEIGHT / 2.0f));
    displayPath.lineTo(mNoteDisplayRect.getCentre());
    displayPath.lineTo(mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
    g.setColour(Utils::getRainbow12Colour(mCurPitchClass));
    g.strokePath(displayPath, juce::PathStrokeType(4.0f));
    g.fillEllipse(mNoteDisplayRect.getCentreX() - (NOTE_BULB_SIZE / 2.0f),
                  mNoteDisplayRect.getY() - (NOTE_BULB_SIZE / 2.0f),
                  NOTE_BULB_SIZE, NOTE_BULB_SIZE);
  }
}

void GRainbowAudioProcessorEditor::resized() {
  auto r = getLocalBounds();

  // Generator tabs
  auto leftPanel = r.removeFromLeft(PANEL_WIDTH);
  mGeneratorTabs.setBounds(leftPanel.removeFromTop(TABS_HEIGHT));
  // Generator boxes
  for (int i = 0; i < mGeneratorBoxes.size(); ++i) {
    mGeneratorBoxes[i].setBounds(leftPanel);
  }

  auto rightPanel = r.removeFromRight(PANEL_WIDTH);
  mGlobalParamBox.setBounds(rightPanel);

  // Open and record buttons
  auto filePanel = r.removeFromTop(BTN_PANEL_HEIGHT + BTN_PADDING);
  filePanel.removeFromLeft(BTN_PADDING);
  filePanel.removeFromTop(BTN_PADDING);
  mBtnOpenFile.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));
  filePanel.removeFromLeft(BTN_PADDING);
  mBtnRecord.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));

  r.removeFromTop(NOTE_DISPLAY_HEIGHT);  // Just padding

  // Arc spectrogram
  mArcSpec.setBounds(r.removeFromTop(r.getWidth() / 2.0f));
  mProgressBar.setBounds(
      mArcSpec.getBounds().withSizeKeepingCentre(PROGRESS_SIZE, PROGRESS_SIZE));

  // Space for note display
  mNoteDisplayRect = r.removeFromTop(NOTE_DISPLAY_HEIGHT).toFloat();

  // Keyboard
  juce::Rectangle<int> keyboardRect =
      r.removeFromTop(r.getHeight() - NOTE_DISPLAY_HEIGHT)
          .reduced(NOTE_DISPLAY_HEIGHT, 0.0f);
  mKeyboard.setBounds(keyboardRect);
}

/** Pauses audio to open file
    @param path optional path to load, otherwise will prompt user for file
   location
*/
void GRainbowAudioProcessorEditor::openNewFile(const char* path) {
  if (path == nullptr) {
    juce::FileChooser chooser("Select a file to granulize...",
                              juce::File::getCurrentWorkingDirectory(),
                              "*.wav;*.mp3", true);

    if (chooser.browseForFileToOpen()) {
      auto file = chooser.getResult();
      processFile(file);
    }
  } else {
    auto file = juce::File(juce::String(path));
    processFile(file);
  }
}

void GRainbowAudioProcessorEditor::processFile(juce::File file) {
  mSynth.processFile(file);  
  mArcSpec.resetBuffers();
}

void GRainbowAudioProcessorEditor::startRecording() {
  if (!juce::RuntimePermissions::isGranted(
          juce::RuntimePermissions::writeExternalStorage)) {
    SafePointer<GRainbowAudioProcessorEditor> safeThis(this);

    juce::RuntimePermissions::request(
        juce::RuntimePermissions::writeExternalStorage,
        [safeThis](bool granted) mutable {
          if (granted) safeThis->startRecording();
        });
    return;
  }

  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  parentDir.getChildFile(RECORDING_FILE).deleteFile();
  mRecordedFile = parentDir.getChildFile(RECORDING_FILE);

  mRecorder.startRecording(mRecordedFile);

  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(
      BinaryData::microphone_png, BinaryData::microphone_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordIcon, 1.0f,
                       juce::Colours::transparentBlack);
  repaint();
}

void GRainbowAudioProcessorEditor::stopRecording() {
  mRecorder.stop();

  processFile(mRecordedFile);

  mRecordedFile = juce::File();

  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(
      BinaryData::microphone_png, BinaryData::microphone_pngSize);
  juce::Image recordOver = juce::PNGImageFormat::loadFrom(
      BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack);
  repaint();
}