#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : mKeyboard(mKeyboardState,
                juce::MidiKeyboardComponent::horizontalKeyboard),
      mSynth(mKeyboardState),
      mForwardFFT(FFT_ORDER) {
  mFormatManager.registerBasicFormats();

  setLookAndFeel(&mRainbowLookAndFeel);

  juce::Image logo = juce::PNGImageFormat::loadFrom(juce::File(
      "C:/Users/brady/Documents/GitHub/gRainbow/gRainbow-circles.png"));

  /* Title section */
  mLogo.setImage(logo, juce::RectanglePlacement::centred);
  addAndMakeVisible(mLogo);

  mBtnOpenFile.setButtonText("Open File");
  mBtnOpenFile.onClick = [this] { openNewFile(); };
  addAndMakeVisible(mBtnOpenFile);

  /* Knobs */

  /* Diversity */
  mSliderDiversity.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDiversity.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDiversity.setRange(0.0, 1.0, 0.01);
  mSliderDiversity.onValueChange = [this] {
    mSynth.setDiversity(mSliderDiversity.getValue());
  };
  mSliderDiversity.setValue(PARAM_DIVERSITY_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderDiversity);

  mLabelDiversity.setText("Diversity", juce::dontSendNotification);
  mLabelDiversity.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDiversity);

  /* Duration */
  mSliderDuration.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDuration.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDuration.setRange(0.0, 1.0, 0.01);
  mSliderDuration.onValueChange = [this] {
    mSynth.setDuration(mSliderDuration.getValue());
  };
  mSliderDuration.setValue(PARAM_DIVERSITY_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderDuration);

  mLabelDuration.setText("Duration", juce::dontSendNotification);
  mLabelDuration.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDuration);

  /* Rate */
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRate.setRange(0.0, 1.0, 0.01);
  mSliderRate.onValueChange = [this] {
    mSynth.setRate(mSliderRate.getValue());
  };
  mSliderRate.setValue(PARAM_RATE_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderRate);

  mLabelRate.setText("Rate", juce::dontSendNotification);
  mLabelRate.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRate);

  addAndMakeVisible(mArcSpec);

  mKeyboard.setAvailableRange(43, 79);
  addAndMakeVisible(mKeyboard);

  setSize(1200, 600);

  // Some platforms require permissions to open input channels so request that
  // here
  if (juce::RuntimePermissions::isRequired(
          juce::RuntimePermissions::recordAudio) &&
      !juce::RuntimePermissions::isGranted(
          juce::RuntimePermissions::recordAudio)) {
    juce::RuntimePermissions::request(
        juce::RuntimePermissions::recordAudio,
        [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
  } else {
    // Specify the number of input and output channels that we want to open
    setAudioChannels(2, 2);
  }

  startTimer(400);  // Keyboard focus timer
}

MainComponent::~MainComponent() {
  setLookAndFeel(nullptr);
  // This shuts down the audio device and clears the audio source.
  shutdownAudio();
}

void MainComponent::timerCallback() {
  mKeyboard.grabKeyboardFocus();
  stopTimer();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected,
                                  double sampleRate) {
  mMidiCollector.reset(sampleRate);
  // mTimeStretcher = std::make_unique<RubberBand::RubberBandStretcher>
  //  (sampleRate, 1,
  //  RubberBand::RubberBandStretcher::DefaultOptions, 1.0, 1.0);
  // mTimeStretcher->reset();
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
        std::vector<GranularSynth::GrainPosition> positionRatios =
            mSynth.playNote(md.getMessage().getNoteNumber());
        mArcSpec.updatePositions(positionRatios);
      } else if (md.getMessage().isNoteOff()) {
        mSynth.stopNote(md.getMessage().getNoteNumber());
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

  auto titleSection = r.removeFromTop(LOGO_HEIGHT);
  mLogo.setBounds(titleSection.withSizeKeepingCentre(LOGO_HEIGHT * 2,
                                                     titleSection.getHeight()));
  mBtnOpenFile.setBounds(
      titleSection.removeFromLeft(titleSection.getWidth() / 4));
  mKeyboard.setBounds(
      r.removeFromBottom(KEYBOARD_HEIGHT)
          .withSizeKeepingCentre(mKeyboard.getTotalKeyboardWidth(),
                                 KEYBOARD_HEIGHT));
  mArcSpec.setBounds(
      r.withWidth(r.getHeight() * 2)
          .withCentre(r.getPosition() +
                      juce::Point<int>(r.getWidth() / 2, r.getHeight() / 2)));
  // Left Panel
  auto leftPanel = r.removeFromLeft((r.getWidth() - mArcSpec.getWidth()) / 2);
  // Row 1
  auto row = leftPanel.removeFromTop(KNOB_HEIGHT + LABEL_HEIGHT);
  // Diversity
  auto knob = row.removeFromLeft(row.getWidth() / 2);
  mSliderDiversity.setBounds(
      knob.withSize(KNOB_HEIGHT * 2, KNOB_HEIGHT)
          .withCentre(knob.getPosition().translated(
              knob.getWidth() / 2, (knob.getHeight() - LABEL_HEIGHT) / 2)));
  mLabelDiversity.setBounds(knob.removeFromBottom(LABEL_HEIGHT));
  // Duration
  knob = row;
  mSliderDuration.setBounds(
      knob.withSize(KNOB_HEIGHT * 2, KNOB_HEIGHT)
          .withCentre(knob.getPosition().translated(
              knob.getWidth() / 2, (knob.getHeight() - LABEL_HEIGHT) / 2)));
  mLabelDuration.setBounds(knob.removeFromBottom(LABEL_HEIGHT));
  // Row 2
  row = leftPanel.removeFromTop(KNOB_HEIGHT + LABEL_HEIGHT);
  // Rate
  knob = row.removeFromLeft(row.getWidth() / 2);
  mSliderRate.setBounds(
      knob.withSize(KNOB_HEIGHT * 2, KNOB_HEIGHT)
          .withCentre(knob.getPosition().translated(
              knob.getWidth() / 2, (knob.getHeight() - LABEL_HEIGHT) / 2)));
  mLabelRate.setBounds(knob.removeFromBottom(LABEL_HEIGHT));
  // auto rightPanel = r.removeFromRight(r.getWidth() / 4);
}

void MainComponent::openNewFile() {
  shutdownAudio();

  juce::FileChooser chooser("Select a file to granulize...",
                            juce::File("C:/users/brady/Music"), "*.wav;*.mp3",
                            false);

  if (chooser.browseForFileToOpen()) {
    auto file = chooser.getResult();
    std::unique_ptr<juce::AudioFormatReader> reader(
        mFormatManager.createReaderFor(file));

    if (reader.get() != nullptr) {
      auto duration = (float)reader->lengthInSamples / reader->sampleRate;
      mFileBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
      reader->read(&mFileBuffer, 0, (int)reader->lengthInSamples, 0, true,
                   true);

      updateFft(reader->sampleRate);
    }
  }
  setAudioChannels(2, 2);
}

void MainComponent::updateFft(double sampleRate) {
  const float* pBuffer = mFileBuffer.getReadPointer(0);
  int curSample = 0;

  bool hasData = mFileBuffer.getNumSamples() > mFftFrame.size();

  mFftData.clear();

  while (hasData) {
    const float* startSample = &pBuffer[curSample];
    int numSamples = mFftFrame.size();
    if (curSample + mFftFrame.size() > mFileBuffer.getNumSamples()) {
      numSamples = (mFileBuffer.getNumSamples() - curSample);
    }
    mFftFrame.fill(0.0f);
    memcpy(mFftFrame.data(), startSample, numSamples);

    // then render our FFT data..
    mForwardFFT.performFrequencyOnlyForwardTransform(mFftFrame.data());

    // Add fft data to our master array
    mFftData.push_back(std::vector<float>(mFftFrame.begin(), mFftFrame.end()));

    curSample += mFftFrame.size();
    if (curSample > mFileBuffer.getNumSamples()) hasData = false;
  }
  updateFftRanges();
  mArcSpec.updateSpectrogram(&mFftData, &mFftRanges);
  mSynth.setFileBuffer(&mFileBuffer, &mFftData, &mFftRanges, sampleRate);
}

void MainComponent::updateFftRanges() {
  if (mFftData.empty()) return;
  mFftRanges.frameRanges.clear();
  float totalMin = std::numeric_limits<float>::max();
  float totalMax = std::numeric_limits<float>::min();
  for (auto i = 0; i < mFftData.size(); ++i) {
    float curMin = std::numeric_limits<float>::max();
    float curMax = std::numeric_limits<float>::min();
    for (auto j = 0; j < mFftData[i].size(); ++j) {
      auto val = mFftData[i][j];
      if (val < curMin) {
        if (val < totalMin) {
          totalMin = val;
        }
        curMin = val;
      }
      if (val > curMax) {
        if (val > totalMax) {
          totalMax = val;
        }
        curMax = val;
      }
    }
    mFftRanges.frameRanges.push_back(juce::Range<float>(curMin, curMax));
  }
  mFftRanges.globalRange.setStart(totalMin);
  mFftRanges.globalRange.setEnd(totalMax);
}