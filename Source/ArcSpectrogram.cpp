/*
  ==============================================================================

    ArcSpectrogram.cpp
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#define _USE_MATH_DEFINES

#include <JuceHeader.h>
#include "ArcSpectrogram.h"
#include <math.h>
#include <limits.h>

//==============================================================================
ArcSpectrogram::ArcSpectrogram() : mForwardFFT(mFftOrder)
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

  drawSpectrogram(g);

  /*juce::Path rainbowPath = juce::Path();
  g.setColour(juce::Colours::white);
  float curStripeStart = 0;
  for (int i = 0; i < 8; i++)
  {
    rainbowPath.addCentredArc(getWidth() / 2.f, getHeight(), curStripeStart, curStripeStart, 0, M_PI, -M_PI, true);
    curStripeStart += getHeight() / 7;
  }
  g.strokePath(rainbowPath, juce::PathStrokeType(2));*/
}

void ArcSpectrogram::resized()
{
  // This method is where you should set the bounds of any child
  // components that your component contains..

}

void ArcSpectrogram::updateFft()
{
  if (mAudioBuffer == nullptr) return;

  const float* pBuffer = mAudioBuffer->getReadPointer(0);
  int curSample = 0;
  
  bool hasData = mAudioBuffer->getNumSamples() > mFftData.size();

  mFftData.clear();

  while (hasData)
  {
    const float* startSample = &pBuffer[curSample];
    int numSamples = mFftFrame.size();
    if (curSample + mFftFrame.size() > mAudioBuffer->getNumSamples()) {
      numSamples = (mAudioBuffer->getNumSamples() - curSample);
    }
    mFftFrame.fill(0.0f);
    memcpy(mFftFrame.data(), startSample, numSamples);

    // then render our FFT data..
    mForwardFFT.performFrequencyOnlyForwardTransform(mFftFrame.data());

    // Add fft data to our master array
    mFftData.push_back(std::vector<float>(mFftFrame.begin(), mFftFrame.end()));

    curSample += mFftFrame.size();
    if (curSample > mAudioBuffer->getNumSamples()) hasData = false;
  }
  mIsLoaded = true;
  repaint();
}

void ArcSpectrogram::drawSpectrogram(juce::Graphics& g)
{
  if (mFftData.empty()) return;
  auto maxLevel = getFftMinMax();
  int startRadius = (getHeight() / 4.0f);
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;
  int height = juce::jmax(2.0f, bowWidth / (float)mFftFrame.size());
  juce::Point<int> startPoint = juce::Point<int>(getWidth() / 2, getHeight());
  
  for (auto curRadius = startRadius; curRadius < endRadius; ++curRadius)
  {
    float arcLen = M_PI * curRadius * 2;
    int pixPerEntry = arcLen / mFftData.size();
    float radPerc = 1.0f - ((curRadius - startRadius) / (float)bowWidth);
    auto skewedProportionY = 1.0f - std::exp(std::log(radPerc) * 0.2f);
    auto specCol = (size_t)juce::jlimit(0, mFftSize / 2, (int)(skewedProportionY * mFftSize / 2));
    auto rainbowColour = getRainbowColour(radPerc);
    g.setColour(rainbowColour);
    juce::Path pixPath;

    for (auto i = 0; i < mFftData.size(); ++i)
    {
      auto level = juce::jmap(mFftData[i][specCol], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
      g.setOpacity(level);
      
      float xPerc = (float)i / mFftData.size();
      float angleRad = -M_PI + (2 * M_PI * xPerc);
      int width = pixPerEntry + 6;  
      
      juce::Point<float> p = startPoint.getPointOnCircumference(curRadius, curRadius, angleRad);
      juce::AffineTransform rotation = juce::AffineTransform();
      rotation = rotation.rotated(angleRad, p.x, p.y);
      juce::Rectangle<float> rect = juce::Rectangle<float>(width, height);
      rect = rect.withCentre(p);
      pixPath.clear();
      pixPath.addRectangle(rect);
      
      g.fillPath(pixPath, rotation);
    }
  }
}

juce::Range<float> ArcSpectrogram::getFftMinMax()
{
  if (mFftData.empty()) return juce::Range<float>(0, 0);
  float curMin = std::numeric_limits<float>::max();
  float curMax = std::numeric_limits<float>::min();
  for (auto i = 0; i < mFftData.size(); ++i)
  {
    for (auto j = 0; j < mFftData[i].size(); ++j)
    {
      auto val = mFftData[i][j];
      if (val < curMin)
      {
        curMin = val;
      }
      if (val > curMax)
      {
        curMax = val;
      }
    }
  }
  return juce::Range<float>(curMin, curMax);
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
  updateFft();
}