#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
  mFormatManager.registerBasicFormats();

  setLookAndFeel(&mRainbowLookAndFeel);

  mBtnOpenFile.setButtonText("Open File");
  mBtnOpenFile.onClick = [this] { openNewFile(); };
  addAndMakeVisible(mBtnOpenFile);

  mSliderTest.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderTest.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  addAndMakeVisible(mSliderTest);

  addAndMakeVisible(mArcSpec);

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
}

MainComponent::~MainComponent()
{
  setLookAndFeel(nullptr);
  // This shuts down the audio device and clears the audio source.
  shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
  // This function will be called when the audio device is started, or when
  // its settings (i.e. sample rate, block size, etc) are changed.

  // You can use this function to initialise any resources you might need,
  // but be careful - it will be called on the audio thread, not the GUI thread.

  // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
  // Your audio-processing code goes here!

  // For more details, see the help for AudioProcessor::getNextAudioBlock()

  // Right now we are not producing any data, in which case we need to clear the buffer
  // (to prevent the output of random noise)
  bufferToFill.clearActiveBufferRegion();
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
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  // You can add your drawing code here!
}

void MainComponent::resized()
{
  auto r = getLocalBounds();
  
  mBtnOpenFile.setBounds(r.removeFromTop(40));
  mSliderTest.setBounds(r.removeFromTop(40).withWidth(80));
  mArcSpec.setBounds(r.withSize(600, 300));
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
      mArcSpec.loadedBuffer(&mFileBuffer);
    }
  }
}