/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
GRainbowAudioProcessorEditor::GRainbowAudioProcessorEditor(
    GranularSynth& synth)
    : AudioProcessorEditor(&synth),
      mSynth(synth),
      mGlobalParamBox(synth.getGlobalParams()),
      mGeneratorsBox(synth.getNoteParams()),
      mArcSpec(synth.getNoteParams()),
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

  /* Generators box */
  mGeneratorsBox.onPositionChanged = [this](int gen, bool isRight) {
    mSynth.incrementPosition(gen, isRight);
  };
  addAndMakeVisible(mGeneratorsBox);

  /* Global parameter box */
  addAndMakeVisible(mGlobalParamBox);

  /* Arc spectrogram */
  addAndMakeVisible(mArcSpec);

  addChildComponent(mProgressBar);

  addAndMakeVisible(mKeyboard);

  mAudioDeviceManager.initialise(1, 2, nullptr, true, {}, nullptr);

  mAudioDeviceManager.addAudioCallback(&mRecorder);

  // Only want keyboard input focus for standalone as DAW will have own input
  // mappings
  if (mSynth.wrapperType ==
      GranularSynth::WrapperType::wrapperType_Standalone) {
    setWantsKeyboardFocus(true);
  }

  startTimer(50);

  setSize(1200, 565);
}

GRainbowAudioProcessorEditor::~GRainbowAudioProcessorEditor() {
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  auto recordFile = parentDir.getChildFile(Utils::FILE_RECORDING);
  recordFile.deleteFile();
  mAudioDeviceManager.removeAudioCallback(&mRecorder);
  setLookAndFeel(nullptr);
}

void GRainbowAudioProcessorEditor::timerCallback() {
  if (mStartedPlayingTrig && mCurPitchClass != Utils::PitchClass::NONE) {
    mArcSpec.setNoteOn(mCurPitchClass);
    mGeneratorsBox.setPitchClass(mCurPitchClass);
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
    std::vector<CandidateParams*> candidates =
        mSynth.getActiveCandidates();
    // Draw position arrows
    for (int i = 0; i < candidates.size(); ++i) {
      if (candidates[i] == nullptr) continue;
      g.setColour(juce::Colour(Utils::GENERATOR_COLOURS_HEX[i]));
      auto middlePos =
          candidates[i]->posRatio->get() + (candidates[i]->duration->get() / 2.0f);
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

  // Generators box
  auto leftPanel = r.removeFromLeft(PANEL_WIDTH);
  mGeneratorsBox.setBounds(leftPanel);

  auto rightPanel = r.removeFromRight(PANEL_WIDTH);
  mGlobalParamBox.setBounds(rightPanel);

  // Open and record buttons
  auto filePanel = r.removeFromTop(BTN_PANEL_HEIGHT + BTN_PADDING);
  filePanel.removeFromLeft(BTN_PADDING);
  filePanel.removeFromTop(BTN_PADDING);
  mBtnOpenFile.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));
  filePanel.removeFromLeft(BTN_PADDING);
  mBtnRecord.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));

  r.removeFromTop(NOTE_DISPLAY_HEIGHT); // Just padding

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
  parentDir.getChildFile(Utils::FILE_RECORDING).deleteFile();
  mRecordedFile = parentDir.getChildFile(Utils::FILE_RECORDING);

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

/**
 * @brief To properly handle all keyboard input, the main component is used as
 * it will always be "in focus". From here it can pass through and decide what
 * keyboard inputs are sent to each child component
 */
bool GRainbowAudioProcessorEditor::keyStateChanged(bool isKeyDown) {
  if (mSynth.wrapperType ==
      GranularSynth::WrapperType::wrapperType_Standalone) {
    mKeyboard.updateKeyState(nullptr, isKeyDown);
  }
  return false;
}

/**
 * @brief keyPressed is called after keyStateChanged, but know which key was
 * pressed
 */
bool GRainbowAudioProcessorEditor::keyPressed(const juce::KeyPress& key) {
  if (mSynth.wrapperType ==
      GranularSynth::WrapperType::wrapperType_Standalone) {
    mKeyboard.updateKeyState(&key, true);
  }
  return false;
}

/** Fast Debug Mode is used to speed up iterations of testing
    This method should be called only once and no-op if not being used
*/
void GRainbowAudioProcessorEditor::fastDebugMode() {
#ifdef FDB_LOAD_FILE
  // Loads a file right away - make sure macro is in quotes in Projucer
  DBG("Fast Debug Mode - Loading file " << FDB_LOAD_FILE);
  openNewFile(FDB_LOAD_FILE);
#endif  // FDB_LOAD_FILE
}