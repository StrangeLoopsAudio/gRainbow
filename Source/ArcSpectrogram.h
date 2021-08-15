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

#include <JuceHeader.h>

#include <random>

#include "Fft.h"
#include "Parameters.h"
#include "Utils.h"

//==============================================================================
/*
 */
class ArcSpectrogram : public juce::AnimatedAppComponent, juce::Thread {
 public:
  ArcSpectrogram(ParamsNote &paramsNote, ParamUI &paramUI);
  ~ArcSpectrogram() override;

  // The logo is not a spec type as its just the default image until buffer is
  // loaded
  enum SpecType { INVALID = -1, SPECTROGRAM = 0, HPCP, DETECTED, COUNT };

  void update() override{};
  void paint(juce::Graphics &) override;
  void resized() override;

  void reset();
  void loadBuffer(std::vector<std::vector<float>> *buffer, SpecType type);
  void setNoteOn(Utils::PitchClass pitchClass);
  void setNoteOff() { mIsPlayingNote = false; }

  //============================================================================
  void run() override;

  static constexpr auto FILE_RECORDING = "gRainbow_user_recording.wav";
  static constexpr auto FILE_SPECTROGRAM = "gRainbow_spec.png";
  static constexpr auto FILE_HPCP = "gRainbow_hpcp.png";
  static constexpr auto FILE_DETECTED = "gRainbow_detected.png";

 private:
  static constexpr auto MIN_FREQ = 100;
  static constexpr auto MAX_FREQ = 5000;
  static constexpr auto BUFFER_PROCESS_TIMEOUT = 10000;
  static constexpr auto REFRESH_RATE_FPS = 30;

  // UI variables
  static constexpr auto SPEC_TYPE_HEIGHT = 50;
  static constexpr auto SPEC_TYPE_WIDTH = 130;
  static constexpr auto MAX_GRAIN_SIZE = 40;
  static constexpr auto NUM_COLS = 600;
  // Colours
  static constexpr auto COLOUR_MULTIPLIER = 20.0f;

  typedef struct ArcGrain {
    ParamGenerator *paramGenerator;
    float gain;
    float envIncSamples;  // How many envelope samples to increment each frame
    int numFramesActive;
    ArcGrain(ParamGenerator *paramGenerator, float gain, float envIncSamples)
        : paramGenerator(paramGenerator),
          gain(gain),
          envIncSamples(envIncSamples),
          numFramesActive(0) {}
  } ArcGrain;

  // Parameters
  // Use to save state since if the plugin is closed and open, will need these
  // to restore the state
  ParamsNote &mParamsNote;
  ParamUI &mParamUI;

  // Buffers used to generate the images
  std::array<std::vector<std::vector<float>> *, SpecType::COUNT> mBuffers;

  // Bookkeeping
  Utils::PitchClass mCurPitchClass;
  juce::Array<ArcGrain> mArcGrains;
  bool mIsPlayingNote;
  SpecType mProcessType;

  std::random_device mRandomDevice{};
  std::mt19937 mGenRandom{mRandomDevice()};
  std::normal_distribution<> mNormalRand{0.0f, 0.4f};

  juce::Image mLogoImage;
  juce::ComboBox mSpecType;

  void grainCreatedCallback(int genIdx, float durationSec, float envGain);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
