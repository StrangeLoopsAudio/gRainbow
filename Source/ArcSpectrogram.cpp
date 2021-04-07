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
  mSpectrogramImage.clear(mSpectrogramImage.getBounds(), juce::Colours::black);

  int numFrames = ((float)mAudioBuffer->getNumSamples() / mFftData.size()) + 1;
  int pixPerFrame = juce::jmax(1, mSpectrogramImage.getWidth() / numFrames);
  DBG("pixPFrame: " << pixPerFrame << ", numFrames: " << numFrames);
  int curFrame = 0;

  while (hasData)
  {
    const float* startSample = &pBuffer[curSample];
    int numSamples = mFftData.size();
    if (curSample + mFftData.size() > mAudioBuffer->getNumSamples()) {
      numSamples = (mAudioBuffer->getNumSamples() - curSample);
    }

    mFftData.fill(0.0f);
    memcpy(mFftData.data(), startSample, numSamples);

    // then render our FFT data..
    mForwardFFT.performFrequencyOnlyForwardTransform(mFftData.data());

    // find the range of values produced, so we can scale our rendering to
    // show up the detail clearly
    auto maxLevel = juce::FloatVectorOperations::findMinAndMax(mFftData.data(), mFftSize / 2);
    auto startX = curFrame * pixPerFrame;
    
    for (auto y = 1; y < imageHeight; ++y)
    {
      auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
      auto fftDataIndex = (size_t)juce::jlimit(0, mFftSize / 2, (int)(skewedProportionY * mFftSize / 2));
      auto level = juce::jmap(mFftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
      auto rainbowColour = getRainbowColour((float)y / imageHeight);
      rainbowColour = rainbowColour.withAlpha(level);
        for (auto x = startX; x < startX + pixPerFrame; ++x)
        {
          mSpectrogramImage.setPixelAt(x, y, rainbowColour);
        }
    }

    repaint();

    curSample += mFftData.size();
    curFrame++;
    if (curSample > mAudioBuffer->getNumSamples()) hasData = false;
  }
  mIsLoaded = true;
}

juce::Colour ArcSpectrogram::getRainbowColour(float value)
{
  int rStripe = (int)std::floor(value * 7);
  float r, g, b;
  switch (rStripe)
  {
  case 0: r = 1; g = 0; b = 0; break;
  case 1: r = 1; g = 0.49; b = 0; break;
  case 2: r = 1; g = 1; b = 0; break;
  case 3: r = 0; g = 1; b = 0; break;
  case 4: r = 0; g = 0; b = 1; break;
  case 5: r = 0.29; g = 0; b = 0.5; break;
  case 6: r = 0.58; g = 0; b = 0.83; break;
  default:
  {
    r = 0; g = 0; b = 0; break;
  }
  }
  return juce::Colour(r * 255.0f, g * 255.0f, b * 255.0f);
}

void ArcSpectrogram::loadedBuffer(juce::AudioSampleBuffer* buffer)
{
  mAudioBuffer = buffer;
  drawSpectrogram();
}