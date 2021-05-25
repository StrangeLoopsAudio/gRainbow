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

  std::function<void(int, GrainPositionFinder::GrainPosition)>
      onPositionUpdated = nullptr;

  void update() override{};
  void paint(juce::Graphics &) override;
  void resized() override;

  void processBuffer(juce::AudioBuffer<float>* fileBuffer, double sampleRate);
  void loadBuffer(std::vector<std::vector<float>> *buffer);
  void setTransients(std::vector<TransientDetector::Transient> *transients);
  void updatePositions(int midiNote,
      std::vector<GrainPositionFinder::GrainPosition> gPositions);

  //============================================================================
  void run() override;

  void buttonClicked(juce::Button *btn) override;

 private:
  static constexpr auto FFT_SIZE = 4096;
  static constexpr auto HOP_SIZE = 2048;
  static constexpr auto MIN_FREQ = 100;
  static constexpr auto MAX_FREQ = 5000;
  static constexpr auto NUM_COLS = 600;

  juce::AudioBuffer<float>* mFileBuffer = nullptr;
  std::vector<std::vector<float>> *mLoadedBuffer = nullptr;
  Fft mFft;

  std::vector<GrainPositionFinder::GrainPosition> mGPositions;
  std::vector<TransientDetector::Transient> *mTransients = nullptr;
  int mCurNote = 0;
  double mSampleRate;

  juce::Image mSpectrogramImage;
  juce::OwnedArray<PositionMarker> mPositionMarkers;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
