/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "Components/TitlePresetPanel.h"
#include "Components/EnvelopeADSR.h"
#include "Components/EnvelopeGrain.h"
#include "Components/AdjustPanel.h"
#include "Components/Modulators/EnvPanel.h"
#include "Components/Modulators/LFOPanel.h"
#include "Components/MasterPanel.h"
#include "Components/TrimSelection.h"
#include "Components/ArcSpectrogram.h"
#include "Components/Piano/PianoPanel.h"
#include "Components/Settings.h"
#include "Components/RainbowLookAndFeel.h"
//#include "DSP/AudioRecorder.h"
#include "DSP/Fft.h"
#include "DSP/GranularSynth.h"
#include "Utils/Utils.h"
#include "Utils/DSP.h"

//==============================================================================
/**
 * @brief The "Main component" that parents all UI elements.
 */
class GRainbowAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     public juce::FileDragAndDropTarget,
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
  static constexpr int BTN_PANEL_HEIGHT = 65;
  static constexpr int PROGRESS_SIZE = 80;
  static constexpr auto FILE_RECORDING = "gRainbow_user_recording.wav";
  static constexpr const char* MANUAL_URL = "https://github.com/StrangeLoopsAudio/gRainbow/blob/main/README.md";
                                       
  // DSP Modules
  GranularSynth& mSynth;
//  AudioRecorder mRecorder;

  // Synth owns, but need to grab params on reloading of plugin
  Parameters& mParameters;

  // main center UI component
  ArcSpectrogram mArcSpec;
  TrimSelection mTrimSelection;
  juce::ProgressBar mProgressBar;
  double PROGRESS_VALUE = -1.0; // Makes indeterminate spinning, shouldn't change

  // UI Components
  TitlePresetPanel mTitlePresetPanel;
  juce::TabbedComponent mTabsGrains, mTabsLFOs, mTabsEnvs;
  EnvelopeADSR mEnvAdsr;
  EnvelopeGrain mEnvGrain;
  AdjustPanel mAdjustPanel;
  EnvPanel mModEnv1, mModEnv2, mModEnv3;
  LFOPanel mModLFO1, mModLFO2, mModLFO3;
  MasterPanel mMasterPanel;
  PianoPanel mPianoPanel;
  juce::SharedResourcePointer<juce::TooltipWindow> mTooltipWindow;
  SettingsComponent mSettings;

  // Bookkeeping
  juce::File mRecordedFile;
  juce::AudioDeviceManager mAudioDeviceManager;
  bool mIsFileHovering = false;
  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::Path mBorderPath;

  juce::Image mRainImage;
  juce::Rectangle<float> mLeftRain;
  juce::Rectangle<float> mRightRain;
  int mLeftRainDeltY = 0;
  int mRightRainDeltY = 0;

  void openNewFile(const char* path = nullptr);
  void loadFile(juce::File file);
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
