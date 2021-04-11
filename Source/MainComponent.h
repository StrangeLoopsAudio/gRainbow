#pragma once

#include <JuceHeader.h>
#include "Grain.h"
#include "ArcSpectrogram.h"
#include "RainbowLookAndFeel.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent, juce::Thread
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
  void run() override;

private:

  typedef struct GrainNote
  {
    int midiNote;
    int voiceNum;
    GrainNote(int midiNote, int voiceNum): midiNote(midiNote), voiceNum(voiceNum) {}
  } GrainNote;

  void openNewFile();

  RainbowLookAndFeel mRainbowLookAndFeel;
  juce::AudioFormatManager mFormatManager;
  juce::AudioBuffer<float> mFileBuffer;

  /* Grain control */
  juce::Array<Grain> mGrains;
  long mTotalSamps;
  juce::Array<GrainNote> mActiveNotes;
  bool mShouldPlayTest = false;
  double mSampleRate;

  /* UI Components */
  juce::TextButton mBtnOpenFile;
  juce::TextButton mBtnPlay;
  juce::TextButton mBtnStop;
  juce::Slider mSliderPosition;
  juce::Label mLabelPosition;
  ArcSpectrogram mArcSpec;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
