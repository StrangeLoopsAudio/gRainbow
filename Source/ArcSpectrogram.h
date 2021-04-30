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

  void processBuffer(juce::AudioBuffer<float> &fileBuffer,
                         std::vector<TransientDetector::Transient> *transients);
  void updatePositions(int midiNote,
      std::vector<GrainPositionFinder::GrainPosition> gPositions);

  //============================================================================
  void run() override;

  void buttonClicked(juce::Button *btn) override;

 private:
  static constexpr auto FFT_SIZE = 2048;
  static constexpr auto HOP_SIZE = 1024;

  Fft mFft;

  std::vector<GrainPositionFinder::GrainPosition> mGPositions;
  std::vector<TransientDetector::Transient> *mTransients = nullptr;
  int mCurNote = 0;
  
  juce::Image mSpectrogramImage;
  juce::OwnedArray<PositionMarker> mPositionMarkers;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
