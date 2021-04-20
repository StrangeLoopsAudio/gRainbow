#pragma once

#include <JuceHeader.h>

#include "ArcSpectrogram.h"
#include "GranularSynth.h"
#include "RainbowLookAndFeel.h"
#include "Utils.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent, juce::Timer {
 public:
  //==============================================================================
  MainComponent();
  ~MainComponent() override;

  //==============================================================================
  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock(
      const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

  //==============================================================================
  void paint(juce::Graphics& g) override;
  void resized() override;

  //==============================================================================
  void timerCallback() override;

 private:
  /* Algorithm Constants */
  static constexpr auto FFT_ORDER = 11;
  static constexpr auto FFT_SIZE = 1 << FFT_ORDER;
  static constexpr auto MIN_MIDINOTE = 43;
  static constexpr auto MAX_MIDINOTE = 91;
  static constexpr auto NUM_HPS_HARMONICS = 3;

  /* UI Layout */
  static constexpr auto KNOB_HEIGHT = 50;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto KEYBOARD_HEIGHT = 100;
  static constexpr auto LOGO_HEIGHT = 150;

  /* Parameter defaults */
  static constexpr auto PARAM_DIVERSITY_DEFAULT = 0.1f;
  static constexpr auto PARAM_DURATION_DEFAULT = 0.2f;
  static constexpr auto PARAM_RATE_DEFAULT = 0.3f;

  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::AudioFormatManager mFormatManager;
  juce::MidiMessageCollector mMidiCollector;
  juce::AudioBuffer<float> mFileBuffer;
  GranularSynth mSynth;

  /* Global fft */
  juce::dsp::FFT mForwardFFT;
  std::array<float, FFT_SIZE * 2> mFftFrame;
  std::vector<std::vector<float>> mFftData;
  std::vector<std::vector<float>> mHpsData;
  std::vector<Utils::HpsPitch> mHpsPitches;
  Utils::SpecRanges mHpsRanges;
  Utils::SpecRanges mFftRanges;
  void updateFft(double sampleRate);
  void updateFftRanges();
  void updateHpsData(double sampleRate);
  void updateEstimatedPitches();

  /* UI Components */
  juce::ImageComponent mLogo;
  juce::TextButton mBtnOpenFile;
  ArcSpectrogram mArcSpec;
  juce::MidiKeyboardState mKeyboardState;
  juce::MidiKeyboardComponent mKeyboard;
  /* Parameters */
  juce::Slider mSliderDiversity;
  juce::Label mLabelDiversity;
  juce::Slider mSliderDuration;
  juce::Label mLabelDuration;
  juce::Slider mSliderRate;
  juce::Label mLabelRate;

  void openNewFile();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
