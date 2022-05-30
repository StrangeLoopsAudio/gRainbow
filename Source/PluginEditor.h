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
#include "GeneratorsBox.h"
#include "GlobalParamBox.h"
#include "NoteGrid.h"
#include "RainbowKeyboard.h"
#include "RainbowLookAndFeel.h"
#include "TransientDetector.h"
#include "Utils.h"

//==============================================================================
/**
 */
class GRainbowAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     juce::FileDragAndDropTarget,
                                     juce::Timer {
 public:
  GRainbowAudioProcessorEditor(GranularSynth& synth);
  ~GRainbowAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void paintOverChildren(juce::Graphics& g) override;
  void resized() override;

  bool isInterestedInFileDrag(const juce::StringArray& files) override;
  void fileDragEnter(const juce::StringArray& files, int x, int y) override;
  void fileDragExit(const juce::StringArray& files) override;
  void filesDropped(const juce::StringArray& files, int x, int y) override;

  void timerCallback() override;

  void fastDebugMode();

 private:
  // UI Layout
  static constexpr auto BTN_PANEL_HEIGHT = 50;
  static constexpr auto BTN_PADDING = 5;
  static constexpr auto OPEN_FILE_WIDTH = 80;
  static constexpr auto PANEL_WIDTH = 300;
  static constexpr auto KNOB_HEIGHT = 50;
  static constexpr auto TABS_HEIGHT = 30;
  static constexpr auto PROGRESS_SIZE = 80;
  static constexpr auto NOTE_BULB_SIZE = 10;
  static constexpr auto NOTE_DISPLAY_HEIGHT = 20;
  static constexpr auto FILE_RECORDING = "gRainbow_user_recording.wav";

  // DSP Modules
  GranularSynth& mSynth;
  AudioRecorder mRecorder;

  // UI Components
  juce::ImageComponent mLogo;
  juce::ImageButton mBtnOpenFile;
  juce::ImageButton mBtnRecord;
  juce::ImageButton mBtnPreset;
  juce::Label mLabelFileName;
  ArcSpectrogram mArcSpec;
  RainbowKeyboard mKeyboard;
  juce::ProgressBar mProgressBar;
  GlobalParamBox mGlobalParamBox;
  NoteGrid mNoteGrid;
  GeneratorsBox mGeneratorsBox;
  juce::Rectangle<float> mNoteDisplayRect;
  juce::SharedResourcePointer<juce::TooltipWindow> mTooltipWindow;

  // Synth owns, but need to grab params on reloading of plugin
  ParamUI& mParamUI;

  // Bookkeeping
  juce::File mRecordedFile;
  juce::AudioDeviceManager mAudioDeviceManager;
  bool mIsFileHovering = false;
  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::AudioFormatManager mFormatManager;

  void openNewFile(const char* path = nullptr);
  void processFile(juce::File file);
  void startRecording();
  void stopRecording();
  void savePreset();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GRainbowAudioProcessorEditor)
};
