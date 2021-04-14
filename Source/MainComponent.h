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
  
  static constexpr auto FFT_ORDER = 10;
  static constexpr auto FFT_SIZE = 1 << FFT_ORDER;
  static constexpr auto KNOB_HEIGHT = 50;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto KEYBOARD_HEIGHT = 100;
  static constexpr auto LOGO_HEIGHT = 150;

  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::AudioFormatManager mFormatManager;
  juce::MidiMessageCollector mMidiCollector;
  juce::AudioBuffer<float> mFileBuffer;
  GranularSynth mSynth;

  /* Global fft */
  juce::dsp::FFT mForwardFFT;
  std::array<float, FFT_SIZE * 2> mFftFrame;
  std::vector<std::vector<float>> mFftData;
  void updateFft(double sampleRate);

  /* UI Components */
  juce::ImageComponent mLogo;
  juce::TextButton mBtnOpenFile;
  juce::Slider mSliderDiversity;
  juce::Label mLabelDiversity;
  juce::Slider mSliderDuration;
  juce::Label mLabelDuration;
  ArcSpectrogram mArcSpec;
  juce::MidiKeyboardState mKeyboardState;
  juce::MidiKeyboardComponent mKeyboard;
  
  void openNewFile();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
