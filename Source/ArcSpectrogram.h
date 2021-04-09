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
class ArcSpectrogram : public juce::Component
{
public:
  ArcSpectrogram();
  ~ArcSpectrogram() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void loadedBuffer(juce::AudioSampleBuffer* buffer);

private:
  static constexpr auto mFftOrder = 10;
  static constexpr auto mFftSize = 1 << mFftOrder;

  juce::dsp::FFT mForwardFFT;
  juce::AudioSampleBuffer* mAudioBuffer;
  juce::Image mSpectrogramImage;
  std::array<float, mFftSize * 2> mFftFrame;
  std::vector<std::vector<float>> mFftData;
  std::vector<juce::Range<float>> mFftFrameRanges;
  juce::Range<float> mFftRange;
  bool mIsLoaded = false;

  void drawSpectrogramImage();
  void updateFft();
  void updateFftRanges();
  juce::Range<float> normalizeFftRange(int frame);
  juce::Colour getRainbowColour(float value);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
