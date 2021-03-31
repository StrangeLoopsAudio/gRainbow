/*
  ==============================================================================

    ArcSpectrogram.cpp
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ArcSpectrogram.h"

//==============================================================================
ArcSpectrogram::ArcSpectrogram() : mForwardFFT(fftOrder),
                                   mSpectrogramImage(juce::Image::RGB, 512, 512, true)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.

}

ArcSpectrogram::~ArcSpectrogram()
{
}

void ArcSpectrogram::paint(juce::Graphics& g)
{
  if (mIsLoaded)
  {
    g.fillAll(juce::Colours::green);
  }
  else
  {
    g.fillAll(juce::Colours::red);
  }
}

void ArcSpectrogram::resized()
{
  // This method is where you should set the bounds of any child
  // components that your component contains..

}

void ArcSpectrogram::loadedBuffer(juce::AudioSampleBuffer& buffer)
{
  mIsLoaded = true;
  repaint();
}