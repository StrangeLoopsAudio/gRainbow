#pragma once

#include <JuceHeader.h>

#include "ArcSpectrogram.h"
#include "RainbowKeyboard.h"
#include "GranularSynth.h"
#include "RainbowLookAndFeel.h"
#include "Utils.h"
#include "TransientDetector.h"
#include "PitchDetector.h"
#include "Fft.h"
#include "AudioRecorder.h"
#include "GrainPositionFinder.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent, juce::Timer, juce::Thread {
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

  void run() override;

  //==============================================================================
  void timerCallback() override;

 private:
  /* Algorithm Constants */
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 2048;
  static constexpr auto RECORDING_FILE = "gRainbow_user_recording.wav";

  /* UI Layout */
  static constexpr auto PANEL_WIDTH = 300;
  static constexpr auto ROW_PADDING_HEIGHT = 10;
  static constexpr auto KNOB_HEIGHT = 50;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto SPEC_WIDTH = 20;
  static constexpr auto SPEC_HEIGHT = 20;
  static constexpr auto PROGRESS_SIZE = 80;
  static constexpr auto KEYBOARD_HEIGHT = 100;
  static constexpr auto MIN_NOTE_NUM = 45;
  static constexpr auto MAX_NOTE_NUM = 56;

  /* Parameter defaults */
  static constexpr auto PARAM_DIVERSITY_DEFAULT = 0.1f;
  static constexpr auto MIN_DIVERSITY = 1.f;
  static constexpr auto MAX_DIVERSITY = 5.f;
  static constexpr auto PARAM_DURATION_DEFAULT = 0.5f;
  static constexpr auto PARAM_RATE_DEFAULT = 0.5f;

  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::AudioFormatManager mFormatManager;
  juce::MidiMessageCollector mMidiCollector;
  double mSampleRate;
  PitchDetector::PitchClass mCurPitchClass = PitchDetector::PitchClass::NONE;
  Fft mFft;
  juce::AudioBuffer<float> mFileBuffer;
  double mLoadingProgress = 0.0;
  bool mIsProcessingComplete = false;

  /* DSP Modules */
  TransientDetector mTransientDetector;
  PitchDetector mPitchDetector;
  GrainPositionFinder mPositionFinder;
  GranularSynth mSynth;
  AudioRecorder mRecorder;

  /* UI Components */
  juce::ImageComponent mLogo;
  juce::TextButton mBtnOpenFile;
  juce::TextButton mBtnRecord;
  ArcSpectrogram mArcSpec;
  RainbowKeyboard mKeyboard;
  juce::ProgressBar mProgressBar;
  /* Bookkeeping */
  juce::MidiKeyboardState mKeyboardState;
  juce::File mRecordedFile;
  juce::AudioDeviceManager mAudioDeviceManager;
  /* Parameters */
  juce::Slider mSliderDiversity;
  juce::Label mLabelDiversity;
  juce::Slider mSliderRate;
  juce::Label mLabelRate;
  juce::Slider mSliderDuration;
  juce::Label mLabelDuration;

  void openNewFile();
  void processFile(juce::File &file);
  void startRecording();
  void stopRecording();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
