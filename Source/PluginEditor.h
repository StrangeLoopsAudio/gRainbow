/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include "Components/ArcSpectrogram.h"
#include "Components/GeneratorsBox.h"
#include "Components/GlobalParamBox.h"
#include "Components/RainbowKeyboard.h"
#include "Components/TrimSelection.h"
#include "Components/Settings.h"
#include "DSP/AudioRecorder.h"
#include "DSP/Fft.h"
#include "DSP/TransientDetector.h"
#include "DSP/GranularSynth.h"
#include "RainbowLookAndFeel.h"
#include "Utils.h"


/**
 * @brief Used on startup to fill unused area with the logo
 */
class GRainbowLogo : public juce::Component {
 public:
  GRainbowLogo();
  ~GRainbowLogo(){};
  void paint(juce::Graphics& g) override;
  void resized() override{};

 private:
  juce::Image mLogoImage;
};

//==============================================================================
/**
 * @brief The "Main component" that parents all UI elements.
 */
class GRainbowAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     juce::FileDragAndDropTarget,
                                     juce::Timer {
 public:
  GRainbowAudioProcessorEditor(GranularSynth& synth);
  ~GRainbowAudioProcessorEditor() override;

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
  static constexpr int BTN_PANEL_HEIGHT = 50;
  static constexpr int BTN_PADDING = 5;
  static constexpr int OPEN_FILE_WIDTH = 80;
  static constexpr int PANEL_WIDTH = 300;
  static constexpr int KNOB_HEIGHT = 50;
  static constexpr int TABS_HEIGHT = 30;
  static constexpr int PROGRESS_SIZE = 80;
  static constexpr int NOTE_BULB_SIZE = 10;
  static constexpr int NOTE_DISPLAY_HEIGHT = 20;
  static constexpr float KEYBOARD_HEIGHT = 0.25f;
  static constexpr auto FILE_RECORDING = "gRainbow_user_recording.wav";

  // DSP Modules
  GranularSynth& mSynth;
  AudioRecorder mRecorder;

  // UI Components
  juce::ImageButton mBtnOpenFile;
  juce::ImageButton mBtnRecord;
  juce::ImageButton mBtnPreset;
  juce::Label mLabelFileName;
  RainbowKeyboard mKeyboard;
  GlobalParamBox mGlobalParamBox;
  GeneratorsBox mGeneratorsBox;
  juce::Rectangle<float> mNoteDisplayRect;
  juce::SharedResourcePointer<juce::TooltipWindow> mTooltipWindow;
  SettingsComponent mSettings;
  juce::Label mResourceUsage;

  // main center UI component
  GRainbowLogo mLogo;
  ArcSpectrogram mArcSpec;
  TrimSelection mTrimSelection;
  juce::ProgressBar mProgressBar;

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
  void processPreset(juce::File file);
  void startRecording();
  void stopRecording();
  void savePreset();
  void updateCenterComponent(ParamUI::CenterComponent component);

  SafePointer<juce::DialogWindow> mDialogWindow;
  std::unique_ptr<juce::FileChooser> mFileChooser;
  void displayError(juce::String message);
  // used so other classes can report errors as this class needs to be the one to display it
  juce::String mErrorMessage;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GRainbowAudioProcessorEditor)
};
