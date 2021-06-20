/*
  ==============================================================================

    ArcSpectrogram.h
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Fft.h"
#include "GranularSynth.h"
#include "TransientDetector.h"
#include "PositionMarker.h"
#include "Utils.h"
#include <random>

//==============================================================================
/*
 */
class ArcSpectrogram : public juce::AnimatedAppComponent,
                       juce::Thread,
                       juce::Button::Listener {
 public:
  ArcSpectrogram();
  ~ArcSpectrogram() override;

  enum SpecType {
    LOGO,
    SPECTROGRAM,
    HPCP,
    NOTES,
    NUM_TYPES
  };

  std::function<void(int, GrainPositionFinder::GrainPosition)>
      onPositionUpdated = nullptr;

  void update() override{};
  void paint(juce::Graphics &) override;
  void resized() override;

  void setSampleRate(double sampleRate) { mSampleRate = sampleRate; }
  void resetBuffers();
  void loadBuffer(std::vector<std::vector<float>> *buffer, SpecType type);
  void setTransients(std::vector<TransientDetector::Transient> *transients);
  void setNoteOn(int midiNote,
      std::vector<GrainPositionFinder::GrainPosition> gPositions);
  void setNoteOff() { mIsPlayingNote = false; }

  //============================================================================
  void run() override;

  void buttonClicked(juce::Button *btn) override;

 private:
  static constexpr auto MIN_FREQ = 100;
  static constexpr auto MAX_FREQ = 5000;
  static constexpr auto BUFFER_PROCESS_TIMEOUT = 10000;

  // UI variables
  static constexpr auto POSITION_MARKER_WIDTH = 10;
  static constexpr auto POSITION_MARKER_HEIGHT = 15;
  static constexpr auto SPEC_TYPE_HEIGHT = 50;
  static constexpr auto SPEC_TYPE_WIDTH = 130;
  static constexpr auto NUM_COLS = 600;
  static constexpr auto SUN_RAY_WIDTH = 0.005;
  //Colours
  static constexpr auto COLOUR_MULTIPLIER = 20.0f;
  static constexpr auto COLOUR_SUN_CENTER = 0xFFFFFF74;
  static constexpr auto COLOUR_SUN_REGULAR_RAYS = 0x33FFFF74;
  static constexpr auto COLOUR_SUN_END = 0xffffffce;
  static constexpr auto COLOUR_RAYS_END = 0x00ffffce;
  //static constexpr auto COLOUR_NIGHT = 0x8806031B;
  //static constexpr auto COLOUR_DAY = 0xFF00566B;
  //Pixel vibration
  static constexpr auto PIXEL_VIBRATION_SIZE = 2;
  static constexpr auto MAX_PIXEL_VIBRATION = 15;
  static constexpr auto MAX_VIBRATION_OFFSET =
      MAX_PIXEL_VIBRATION * MAX_PIXEL_VIBRATION;

  // TODO: make this relative
  static constexpr auto LOGO_PATH = "C:/Users/brady/Documents/GitHub/gRainbow/gRainbow-circles.png";

  std::array<std::vector<std::vector<float>> *, SpecType::NUM_TYPES - 1> mBuffers;
  std::vector<GrainPositionFinder::GrainPosition> mGPositions;
  std::vector<TransientDetector::Transient> *mTransients = nullptr;

  int mCurNote = 0;
  bool mIsPlayingNote = false;
  double mSampleRate;
  SpecType mProcessType = SpecType::LOGO;

  std::random_device mRandomDevice{};
  std::mt19937 mGenRandom{mRandomDevice()};
  std::normal_distribution<> mNormalRand{0.0f, 0.4f};

  std::array<juce::Image, SpecType::NUM_TYPES> mImages;
  juce::ComboBox mSpecType;
  juce::OwnedArray<PositionMarker> mPositionMarkers;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
