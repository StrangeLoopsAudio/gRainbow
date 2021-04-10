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
#include "Utils.h"
#include <math.h>
#include <limits.h>

//==============================================================================
ArcSpectrogram::ArcSpectrogram() : mForwardFFT(mFftOrder), mSpectrogramImage(juce::Image::RGB, 512, 512, true)
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

  g.drawImageAt(mSpectrogramImage, 0, 0);

  /* juce::Path rainbowPath = juce::Path();
  g.setColour(juce::Colours::black);
  int curStripeStart = getHeight() / 4;
  int stripeInc = (getHeight() - curStripeStart) / 7;
  for (int i = 0; i < 8; i++)
  {
    rainbowPath.addCentredArc(getWidth() / 2.f, getHeight(), curStripeStart, curStripeStart, 0, 1.5 * M_PI, 2.5 * M_PI, true);
    curStripeStart += stripeInc;
  }
  g.strokePath(rainbowPath, juce::PathStrokeType(4)); */
}

void ArcSpectrogram::resized()
{
}

void ArcSpectrogram::updateFft()
{
  if (mAudioBuffer == nullptr) return;

  const float* pBuffer = mAudioBuffer->getReadPointer(0);
  int curSample = 0;
  
  bool hasData = mAudioBuffer->getNumSamples() > mFftFrame.size();

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
  updateFftRanges();
  drawSpectrogramImage();
  repaint();
}

void ArcSpectrogram::drawSpectrogramImage()
{
  if (mFftData.empty()) return;
  int startRadius = (getHeight() / 4.0f);
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;
  int height = juce::jmax(2.0f, bowWidth / (float)mFftFrame.size());
  juce::Point<int> startPoint = juce::Point<int>(getWidth() / 2, getHeight());
  mSpectrogramImage = juce::Image(juce::Image::RGB, getWidth(), getHeight(), true);
  juce::Graphics g(mSpectrogramImage);

  for (auto i = 0; i < mFftData.size(); ++i)
  {
    //auto maxLevel = normalizeFftRange(i);
    auto maxLevel = mFftFrameRanges[i];
    for (auto curRadius = startRadius; curRadius < endRadius; ++curRadius)
    {
      float arcLen = M_PI * curRadius * 2;
      int pixPerEntry = arcLen / mFftData.size();
      float radPerc = 1.0f - ((curRadius - startRadius) / (float)bowWidth);
      auto skewedProportionY = 1.0f - std::exp(std::log(radPerc) * 0.2f);
      //auto skewedProportionY = 1.0f - radPerc;
      auto specRow = (size_t)juce::jmap(skewedProportionY, 0.0f, (float)mFftSize / 3.5f);
      
      auto rainbowColour = Utils::getRainbowColour(radPerc);
      g.setColour(rainbowColour);

      auto level = juce::jmap(mFftData[i][specRow], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
      g.setOpacity(level);
      
      float xPerc = (float)i / mFftData.size();
      float angleRad =  (M_PI * xPerc) - (M_PI / 2.0f);
      int width = pixPerEntry + 6;
      
      juce::Point<float> p = startPoint.getPointOnCircumference(curRadius, curRadius, angleRad);
      juce::AffineTransform rotation = juce::AffineTransform();
      rotation = rotation.rotated(angleRad, p.x, p.y);
      juce::Rectangle<float> rect = juce::Rectangle<float>(width, height);
      rect = rect.withCentre(p);
      rect = rect.transformedBy(rotation);
      juce::Path rectPath;
      rectPath.addRectangle(rect);
      
      g.fillPath(rectPath, rotation);
    }
  }
}

juce::Range<float> ArcSpectrogram::normalizeFftRange(int frame)
{
  auto r = mFftFrameRanges[frame];
  auto pullAmt = juce::jmap(r.getLength() / mFftRange.getLength(), 0.0f, mFftRange.getLength(), 1.0f, 10.0f);
  return juce::Range<float>(r.getStart(), r.getEnd() * pullAmt);
}

void ArcSpectrogram::updateFftRanges()
{
  mFftFrameRanges.clear();
  if (mFftData.empty()) return;
  float totalMin = std::numeric_limits<float>::max();
  float totalMax = std::numeric_limits<float>::min();
  for (auto i = 0; i < mFftData.size(); ++i)
  {
    float curMin = std::numeric_limits<float>::max();
    float curMax = std::numeric_limits<float>::min();
    for (auto j = 0; j < mFftFrame.size(); ++j)
    {
      auto val = mFftData[i][j];
      if (val < curMin)
      {
        if (val < totalMin)
        {
          totalMin = val;
        }
        curMin = val;
      }
      if (val > curMax)
      {
        if (val > totalMax)
        {
          totalMax = val;
        }
        curMax = val;
      }
    }
    mFftFrameRanges.push_back(juce::Range<float>(curMin, curMax));
  }
  mFftRange.setStart(totalMin);
  mFftRange.setEnd(totalMax);
}

void ArcSpectrogram::loadedBuffer(juce::AudioSampleBuffer* buffer)
{
  mAudioBuffer = buffer;
  updateFft();
}