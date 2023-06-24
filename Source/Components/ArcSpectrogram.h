/*
  ==============================================================================

    ArcSpectrogram.h
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

    The Arc Spectrogram is a rainbow arc version of the spectrogram,
    HPCP profile, and detected notes. It also displays the grain positions
    and visualizes grains that are playing.

  ==============================================================================
*/

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include <random>
#include <bitset>

#include "DSP/Fft.h"
#include "Parameters.h"
#include "Utils/Utils.h"
#include "Utils/MidiNote.h"

//==============================================================================
/*
 */
class ArcSpectrogram : public juce::AnimatedAppComponent, juce::Thread {
 public:
  ArcSpectrogram(Parameters& parameters);
  ~ArcSpectrogram() override;

  void update() override {}
  void paint(juce::Graphics &) override;
  void resized() override;

  void reset();
  bool shouldLoadImage(ParamUI::SpecType type) { return !mIsProcessing && !mImagesComplete[type]; }
  void loadSpecBuffer(Utils::SpecBuffer *buffer, ParamUI::SpecType type);
  void loadWaveformBuffer(juce::AudioBuffer<float> *audioBuffer);  // Raw audio samples from file
  void loadPreset();
  void setMidiNotes(const juce::Array<Utils::MidiNote> &midiNotes);
  void setSpecType(ParamUI::SpecType type) { mSpecType.setSelectedItemIndex(type, juce::dontSendNotification); }

  //============================================================================
  void run() override;

  // Callback functions when all images are created
  std::function<void(void)> onImagesComplete = nullptr;

 private:
  static constexpr auto MIN_FREQ = 100;
  static constexpr auto MAX_FREQ = 5000;
  static constexpr auto BUFFER_PROCESS_TIMEOUT = 10000;
  static constexpr auto REFRESH_RATE_FPS = 30;

  // UI variables
  static constexpr auto SPEC_TYPE_HEIGHT = 40;
  static constexpr auto SPEC_TYPE_WIDTH = 100;
  static constexpr auto CANDIDATE_BUBBLE_SIZE = 14;
  static constexpr auto MAX_GRAIN_SIZE = 40;
  static constexpr auto MAX_NUM_GRAINS = 40;
  static constexpr auto NUM_COLS = 600;
  // Colours
  static constexpr auto COLOUR_MULTIPLIER = 20.0f;

  typedef struct ArcGrain {
    ParamGenerator *paramGenerator;
    float gain;
    float envIncSamples;  // How many envelope samples to increment each frame
    int numFramesActive;
    Utils::PitchClass pitchClass;
    ArcGrain(ParamGenerator *paramGenerator_, float gain_, float envIncSamples_, Utils::PitchClass pitchClass_)
        : paramGenerator(paramGenerator_),
          gain(gain_),
          envIncSamples(envIncSamples_),
          numFramesActive(0),
          pitchClass(pitchClass_) {}
  } ArcGrain;

  // Parameters
  // Use to save state since if the plugin is closed and open, will need these
  // to restore the state
  Parameters& mParameters;

  // Buffers used to generate the images
  std::array<void *, ParamUI::SpecType::COUNT> mBuffers;

  // Bookkeeping
  std::bitset<Utils::PitchClass::COUNT> mActivePitchClass;
  juce::Array<ArcGrain> mArcGrains;
  bool mIsProcessing = false;
  bool mImagesComplete[ParamUI::SpecType::COUNT];

  // UI values saved on resize
  juce::Point<float> mCenterPoint;
  int mStartRadius;
  int mEndRadius;
  int mBowWidth;

  std::random_device mRandomDevice{};
  std::mt19937 mGenRandom{mRandomDevice()};
  std::normal_distribution<> mNormalRand{0.0f, 0.4f};

  juce::ComboBox mSpecType;

  void onImageComplete(ParamUI::SpecType specType);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
