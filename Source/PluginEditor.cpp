/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "Preset.h"

//==============================================================================
GRainbowAudioProcessorEditor::GRainbowAudioProcessorEditor(GranularSynth& synth)
    : AudioProcessorEditor(&synth),
      mSynth(synth),
      mGlobalParamBox(synth.getParamGlobal()),
      mGeneratorsBox(synth.getParamsNote(), synth.getParamUI()),
      mArcSpec(synth.getParamsNote(), synth.getParamUI()),
      mKeyboard(mSynth.getKeyboardState()),
      mProgressBar(mLoadingProgress) {
  mCurPitchClass = (Utils::PitchClass)synth.getParamUI().pitchClass;

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
                                    ArcSpectrogram::SpecType type) {
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

  // Open file button
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

  // Recording button
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

  // Present button
  juce::Image presetNormal = juce::PNGImageFormat::loadFrom(
      BinaryData::presetNormal_png, BinaryData::presetNormal_pngSize);
  juce::Image presetOver = juce::PNGImageFormat::loadFrom(
      BinaryData::presetOver_png, BinaryData::presetOver_pngSize);
  mBtnPreset.setImages(false, true, true, presetNormal, 1.0f,
                       juce::Colours::transparentBlack, presetOver, 1.0f,
                       juce::Colours::transparentBlack, presetOver, 1.0f,
                       juce::Colours::transparentBlack);
  mBtnPreset.onClick = [this] { savePreset(); };
  addAndMakeVisible(mBtnPreset);
  // don't enable until audio clip is loaded
  mBtnPreset.setEnabled(false);

  // File info label
  mLabelFilenfo.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mLabelFilenfo);

  // Generators box
  mGeneratorsBox.onPositionChanged = [this](int gen, bool isRight) {
    mSynth.incrementPosition(gen, isRight);
  };
  addAndMakeVisible(mGeneratorsBox);

  // Global parameter box
  addAndMakeVisible(mGlobalParamBox);

  // Arc spectrogram
  addAndMakeVisible(mArcSpec);

  addChildComponent(mProgressBar);

  addAndMakeVisible(mKeyboard);

  mAudioDeviceManager.initialise(1, 2, nullptr, true, {}, nullptr);

  mAudioDeviceManager.addAudioCallback(&mRecorder);

  mFormatManager.registerBasicFormats();

  // Only want keyboard input focus for standalone as DAW will have own input
  // mappings
  if (mSynth.wrapperType ==
      GranularSynth::WrapperType::wrapperType_Standalone) {
    setWantsKeyboardFocus(true);
    // Standalone will persist between usages
    mSynth.resetParameters();
  }

  startTimer(50);

  setSize(1200, 565);
}

GRainbowAudioProcessorEditor::~GRainbowAudioProcessorEditor() {
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  auto recordFile = parentDir.getChildFile(ArcSpectrogram::FILE_RECORDING);
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

  // Draw background for preset button
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnPreset.getBounds().toFloat(), 14);
}

void GRainbowAudioProcessorEditor::paintOverChildren(juce::Graphics& g) {
  // Draw note display
  if (mCurPitchClass != Utils::PitchClass::NONE) {
    std::vector<ParamCandidate*> candidates = mSynth.getActiveCandidates();
    // Draw position arrows
    for (int i = 0; i < candidates.size(); ++i) {
      if (candidates[i] == nullptr) continue;
      g.setColour(juce::Colour(Utils::GENERATOR_COLOURS_HEX[i]));
      auto middlePos =
          candidates[i]->posRatio + (candidates[i]->duration / 2.0f);
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
  filePanel.removeFromTop(BTN_PADDING);
  filePanel.removeFromLeft(BTN_PADDING);
  mBtnOpenFile.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));
  filePanel.removeFromLeft(BTN_PADDING);
  mBtnRecord.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));
  // preset button
  filePanel.removeFromRight(BTN_PADDING);
  mBtnPreset.setBounds(filePanel.removeFromRight(OPEN_FILE_WIDTH));
  // no button currntly, but add padding as if one was present
  filePanel.removeFromRight(BTN_PADDING);
  filePanel.removeFromRight(OPEN_FILE_WIDTH);
  // remaining space on sides remaing is for file information
  mLabelFilenfo.setBounds(filePanel.removeFromTop(BTN_PANEL_HEIGHT));

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
                              "*.wav;*.mp3;*.gbow", true);

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
  juce::AudioBuffer<float> fileAudioBuffer;
  double sampleRate;
  bool preset = (file.getFileExtension() == ".gbow");

  if (preset) {
    Preset::Header presetFileHeader;
    juce::FileInputStream input(file);
    if (input.openedOk()) {
      input.read(&presetFileHeader, sizeof(presetFileHeader));
      // TODO - give better warnings
      jassert(presetFileHeader.magic == Preset::MAGIC);
      jassert(presetFileHeader.versionMajor == Preset::VERSION_MAJOR);
      jassert(presetFileHeader.versionMinor == Preset::VERSION_MINOR);

      fileAudioBuffer.setSize(presetFileHeader.audioBufferChannel,
                              presetFileHeader.audioBufferNumberOfSamples);
      input.read(fileAudioBuffer.getWritePointer(0),
                 presetFileHeader.audioBufferSize);
      sampleRate = presetFileHeader.audioBufferSamplerRate;

      // juce::FileInputStream uses 'int' to read
      int xmlSize =
          static_cast<int>(input.getTotalLength() - input.getPosition());
      void* xmlData = malloc(xmlSize);
      jassert(xmlData != nullptr);
      input.read(xmlData, xmlSize);
      mSynth.setPresetParamsXml(xmlData, xmlSize);
      free(xmlData);
    }
  } else {
    // loading audio clip
    std::unique_ptr<juce::AudioFormatReader> reader(
        mFormatManager.createReaderFor(file));
    jassert(reader.get() != nullptr);
    fileAudioBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&fileAudioBuffer, 0, (int)reader->lengthInSamples, 0, true,
                 true);
    sampleRate = reader->sampleRate;
  }

  mSynth.processFile(&fileAudioBuffer, sampleRate, preset);
  mArcSpec.reset();

  mBtnPreset.setEnabled(true);  // if it wasn't already enabled
  mLabelFilenfo.setText(file.getFileName(), juce::dontSendNotification);
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
  parentDir.getChildFile(ArcSpectrogram::FILE_RECORDING).deleteFile();
  mRecordedFile = parentDir.getChildFile(ArcSpectrogram::FILE_RECORDING);

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

void GRainbowAudioProcessorEditor::savePreset() {
  Preset::Header presetFileHeader;
  presetFileHeader.magic = Preset::MAGIC;
  presetFileHeader.versionMajor = Preset::VERSION_MAJOR;
  presetFileHeader.versionMinor = Preset::VERSION_MINOR;

  juce::FileChooser chooser("Save gRainbow presets to a file",
                            juce::File::getCurrentWorkingDirectory(), "*.gbow",
                            true);

  if (chooser.browseForFileToSave(true)) {
    juce::File file = chooser.getResult().withFileExtension("gbow");

    if (file.hasWriteAccess()) {
      const juce::AudioBuffer<float>& audioBuffer = mSynth.getAudioBuffer();
      presetFileHeader.audioBufferSamplerRate = mSynth.getSampleRate();
      presetFileHeader.audioBufferNumberOfSamples = audioBuffer.getNumSamples();
      presetFileHeader.audioBufferChannel = audioBuffer.getNumChannels();
      presetFileHeader.audioBufferSize =
          presetFileHeader.audioBufferNumberOfSamples *
          presetFileHeader.audioBufferChannel * sizeof(float);

      // first write is a 'replace' to clear any file if overriding
      file.replaceWithData(&presetFileHeader, sizeof(presetFileHeader));
      file.appendData(
          reinterpret_cast<const void*>(audioBuffer.getReadPointer(0)),
          presetFileHeader.audioBufferSize);

      // XML structure of preset contains all audio related information
      // These include not just AudioParams but also other params not exposes to
      // the DAW or UI directly
      juce::MemoryBlock xmlMemoryBlock;
      mSynth.getPresetParamsXml(xmlMemoryBlock);
      file.appendData(xmlMemoryBlock.getData(), xmlMemoryBlock.getSize());
    } else {
      // TODO - let users know we can't write here
    }
  }
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