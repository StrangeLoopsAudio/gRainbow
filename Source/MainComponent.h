#pragma once

#include <JuceHeader.h>

#include "ArcSpectrogram.h"
#include "AudioRecorder.h"
#include "Fft.h"
#include "GrainPositionFinder.h"
#include "GranularSynth.h"
#include "PitchDetector.h"
#include "PositionBox.h"
#include "RainbowKeyboard.h"
#include "RainbowLookAndFeel.h"
#include "TransientDetector.h"
#include "Utils.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
                      juce::Timer,
                      juce::Thread {
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
  void paintOverChildren(juce::Graphics& g) override;
  void resized() override;

  void run() override;

  //==============================================================================
  void timerCallback() override;

  void fastDebugMode();

 private:
  /* Algorithm Constants */
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 2048;
  static constexpr auto RECORDING_FILE = "gRainbow_user_recording.wav";
  static constexpr auto NUM_BOXES = 4;

  /* UI Layout */
  static constexpr auto BTN_PANEL_HEIGHT = 50;
  static constexpr auto BTN_PADDING = 5;
  static constexpr auto OPEN_FILE_WIDTH = 80;
  static constexpr auto PANEL_WIDTH = 300;
  static constexpr auto ROW_PADDING_HEIGHT = 10;
  static constexpr auto KNOB_HEIGHT = 50;
  static constexpr auto LABEL_HEIGHT = 20;
  static constexpr auto SPEC_WIDTH = 20;
  static constexpr auto SPEC_HEIGHT = 20;
  static constexpr auto PROGRESS_SIZE = 80;
  static constexpr auto KEYBOARD_HEIGHT = 200;
  static constexpr auto MIN_NOTE_NUM = 45;
  static constexpr auto MAX_NOTE_NUM = 56;
  static constexpr auto NOTE_BULB_SIZE = 10;
  static constexpr auto NOTE_DISPLAY_HEIGHT = 20;

  /* DSP Modules */
  TransientDetector mTransientDetector;
  PitchDetector mPitchDetector;
  GrainPositionFinder mPositionFinder;
  GranularSynth mSynth;
  AudioRecorder mRecorder;

  /* UI Components */
  juce::ImageComponent mLogo;
  juce::ImageButton mBtnOpenFile;
  juce::ImageButton mBtnRecord;
  ArcSpectrogram mArcSpec;
  RainbowKeyboard mKeyboard;
  juce::ProgressBar mProgressBar;
  std::array<PositionBox, NUM_BOXES> mPositionBoxes;
  juce::Rectangle<float> mNoteDisplayRect;

  /* Bookkeeping */
  double mSampleRate;
  PitchDetector::PitchClass mCurPitchClass = PitchDetector::PitchClass::NONE;
  std::vector<GrainPositionFinder::GrainPosition> mCurPositions;
  juce::MidiKeyboardState mKeyboardState;
  juce::File mRecordedFile;
  juce::AudioDeviceManager mAudioDeviceManager;
  std::array<std::array<int, NUM_BOXES>,
             PitchDetector::PitchClass::NUM_PITCH_CLASSES>
      mPositions;
  double mLoadingProgress = 0.0;
  bool mIsProcessingComplete = false;
  bool mStartedPlayingTrig = false;
  Fft mFft;
  juce::AudioBuffer<float> mFileBuffer;
  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::AudioFormatManager mFormatManager;
  juce::MidiMessageCollector mMidiCollector;

  void openNewFile(const char* path = nullptr);
  void processFile(juce::File file);
  void startRecording();
  void stopRecording();
  void resetPositions();
  int findNextPosition(int boxNum, bool isRight);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
