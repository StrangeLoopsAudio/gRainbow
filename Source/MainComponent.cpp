#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent():
  mKeyboard(mKeyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
  mSynth(mKeyboardState),
  mForwardFFT(mFftOrder)
{
  mFormatManager.registerBasicFormats();

  setLookAndFeel(&mRainbowLookAndFeel);

  mBtnOpenFile.setButtonText("Open File");
  mBtnOpenFile.onClick = [this] { openNewFile(); };
  addAndMakeVisible(mBtnOpenFile);

  mSliderPosition.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderPosition.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderPosition.setRange(0.0, 1.0, 0.01);
  //mSliderPosition.onValueChange = [this] { mArcSpec.changePosition(mSliderPosition.getValue()); };
  addAndMakeVisible(mSliderPosition);

  mLabelPosition.setText("Position", juce::dontSendNotification);
  mLabelPosition.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mLabelPosition);

  addAndMakeVisible(mArcSpec);

  addAndMakeVisible(mKeyboard);

  setSize(1200, 600);

  // Some platforms require permissions to open input channels so request that here
  if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
    && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
  {
    juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
      [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
  }
  else
  {
    // Specify the number of input and output channels that we want to open
    setAudioChannels(2, 2);
  }

  startTimer(400); // Keyboard focus timer
}

MainComponent::~MainComponent()
{
  setLookAndFeel(nullptr);
  // This shuts down the audio device and clears the audio source.
  shutdownAudio();
}

void MainComponent::timerCallback()
{
  mKeyboard.grabKeyboardFocus();
  stopTimer();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
  mMidiCollector.reset(sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
  bufferToFill.clearActiveBufferRegion();

  // fill a midi buffer with incoming messages from the midi input.
  juce::MidiBuffer incomingMidi;
  mMidiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
  mKeyboardState.processNextMidiBuffer(incomingMidi, 0, bufferToFill.numSamples, true);
  if (!incomingMidi.isEmpty())
  {
    for (juce::MidiMessageMetadata md : incomingMidi)
    {
      if (md.getMessage().isNoteOn())
      {
        std::vector<int> positions = mSynth.playNote(md.getMessage().getNoteNumber(), 2);
        std::vector<float> positionRatios;
        for (int pos : positions)
        {
          positionRatios.push_back((float)pos / mFftData.size());
        }
        mArcSpec.updatePositions(positionRatios);
      }
    }
  }

  mSynth.process(bufferToFill.buffer);
}

void MainComponent::releaseResources()
{
  // This will be called when the audio device stops, or when it is being
  // restarted due to a setting change.

  // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
  // (Our component is opaque, so we must completely fill the background with a solid colour)
  g.fillAll(juce::Colours::black);
}

void MainComponent::resized()
{
  auto r = getLocalBounds();
  
  mBtnOpenFile.setBounds(r.removeFromTop(40));
/*  auto testSliderRect = r.removeFromTop(60).withWidth(80);
  mSliderPosition.setBounds(testSliderRect.removeFromTop(40));
  mLabelPosition.setBounds(testSliderRect); */
  mKeyboard.setBounds(r.removeFromBottom(150));
  mArcSpec.setBounds(r.withWidth(r.getWidth() / 2).withCentre(r.getPosition() + juce::Point<int>(r.getWidth() / 2, r.getHeight() / 2)));
}

void MainComponent::openNewFile()
{
  shutdownAudio();

  juce::FileChooser chooser("Select a file to granulize...",
    juce::File("C:/users/brady/Music"),
    "*.wav;*.mp3",
    false);

  if (chooser.browseForFileToOpen())
  {
    auto file = chooser.getResult();
    std::unique_ptr<juce::AudioFormatReader> reader(mFormatManager.createReaderFor(file));

    if (reader.get() != nullptr)
    {
      auto duration = (float)reader->lengthInSamples / reader->sampleRate;
      mFileBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
      reader->read(&mFileBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
      setAudioChannels(reader->numChannels, reader->numChannels);
      
      updateFft(reader->sampleRate);
    }
  }
}

void MainComponent::updateFft(double sampleRate)
{
  const float* pBuffer = mFileBuffer.getReadPointer(0);
  int curSample = 0;

  bool hasData = mFileBuffer.getNumSamples() > mFftFrame.size();

  mFftData.clear();

  while (hasData)
  {
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
  mArcSpec.updateSpectrogram(&mFftData);
  mSynth.setSampleRate(sampleRate);
  mSynth.setFileBuffer(&mFileBuffer, &mFftData);
}