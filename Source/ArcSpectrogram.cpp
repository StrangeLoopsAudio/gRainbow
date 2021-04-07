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
ArcSpectrogram::ArcSpectrogram() : mForwardFFT(mFftOrder),
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
  g.fillAll(juce::Colours::black);
  g.setOpacity(1.0f);
  g.drawImage(mSpectrogramImage, getLocalBounds().toFloat());
}

void ArcSpectrogram::resized()
{
  // This method is where you should set the bounds of any child
  // components that your component contains..

}

void ArcSpectrogram::drawSpectrogram()
{
  if (mAudioBuffer == nullptr) return;
  
  auto rightHandEdge = mSpectrogramImage.getWidth() - 1;
  auto imageHeight = mSpectrogramImage.getHeight();

  const float* pBuffer = mAudioBuffer->getReadPointer(0);
  int curSample = 0;
  
  bool hasData = mAudioBuffer->getNumSamples() > mFftData.size();

  while (hasData)
  {
    const float* startSample = &pBuffer[curSample];
    int numSamples = mFftData.size();
    if (curSample + mFftData.size() > mAudioBuffer->getNumSamples()) {
      numSamples = (mAudioBuffer->getNumSamples() - curSample);
    }

    mFftData.fill(0.0f);
    memcpy(mFftData.data(), startSample, numSamples);

    // first, shuffle our image leftwards by 1 pixel..
    mSpectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

    // then render our FFT data..
    mForwardFFT.performFrequencyOnlyForwardTransform(mFftData.data());

    // find the range of values produced, so we can scale our rendering to
    // show up the detail clearly
    auto maxLevel = juce::FloatVectorOperations::findMinAndMax(mFftData.data(), mFftSize / 2);

    for (auto y = 1; y < imageHeight; ++y)
    {
      auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
      auto fftDataIndex = (size_t)juce::jlimit(0, mFftSize / 2, (int)(skewedProportionY * mFftSize / 2));
      auto level = juce::jmap(mFftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

      mSpectrogramImage.setPixelAt(rightHandEdge, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f));
    }

    repaint();

    curSample += mFftData.size();
    if (curSample > mAudioBuffer->getNumSamples()) hasData = false;
  }
  mIsLoaded = true;
}

void ArcSpectrogram::loadedBuffer(juce::AudioSampleBuffer* buffer)
{
  mAudioBuffer = buffer;
  drawSpectrogram();
}