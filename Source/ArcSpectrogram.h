/*
  ==============================================================================

    ArcSpectrogram.h
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "GranularSynth.h"
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

  void updateSpectrogram(std::vector<std::vector<float>> *fftData,
                         Utils::FftRanges *fftRanges);
  void updatePositions(std::vector<GranularSynth::GrainPosition> gPositions);

  //============================================================================
  void run() override;

 private:
  std::vector<GranularSynth::GrainPosition> mPositions;
  std::vector<std::vector<float>> *mFftData = nullptr;
  Utils::FftRanges *mFftRanges = nullptr;

  juce::Image mSpectrogramImage;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
