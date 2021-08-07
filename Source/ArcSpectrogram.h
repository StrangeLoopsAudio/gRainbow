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
#include "GranularSynth.h"
#include "TransientDetector.h"
#include "Utils.h"

//==============================================================================
/*
 */
class ArcSpectrogram : public juce::AnimatedAppComponent,
                       juce::Thread {
 public:
  ArcSpectrogram();
  ~ArcSpectrogram() override;

  void update() override{};
  void paint(juce::Graphics &) override;
  void resized() override;

  void setSampleRate(double sampleRate) { mSampleRate = sampleRate; }
  void resetBuffers();
  void loadBuffer(std::vector<std::vector<float>> *buffer, Utils::SpecType type);
  void setTransients(std::vector<TransientDetector::Transient> *transients);
  void setPositions(std::vector<GrainPositionFinder::GrainPosition> gPositions);
  void setNoteOn(int midiNote,
                 std::vector<GrainPositionFinder::GrainPosition> gPositions);
  void setNoteOff() { mIsPlayingNote = false; }

  //============================================================================
  void run() override;

 private:
  static constexpr auto MIN_FREQ = 100;
  static constexpr auto MAX_FREQ = 5000;
  static constexpr auto BUFFER_PROCESS_TIMEOUT = 10000;

  // UI variables
  static constexpr auto SPEC_TYPE_HEIGHT = 50;
  static constexpr auto SPEC_TYPE_WIDTH = 130;
  static constexpr auto NUM_COLS = 600;
  // Colours
  static constexpr auto COLOUR_MULTIPLIER = 20.0f;

  // Pixel vibration
  static constexpr auto PIXEL_VIBRATION_SIZE = 2;
  static constexpr auto MAX_PIXEL_VIBRATION = 15;
  static constexpr auto MAX_VIBRATION_OFFSET =
      MAX_PIXEL_VIBRATION * MAX_PIXEL_VIBRATION;

  std::array<std::vector<std::vector<float>> *, Utils::SpecType::NUM_TYPES - 1>
      mBuffers;
  std::vector<GrainPositionFinder::GrainPosition> mGPositions;
  std::vector<TransientDetector::Transient> *mTransients = nullptr;

  int mCurNote = 0;
  bool mIsPlayingNote = false;
  double mSampleRate;
  Utils::SpecType mProcessType = Utils::SpecType::LOGO;

  std::random_device mRandomDevice{};
  std::mt19937 mGenRandom{mRandomDevice()};
  std::normal_distribution<> mNormalRand{0.0f, 0.4f};

  std::array<juce::Image, Utils::SpecType::NUM_TYPES> mImages;
  juce::ComboBox mSpecType;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
