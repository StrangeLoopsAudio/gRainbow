#pragma once

#include <JuceHeader.h>
#include "GranularSynth.h"
#include "ArcSpectrogram.h"
#include "RainbowLookAndFeel.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent, juce::Timer
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

  //==============================================================================
  void timerCallback() override;

private:
  void openNewFile();

  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::AudioFormatManager mFormatManager;
  juce::MidiMessageCollector mMidiCollector;
  juce::AudioBuffer<float> mFileBuffer;
  GranularSynth mSynth;

  /* Global fft */
  static constexpr auto mFftOrder = 10;
  static constexpr auto mFftSize = 1 << mFftOrder;
  juce::dsp::FFT mForwardFFT;
  std::array<float, mFftSize * 2> mFftFrame;
  std::vector<std::vector<float>> mFftData;
  void updateFft(double sampleRate);

  /* UI Components */
  juce::TextButton mBtnOpenFile;
  juce::TextButton mBtnPlay;
  juce::TextButton mBtnStop;
  juce::Slider mSliderPosition;
  juce::Label mLabelPosition;
  ArcSpectrogram mArcSpec;
  juce::MidiKeyboardState mKeyboardState;
  juce::MidiKeyboardComponent mKeyboard;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
