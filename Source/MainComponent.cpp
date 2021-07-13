#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : mKeyboard(mKeyboardState),
      mFft(FFT_SIZE, HOP_SIZE),
      juce::Thread("main fft thread"),
      mProgressBar(mLoadingProgress) {
  mFormatManager.registerBasicFormats();

  setLookAndFeel(&mRainbowLookAndFeel);

  /* Open file button */
  mBtnOpenFile.setButtonText("Open File");
  mBtnOpenFile.onClick = [this] { openNewFile(); };
  addAndMakeVisible(mBtnOpenFile);

  /* Recording button */
  mBtnRecord.setButtonText("Start Recording");
  mBtnRecord.setColour(juce::TextButton::ColourIds::buttonColourId,
                       juce::Colours::green);
  mBtnRecord.onClick = [this] {
    if (mRecorder.isRecording()) {
      stopRecording();
    } else {
      startRecording();
    }
  };
  addAndMakeVisible(mBtnRecord);

  /* Position boxes */
  for (int i = 0; i < mPositionBoxes.size(); ++i) {
    mPositionBoxes[i].setColour((Utils::PositionColour)i);
    mPositionBoxes[i].setActive(i == 0);
    mSynth.updateParameters((Utils::PositionColour)i,
                            mPositionBoxes[i].getParams());
    mPositionBoxes[i].onPositionChanged = [this, i](bool isRight) {
      mPositions[mCurPitchClass][i] =
          findNextPosition(i, isRight);
      for (int box = 0; box < mPositionBoxes.size(); ++box) {
          mPositionBoxes[box].setPositions(
              std::vector<int>(mPositions[mCurPitchClass].begin(),
                               mPositions[mCurPitchClass].end()));
      }
    };
    mPositionBoxes[i].onParameterChanged =
        [this](Utils::PositionColour pos,
               GranularSynth::ParameterType param, float value) {
          if (param == GranularSynth::ParameterType::SOLO) {
            for (int i = 0; i < Utils::PositionColour::NUM_BOXES;
                 ++i) {
              if (i != pos) {
                mPositionBoxes[i].setState(
                    (value == true) ? PositionBox::BoxState::SOLO_WAIT
                                    : PositionBox::BoxState::READY);
              }
            }
          }
          mSynth.updateParameter(pos, param, value);
        };
    addAndMakeVisible(mPositionBoxes[i]);
  }

  /* Arc spectrogram */
  addAndMakeVisible(mArcSpec);
  mArcSpec.onPositionUpdated = [this](int midiNote,
                                      GrainPositionFinder::GrainPosition gPos) {
    mPositionFinder.updatePosition(midiNote, gPos);
  };

  addChildComponent(mProgressBar);

  mTransientDetector.onTransientsUpdated =
      [this](std::vector<TransientDetector::Transient>& transients) {
        mArcSpec.setTransients(&transients);
      };

  mPitchDetector.onPitchesUpdated =
      [this](std::vector<std::vector<float>>& hpcpBuffer, std::vector<std::vector<float>>& notesBuffer) {
        mArcSpec.loadBuffer(&hpcpBuffer, ArcSpectrogram::SpecType::HPCP);
        mArcSpec.loadBuffer(&notesBuffer, ArcSpectrogram::SpecType::NOTES);
        mPositionFinder.setPitches(&mPitchDetector.getPitches());
        mIsProcessingComplete = true;
      };

  mPitchDetector.onProgressUpdated = [this](float progress) {
    mLoadingProgress = progress;
    juce::MessageManagerLock lock;
    if (progress >= 1.0) {
      mProgressBar.setVisible(false);
    } else if (!mProgressBar.isVisible()) {
      mProgressBar.setVisible(true);
    }
  };

  addAndMakeVisible(mKeyboard);

  setSize(1200, 565);

  // Some platforms require permissions to open input channels so request that
  // here
  if (juce::RuntimePermissions::isRequired(
          juce::RuntimePermissions::recordAudio) &&
      !juce::RuntimePermissions::isGranted(
          juce::RuntimePermissions::recordAudio)) {
    juce::RuntimePermissions::request(
        juce::RuntimePermissions::recordAudio,
        [&](bool granted) {
          int numInputChannels = granted ? 1 : 0;
        mAudioDeviceManager.initialise(numInputChannels, 2, nullptr,
                                                   true, {}, nullptr);
        setAudioChannels(numInputChannels, 2);
      });
  } else {
    // Specify the number of input and output channels that we want to open
    mAudioDeviceManager.initialise(2, 2, nullptr, true, {},
                                   nullptr);
    setAudioChannels(2, 2);
  }
  mAudioDeviceManager.addAudioCallback(&mRecorder);

  startTimer(50);  // Keyboard focus timer
}

MainComponent::~MainComponent() {
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  auto recordFile = parentDir.getChildFile(RECORDING_FILE);
  recordFile.deleteFile();
  mAudioDeviceManager.removeAudioCallback(&mRecorder);
  setLookAndFeel(nullptr);
  stopThread(4000);
  // This shuts down the audio device and clears the audio source.
  shutdownAudio();
}

void MainComponent::timerCallback() {
  if (mStartedPlayingTrig && mCurPitchClass != PitchDetector::PitchClass::NONE) {
    std::vector<GrainPositionFinder::GrainPosition> gPositions =
        mPositionFinder.findPositions(PositionBox::MAX_POSITIONS, mCurPitchClass);
    std::vector<GrainPositionFinder::GrainPosition> gPosToPlay;
    for (int i = 0; i < mPositionBoxes.size(); ++i) {
      bool canPlay =
          mPositionBoxes[i].getState() != PositionBox::BoxState::SOLO_WAIT;
      bool shouldPlay =
          mPositionBoxes[i].getActive() ||
          (mPositionBoxes[i].getState() == PositionBox::BoxState::SOLO);
      if (canPlay && shouldPlay && !mPositions.empty() &&
          gPositions.size() >= NUM_BOXES) {
        gPositions[mPositions[mCurPitchClass][i]].isActive = true;
        gPosToPlay.push_back(gPositions[mPositions[mCurPitchClass][i]]);
      } else {
        // Push back an inactive position for the synth
        gPosToPlay.push_back(
            GrainPositionFinder::GrainPosition(PitchDetector::Pitch(), 1.0));
      }
    }

    if (!mPositions.empty()) {
      std::vector<int> boxPositions = std::vector<int>(
          mPositions[mCurPitchClass].begin(), mPositions[mCurPitchClass].end());
      for (int i = 0; i < mPositionBoxes.size(); ++i) {
        mPositionBoxes[i].setPositions(boxPositions);
        mPositionBoxes[i].setNumPositions(gPositions.size());
      }

      mCurPositions = gPositions;
      mSynth.setNoteOn(mCurPitchClass, gPosToPlay);
      mArcSpec.setNoteOn(mCurPitchClass, gPositions, boxPositions);
    }
    mStartedPlayingTrig = false;
    repaint(); // Update note display
  }
}

void MainComponent::run() {
  mFft.processBuffer(mFileBuffer);
  mArcSpec.loadBuffer(&mFft.getSpectrum(), ArcSpectrogram::SpecType::SPECTROGRAM);
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected,
                                  double sampleRate) {
  mSampleRate = sampleRate;
  mArcSpec.setSampleRate(sampleRate);
  mMidiCollector.reset(sampleRate);
}

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill) {
  bufferToFill.clearActiveBufferRegion();

  // fill a midi buffer with incoming messages from the midi input.
  juce::MidiBuffer incomingMidi;
  mMidiCollector.removeNextBlockOfMessages(incomingMidi,
                                           bufferToFill.numSamples);
  mKeyboardState.processNextMidiBuffer(incomingMidi, 0, bufferToFill.numSamples,
                                       true);
  if (!incomingMidi.isEmpty()) {
    for (juce::MidiMessageMetadata md : incomingMidi) {
      if (md.getMessage().isNoteOn()) {
        mCurPitchClass =
            (PitchDetector::PitchClass)md.getMessage().getNoteNumber();
        mStartedPlayingTrig = true;
      } else if (md.getMessage().isNoteOff()) {
        mSynth.setNoteOff(
            (PitchDetector::PitchClass)md.getMessage().getNoteNumber());
        mArcSpec.setNoteOff();
      }
    }
  }

  mSynth.process(bufferToFill.buffer);
}

void MainComponent::releaseResources() {
  // This will be called when the audio device stops, or when it is being
  // restarted due to a setting change.

  // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
}

void MainComponent::paintOverChildren(juce::Graphics& g) {
  // Draw note display
  if (mCurPitchClass != PitchDetector::PitchClass::NONE) {
    // Draw position arrows
    for (int i = 0; i < mCurPositions.size(); ++i) {
      if (mCurPositions[i].isActive) {
        g.setColour(juce::Colour(Utils::POSITION_COLOURS[i]));
        auto middlePos = mCurPositions[i].pitch.posRatio +
                         (mCurPositions[i].pitch.duration / 2.0f);
        float angleRad = (juce::MathConstants<float>::pi * middlePos) -
                         (juce::MathConstants<float>::pi / 2.0f);
        juce::Point<float> startPoint = juce::Point<float>(
            mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
        juce::Point<float> endPoint = startPoint.getPointOnCircumference(
            mArcSpec.getHeight() / 4.5f, angleRad);
        g.drawArrow(juce::Line<float>(startPoint, endPoint), 4.0f, 10.0f, 6.0f);
      }
    }
    // Draw path to positions
    float noteX =
        mKeyboard.getBounds().getX() +
        (mKeyboard.getWidth() * mKeyboard.getPitchXRatio(mCurPitchClass));
    juce::Path displayPath;
    displayPath.startNewSubPath(noteX, mNoteDisplayRect.getBottom());
    displayPath.lineTo(noteX, mNoteDisplayRect.getBottom() - (NOTE_DISPLAY_HEIGHT / 2.0f));
    displayPath.lineTo(mNoteDisplayRect.getCentre());
    displayPath.lineTo(mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
    g.setColour(Utils::getRainbow12Colour(mCurPitchClass));
    g.strokePath(displayPath, juce::PathStrokeType(4.0f));
    g.fillEllipse(mNoteDisplayRect.getCentreX() - (NOTE_BULB_SIZE / 2.0f),
                  mNoteDisplayRect.getY() - (NOTE_BULB_SIZE / 2.0f),
                  NOTE_BULB_SIZE, NOTE_BULB_SIZE);
  }
}

void MainComponent::resized() {
  auto r = getLocalBounds();

  // Position boxes
  auto leftPanel = r.removeFromLeft(PANEL_WIDTH);
  mPositionBoxes[0].setBounds(
      leftPanel.removeFromTop(leftPanel.getHeight() / 2));
  mPositionBoxes[1].setBounds(leftPanel);
  auto rightPanel = r.removeFromRight(PANEL_WIDTH);
  mPositionBoxes[2].setBounds(
      rightPanel.removeFromTop(rightPanel.getHeight() / 2));
  mPositionBoxes[3].setBounds(rightPanel);

  // Open and record buttons
  auto filePanel = r.removeFromTop(KNOB_HEIGHT);
  mBtnOpenFile.setBounds(filePanel.removeFromLeft(filePanel.getWidth() / 2));
  mBtnRecord.setBounds(filePanel);

  r.removeFromTop(NOTE_DISPLAY_HEIGHT); // Just padding
  
  // Arc spectrogram
  mArcSpec.setBounds(r.removeFromTop(r.getWidth() / 2.0f));
  mProgressBar.setBounds(
      mArcSpec.getBounds().withSizeKeepingCentre(PROGRESS_SIZE, PROGRESS_SIZE));

  // Space for note display
  mNoteDisplayRect = r.removeFromTop(NOTE_DISPLAY_HEIGHT).toFloat();

  // Keyboard
  mKeyboard.setBounds(r.removeFromTop(KEYBOARD_HEIGHT));
}

/** Pauses audio to open file
    @param path optional path to load, otherwise will prompt user for file
   location
*/
void MainComponent::openNewFile(const char* path) {
  shutdownAudio();

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

  setAudioChannels(2, 2);
}

void MainComponent::processFile(juce::File file) {
  std::unique_ptr<juce::AudioFormatReader> reader(
      mFormatManager.createReaderFor(file));

  if (reader.get() != nullptr) {
    mFileBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);

    juce::AudioBuffer<float> tempBuffer = juce::AudioBuffer<float>(
        reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&tempBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    juce::ScopedPointer<juce::LagrangeInterpolator> resampler =
        new juce::LagrangeInterpolator();
    double ratio = reader->sampleRate / mSampleRate;
    const float** inputs = tempBuffer.getArrayOfReadPointers();
    float** outputs = mFileBuffer.getArrayOfWritePointers();
    for (int c = 0; c < mFileBuffer.getNumChannels(); c++) {
      resampler->reset();
      resampler->process(ratio, inputs[c], outputs[c],
                         mFileBuffer.getNumSamples());
    }
    resetPositions();
    mArcSpec.resetBuffers();
    stopThread(4000);
    startThread();  // process fft and pass to arc spec
    // mTransientDetector.processBuffer(&mFileBuffer);
    mPitchDetector.processBuffer(&mFileBuffer, mSampleRate);
    mSynth.setFileBuffer(&mFileBuffer, mSampleRate);
    mIsProcessingComplete = false; // Reset processing flag
  }
}

void MainComponent::resetPositions() {
  for (int i = 0; i < mPositions.size(); ++i) {
    for (int j = 0; j < mPositions[i].size(); ++j) {
      mPositions[i][j] = j;
    }
  }
}

void MainComponent::startRecording() {
  if (!juce::RuntimePermissions::isGranted(
          juce::RuntimePermissions::writeExternalStorage)) {
    SafePointer<MainComponent> safeThis(this);

    juce::RuntimePermissions::request(
        juce::RuntimePermissions::writeExternalStorage,
        [safeThis](bool granted) mutable {
          if (granted) safeThis->startRecording();
        });
    return;
  }

  auto parentDir =
      juce::File::getSpecialLocation(juce::File::tempDirectory);
  parentDir.getChildFile(RECORDING_FILE).deleteFile();
  mRecordedFile = parentDir.getChildFile(RECORDING_FILE);

  mRecorder.startRecording(mRecordedFile);

  mBtnRecord.setButtonText("Stop Recording");
  mBtnRecord.setColour(juce::TextButton::ColourIds::buttonColourId,
                       juce::Colours::red);
}

void MainComponent::stopRecording() {
  mRecorder.stop();

  processFile(mRecordedFile);

  mRecordedFile = juce::File();

  mBtnRecord.setButtonText("Start Recording");
  mBtnRecord.setColour(juce::TextButton::ColourIds::buttonColourId,
                       juce::Colours::green);
}

int MainComponent::findNextPosition(int boxNum, bool isRight) {
  int curPos = mPositions[mCurPitchClass][boxNum];
  for (int i = 1; i <= PositionBox::MAX_POSITIONS; ++i) {
    int newPos = isRight ? curPos + i : curPos - i;
    newPos = newPos % PositionBox::MAX_POSITIONS;
    bool isValid = true;
    for (int j = 0; j < mPositions[mCurPitchClass].size(); ++j) {
      if (mPositions[mCurPitchClass][j] == newPos && boxNum != j) {
        // Position already taken, move on
        isValid = false;
        break;
      }
    }
    if (isValid) return newPos;
  }
  return curPos;
}

/** Fast Debug Mode is used to speed up iterations of testing
    This method should be called only once and no-op if not being used
*/
void MainComponent::fastDebugMode() {
#ifdef FDB_LOAD_FILE
  // Loads a file right away - make sure macro is in quotes in Projucer
  DBG("Fast Debug Mode - Loading file " << FDB_LOAD_FILE);
  openNewFile(FDB_LOAD_FILE);
#endif  // FDB_LOAD_FILE
}