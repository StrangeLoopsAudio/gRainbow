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
  void loadBuffer(std::vector<std::vector<float>> *buffer, SpecType type);
  void setTransients(std::vector<TransientDetector::Transient> *transients);
  void updatePositions(int midiNote,
      std::vector<GrainPositionFinder::GrainPosition> gPositions);

  //============================================================================
  void run() override;

  void buttonClicked(juce::Button *btn) override;

 private:
  static constexpr auto MIN_FREQ = 100;
  static constexpr auto MAX_FREQ = 5000;
  static constexpr auto BUFFER_PROCESS_TIMEOUT = 10000;

  // UI variables
  static constexpr auto SPEC_TYPE_HEIGHT = 50;
  static constexpr auto SPEC_TYPE_WIDTH = 130;
  static constexpr auto NUM_COLS = 600;
  // TODO: make this relative
  static constexpr auto LOGO_PATH = "C:/Users/brady/Documents/GitHub/gRainbow/gRainbow-circles.png";

  std::array<std::vector<std::vector<float>> *, SpecType::NUM_TYPES - 1> mBuffers;
  std::vector<GrainPositionFinder::GrainPosition> mGPositions;
  std::vector<TransientDetector::Transient> *mTransients = nullptr;

  int mCurNote = 0;
  double mSampleRate;
  SpecType mProcessType = (SpecType)0;

  std::array<juce::Image, SpecType::NUM_TYPES> mImages;
  juce::ComboBox mSpecType;
  juce::OwnedArray<PositionMarker> mPositionMarkers;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
