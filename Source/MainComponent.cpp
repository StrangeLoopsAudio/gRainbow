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
    mPositionBoxes[i].setColour((GranularSynth::PositionColour)i);
    if (i == 0) {
      mPositionBoxes[i].setActive(true);
    }
    mPositionBoxes[i].onParameterChanged =
          [this](GranularSynth::PositionColour pos,
               GranularSynth::ParameterType param, float value) {
          mSynth.updatePositionSettings(pos, param, value);
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

  /* Synth */
  mSynth.updatePositionSettings(GranularSynth::PositionColour::BLUE,
                                GranularSynth::ParameterType::ENABLED, true);

  setSize(1200, 600);

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
  if (mCurPitchClass != PitchDetector::PitchClass::NONE) {
    std::vector<GrainPositionFinder::GrainPosition> gPositions =
        mPositionFinder.findPositions(NUM_POSITIONS, mCurPitchClass);
    for (int i = 0; i < gPositions.size(); ++i) {
      gPositions[i].isActive = mPositionBoxes[i].getActive();
    }
    mSynth.setPositions(mCurPitchClass, gPositions);
    mArcSpec.setNoteOn(mCurPitchClass, gPositions);
    mCurPitchClass = PitchDetector::PitchClass::NONE;
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
      if (md.getMessage().isNoteOn() && mIsProcessingComplete) {
        mCurPitchClass =
            (PitchDetector::PitchClass)md.getMessage().getNoteNumber();
      } else if (md.getMessage().isNoteOff() && mIsProcessingComplete) {
        mSynth.stopNote(
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
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(juce::Colours::black);
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

  auto filePanel = r.removeFromTop(KNOB_HEIGHT);
  mBtnOpenFile.setBounds(filePanel.removeFromLeft(filePanel.getWidth() / 2));
  mBtnRecord.setBounds(filePanel);

  mKeyboard.setBounds(r.removeFromBottom(KEYBOARD_HEIGHT)
                          .withSizeKeepingCentre(PANEL_WIDTH, KEYBOARD_HEIGHT));

  mArcSpec.setBounds(r.removeFromBottom(r.getWidth() / 2.0f));
  mProgressBar.setBounds(
      mArcSpec.getBounds().withSizeKeepingCentre(PROGRESS_SIZE, PROGRESS_SIZE));
}

void MainComponent::openNewFile() {
  shutdownAudio();

  juce::FileChooser chooser("Select a file to granulize...",
                            juce::File::getCurrentWorkingDirectory(), "*.wav;*.mp3",
                            true);

  if (chooser.browseForFileToOpen()) {
    auto file = chooser.getResult();
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
    mArcSpec.resetBuffers();
    stopThread(4000);
    startThread();  // process fft and pass to arc spec
    // mTransientDetector.processBuffer(&mFileBuffer);
    mPitchDetector.processBuffer(&mFileBuffer, mSampleRate);
    mSynth.setFileBuffer(&mFileBuffer, mSampleRate);
    mIsProcessingComplete = false; // Reset processing flag
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