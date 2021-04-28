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
#include "Utils.h"

//==============================================================================
/*
 */
class ArcSpectrogram : public juce::AnimatedAppComponent, juce::Thread {
 public:
  ArcSpectrogram();
  ~ArcSpectrogram() override;

  void update() override{};
  void paint(juce::Graphics &) override;
  void resized() override;

  void processBuffer(juce::AudioBuffer<float> &fileBuffer,
                         std::vector<TransientDetector::Transient> *transients);
  void updatePositions(
      std::vector<GrainPositionFinder::GrainPosition> gPositions);

  //============================================================================
  void run() override;

 private:
  static constexpr auto FFT_SIZE = 2048;
  static constexpr auto HOP_SIZE = 1024;

  Fft mFft;

  std::vector<GrainPositionFinder::GrainPosition> mPositions;
  std::vector<TransientDetector::Transient> *mTransients = nullptr;

  juce::Image mSpectrogramImage;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
