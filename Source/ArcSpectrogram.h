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

  void loadedBuffer(juce::AudioSampleBuffer& buffer);

private:
  static constexpr auto fftOrder = 10;
  static constexpr auto fftSize = 1 << fftOrder;

  juce::dsp::FFT mForwardFFT;
  juce::Image mSpectrogramImage;
  bool mIsLoaded = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
