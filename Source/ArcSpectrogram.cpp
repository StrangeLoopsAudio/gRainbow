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
ArcSpectrogram::ArcSpectrogram() : mSpectrogramImage(juce::Image::RGB, 512, 512, true),
juce::Thread("spectrogram thread")
{
  setFramesPerSecond(10);
}

ArcSpectrogram::~ArcSpectrogram()
{
}

void ArcSpectrogram::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::black);

  // Draw fft
  g.drawImageAt(mSpectrogramImage, 0, 0);

  // Draw position marker
  juce::Point<int> centerPoint = juce::Point<int>(getWidth() / 2, getHeight());
  g.setColour(juce::Colours::white);
  for (float posRatio : mPositionRatios)
  {
    auto startPoint = centerPoint.getPointOnCircumference(getHeight() / 4.0f, (1.5 * M_PI) + (posRatio * M_PI));
    auto endPoint = centerPoint.getPointOnCircumference(getHeight(), (1.5 * M_PI) + (posRatio * M_PI));
    g.drawLine(juce::Line<float>(startPoint, endPoint), 2.0f);
  }

  // Draw borders
  g.setColour(juce::Colours::white);
  g.drawRect(getLocalBounds(), 2);

}

void ArcSpectrogram::resized()
{
}

void ArcSpectrogram::run()
{
  if (mFftData == nullptr) return;
  int startRadius = getHeight() / 4.0f;
  int endRadius = getHeight();
  int bowWidth = endRadius - startRadius;
  int height = juce::jmax(2.0f, bowWidth / (float)mFftData->at(0).size());
  juce::Point<int> startPoint = juce::Point<int>(getWidth() / 2, getHeight());
  mSpectrogramImage = juce::Image(juce::Image::RGB, getWidth(), getHeight(), true);
  juce::Graphics g(mSpectrogramImage);

  for (auto i = 0; i < mFftData->size(); ++i)
  {
    for (auto curRadius = startRadius; curRadius < endRadius; ++curRadius)
    {
      float arcLen = M_PI * curRadius * 2;
      int pixPerEntry = arcLen / mFftData->size();
      float radPerc = 1.0f - ((curRadius - startRadius) / (float)bowWidth);
      auto skewedProportionY = 1.0f - std::exp(std::log(radPerc) * 0.2f);
      //auto skewedProportionY = 1.0f - radPerc;
      auto specRow = (size_t)juce::jmap(skewedProportionY, 0.0f, (float)(mFftData->at(i).size() / 2.0f) / 3.5f);

      auto rainbowColour = Utils::getRainbowColour(radPerc);
      g.setColour(rainbowColour);

      auto level = juce::jmap(mFftData->at(i)[specRow], 0.0f, juce::jmax(mFftRange.getEnd(), 1e-5f), 0.0f, 1.0f);
      g.setOpacity(level);

      float xPerc = (float)i / mFftData->size();
      float angleRad = (M_PI * xPerc) - (M_PI / 2.0f);
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

void ArcSpectrogram::updateFftRanges()
{
  if (mFftData == nullptr) return;
  mFftFrameRanges.clear();
  float totalMin = std::numeric_limits<float>::max();
  float totalMax = std::numeric_limits<float>::min();
  for (auto i = 0; i < mFftData->size(); ++i)
  {
    float curMin = std::numeric_limits<float>::max();
    float curMax = std::numeric_limits<float>::min();
    for (auto j = 0; j < mFftData->at(i).size(); ++j)
    {
      auto val = mFftData->at(i)[j];
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

void ArcSpectrogram::updateSpectrogram(std::vector<std::vector<float>>* fftData)
{
  mFftData = fftData;
  updateFftRanges();
  startThread(); // Update spectrogram image
}

void ArcSpectrogram::updatePositions(std::vector<float> positionRatios)
{
  mPositionRatios = positionRatios;
  //repaint();
}