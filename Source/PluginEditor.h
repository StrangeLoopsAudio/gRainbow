/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "ArcSpectrogram.h"
#include "AudioRecorder.h"
#include "Fft.h"
#include "GrainPositionFinder.h"
#include "PitchDetector.h"
#include "PluginProcessor.h"
#include "PositionBox.h"
#include "PositionTabs.h"
#include "RainbowKeyboard.h"
#include "RainbowLookAndFeel.h"
#include "TransientDetector.h"
#include "Utils.h"

//==============================================================================
/**
 */
class GRainbowAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     public juce::Timer,
                                     public juce::Thread {
 public:
  GRainbowAudioProcessorEditor(GRainbowAudioProcessor&);
  ~GRainbowAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void paintOverChildren(juce::Graphics& g) override;
  void resized() override;

  void timerCallback() override;
  void run() override;

 private:
  /* Algorithm Constants */
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 2048;
  static constexpr auto RECORDING_FILE = "gRainbow_user_recording.wav";

  /* UI Layout */
  static constexpr auto BTN_PANEL_HEIGHT = 50;
  static constexpr auto BTN_PADDING = 5;
  static constexpr auto OPEN_FILE_WIDTH = 80;
  static constexpr auto PANEL_WIDTH = 300;
  static constexpr auto KNOB_HEIGHT = 50;
  static constexpr auto TABS_HEIGHT = 30;
  static constexpr auto PROGRESS_SIZE = 80;
  static constexpr auto NOTE_BULB_SIZE = 10;
  static constexpr auto NOTE_DISPLAY_HEIGHT = 20;

  /* DSP Modules */
  GRainbowAudioProcessor& mProcessor;
  TransientDetector mTransientDetector;
  PitchDetector mPitchDetector;
  AudioRecorder mRecorder;

  /* UI Components */
  juce::ImageComponent mLogo;
  juce::ImageButton mBtnOpenFile;
  juce::ImageButton mBtnRecord;
  ArcSpectrogram mArcSpec;
  RainbowKeyboard mKeyboard;
  juce::ProgressBar mProgressBar;
  PositionTabs mPositionTabs;
  std::array<PositionBox, Utils::GeneratorColour::NUM_GEN> mPositionBoxes;
  juce::Rectangle<float> mNoteDisplayRect;

  /* Bookkeeping */
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::NONE;
  juce::File mRecordedFile;
  juce::AudioDeviceManager mAudioDeviceManager;
  int mCurPositionTab = 0;
  double mLoadingProgress = 0.0;
  bool mIsProcessingComplete = false;
  bool mStartedPlayingTrig = false;
  Fft mFft;
  juce::AudioBuffer<float> mFileBuffer;
  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::AudioFormatManager mFormatManager;

  void openNewFile(const char* path = nullptr);
  void processFile(juce::File file);
  void startRecording();
  void stopRecording();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GRainbowAudioProcessorEditor)
};
