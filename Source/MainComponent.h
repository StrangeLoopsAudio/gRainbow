#pragma once

#include <JuceHeader.h>
#include "ArcSpectrogram.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent
{
public:
  //==============================================================================
  MainComponent();
  ~MainComponent() override;

  //==============================================================================
  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

  //==============================================================================
  void paint(juce::Graphics& g) override;
  void resized() override;

private:

  void openNewFile();

  juce::TextButton mBtnOpenFile;
  ArcSpectrogram mArcSpec;
  juce::AudioFormatManager mFormatManager;
  juce::AudioSampleBuffer mFileBuffer;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
