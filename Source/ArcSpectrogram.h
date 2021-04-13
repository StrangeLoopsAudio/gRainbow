/*
  ==============================================================================

    ArcSpectrogram.h
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ArcSpectrogram : public juce::AnimatedAppComponent
{
public:
  ArcSpectrogram();
  ~ArcSpectrogram() override;

  void update() override {};
  void paint(juce::Graphics&) override;
  void resized() override;

  void updateSpectrogram(std::vector<std::vector<float>> &fftData);
  void updatePositions(std::vector<float> positionRatios);

private:

  std::vector<float> mPositionRatios;
  std::vector<juce::Range<float>> mFftFrameRanges;
  juce::Range<float> mFftRange;
  
  juce::Image mSpectrogramImage;

  void drawSpectrogramImage(std::vector<std::vector<float>>& fftData);
  void updateFftRanges(std::vector<std::vector<float>>& fftData);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
