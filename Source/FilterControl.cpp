/*
  ==============================================================================

    FilterControl.cpp
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FilterControl.h"

//==============================================================================
FilterControl::FilterControl() {
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
}

FilterControl::~FilterControl() {}

void FilterControl::paint(juce::Graphics& g) {
  juce::Colour envColour = mIsActive ? mColour : juce::Colours::darkgrey;
  g.setFont(14.0f);

  // draw filter type rectangles
  g.setColour(envColour.brighter());
  g.drawRoundedRectangle(mLowPassRect, 10.0f, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(FILTER_TYPE_NAMES[1], mLowPassTextRect, juce::Justification::centred);

  g.setColour(envColour.brighter());
  g.drawRoundedRectangle(mHighPassRect, 10.0f, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(FILTER_TYPE_NAMES[2], mHighPassTextRect, juce::Justification::centred);

  g.setColour(envColour.brighter());
  g.drawRoundedRectangle(mBandPassRect, 10.0f, 2.0f);
  g.setColour(juce::Colours::white);
  g.drawText(FILTER_TYPE_NAMES[3], mBandPassTextRect, juce::Justification::centred);

  switch (mCurHoverFilterType) {
    // Select current filter type
    case (Utils::FilterType::LOWPASS):
      g.setColour(envColour.withAlpha(0.3f));
      g.fillRoundedRectangle(mLowPassRect, 10.0f);
      break;
    case (Utils::FilterType::HIGHPASS):
      g.setColour(envColour.withAlpha(0.3f));
      g.fillRoundedRectangle(mHighPassRect, 10.0f);
      break;
    case (Utils::FilterType::BANDPASS):
      g.setColour(envColour.withAlpha(0.3f));
      g.fillRoundedRectangle(mBandPassRect, 10.0f);
      break;
  }

  // Draw selected filter path
  juce::Path mFilterPath;
  juce::Path mHighlightPath;
  const float highlightWidth = 3.0f;
  const juce::Point<float> localTopLeft = getLocalBounds().getTopLeft().toFloat();
  const juce::Point<float> localBottomLeft = getLocalBounds().getBottomLeft().toFloat();
  const float cutoffWidth = getWidth() * mCutoff;

  switch (mFilterType) {
    case (Utils::FilterType::LOWPASS):
      g.setColour(envColour.withAlpha(0.5f));
      g.fillRoundedRectangle(mLowPassRect, 10.0f);

      g.setFillType(juce::ColourGradient(envColour, localTopLeft, envColour.withAlpha(0.4f), localBottomLeft, false));

      mFilterPath.startNewSubPath(juce::Point<float>(0, mBtnResPadding));
      mFilterPath.lineTo(juce::Point<float>(juce::jmax(0.0f, cutoffWidth - RES_WIDTH), mBtnResPadding));
      mFilterPath.lineTo(juce::Point<float>(cutoffWidth, mBtnPadding + (1.0f - mResonance) * MAX_RES_HEIGHT));
      mFilterPath.lineTo(mFilterPath.getCurrentPosition().translated(.5f * 0.1f * getWidth(), 0.0f).withY(getHeight()));
      mFilterPath.lineTo(juce::Point<float>(0, getHeight()));
      mFilterPath.lineTo(juce::Point<float>(0, 0));
      mFilterPath.closeSubPath();
      g.fillPath(mFilterPath);

      // Draw highlights on top of path
      g.setColour(mIsActive ? envColour.brighter().brighter() : juce::Colours::darkgrey);
      mHighlightPath.addLineSegment(juce::Line<float>(0, mBtnResPadding, mCutoff * getWidth() - RES_WIDTH, mBtnResPadding),
                                    highlightWidth);
      mHighlightPath.addLineSegment(juce::Line<float>(mCutoff * getWidth() - RES_WIDTH, mBtnResPadding, mCutoff * getWidth(),
                                                      mBtnPadding + (1.0f - mResonance) * MAX_RES_HEIGHT),
                                    highlightWidth);
      mHighlightPath.addLineSegment(juce::Line<float>(mCutoff * getWidth(), mBtnPadding + (1.0f - mResonance) * MAX_RES_HEIGHT,
                                                      (cutoffWidth) + 0.5f * 0.1f * getWidth(), getHeight() * 1.0f),
                                    highlightWidth);
      mHighlightPath.closeSubPath();
      g.fillPath(mHighlightPath);
      break;
    case (Utils::FilterType::HIGHPASS):
      g.setColour(envColour.withAlpha(0.5f));
      g.fillRoundedRectangle(mHighPassRect, 10.0f);

      g.setFillType(juce::ColourGradient(envColour, localTopLeft, envColour.withAlpha(0.4f), localBottomLeft, false));

      mFilterPath.startNewSubPath(getWidth() * 1.0f, mBtnResPadding);
      mFilterPath.lineTo(cutoffWidth + RES_WIDTH, mBtnResPadding);
      mFilterPath.lineTo(cutoffWidth, mBtnPadding + (1.0f - mResonance) * MAX_RES_HEIGHT);
      mFilterPath.lineTo(mFilterPath.getCurrentPosition().translated(-(0.5f * 0.1f * getWidth()), 0).withY(getHeight()));
      mFilterPath.lineTo(juce::Point<float>(getWidth() * 1.0f, getHeight() * 1.0f));
      mFilterPath.lineTo(juce::Point<float>(getWidth() * 1.0f, mBtnPadding));
      mFilterPath.closeSubPath();
      g.fillPath(mFilterPath);

      // Draw highlights on top of path
      g.setColour(mIsActive ? envColour.brighter().brighter() : juce::Colours::darkgrey);

      mHighlightPath.addLineSegment(juce::Line<float>(getWidth(), mBtnResPadding, cutoffWidth + RES_WIDTH, mBtnResPadding),
                                    highlightWidth);
      mHighlightPath.addLineSegment(juce::Line<float>(cutoffWidth + RES_WIDTH, mBtnResPadding, cutoffWidth,
                                                      mBtnPadding + (1.0f - mResonance) * MAX_RES_HEIGHT),
                                    highlightWidth);
      mHighlightPath.addLineSegment(juce::Line<float>(cutoffWidth, mBtnPadding + (1.0f - mResonance) * MAX_RES_HEIGHT,
                                                      (cutoffWidth)-0.5f * 0.1f * getWidth(), getHeight()),
                                    highlightWidth);

      mHighlightPath.closeSubPath();
      g.fillPath(mHighlightPath);
      break;
    case (Utils::FilterType::BANDPASS):
      g.setFillType(juce::ColourGradient(envColour, localTopLeft, envColour.withAlpha(0.4f), localBottomLeft, false));

      mFilterPath.startNewSubPath(0, getHeight());
      mFilterPath.lineTo(cutoffWidth - (1.0f - mResonance) * 0.1f * getWidth(), getHeight());
      mFilterPath.lineTo(
          mFilterPath.getCurrentPosition().translated(((1.0f - mResonance) * 0.1f * getWidth()), 0).withY(mBtnPadding));
      mFilterPath.lineTo(mFilterPath.getCurrentPosition().translated(0.25f * getWidth(), 0).withY(mBtnPadding));
      mFilterPath.lineTo(
          mFilterPath.getCurrentPosition().translated((1.0f - mResonance) * 0.1f * getWidth(), 0).withY(getHeight()));
      mFilterPath.lineTo(getWidth(), getHeight());
      mFilterPath.lineTo(0, getHeight());
      mFilterPath.closeSubPath();
      g.fillPath(mFilterPath);

      // Draw highlights on top of path
      g.setColour(mIsActive ? envColour.brighter().brighter() : juce::Colours::darkgrey);

      mHighlightPath.addLineSegment(
          juce::Line<float>(cutoffWidth - (1.0f - mResonance) * 0.1f * getWidth(), getHeight(), cutoffWidth, mBtnPadding),
          highlightWidth);
      mHighlightPath.addLineSegment(juce::Line<float>(cutoffWidth, mBtnPadding, cutoffWidth + 0.25f * getWidth(), mBtnPadding),
                                    highlightWidth);
      mHighlightPath.addLineSegment(
          juce::Line<float>(cutoffWidth + 0.25f * getWidth(), mBtnPadding,
                            cutoffWidth + 0.25f * getWidth() + (1.0f - mResonance) * 0.1f * getWidth(), getHeight()),
          highlightWidth);
      mHighlightPath.closeSubPath();
      g.fillPath(mHighlightPath);

      g.setColour(envColour.withAlpha(0.5f));
      g.fillRoundedRectangle(mBandPassRect, 10.0f);
      break;
    case (Utils::FilterType::NO_FILTER):
      g.setFillType(juce::ColourGradient(envColour, localTopLeft, envColour.withAlpha(0.4f), localBottomLeft, false));
      juce::Colour noFilterColor = juce::Colours::darkgrey;
      g.setColour(noFilterColor);
      mFilterPath.startNewSubPath(0, mBtnPadding);
      mFilterPath.lineTo(getWidth(), mBtnPadding);
      mFilterPath.lineTo(getWidth(), getHeight());
      mFilterPath.lineTo(0, getHeight());
      mFilterPath.closeSubPath();
      g.fillPath(mFilterPath);

      // Draw highlights on top of path
      g.setColour(noFilterColor.brighter().brighter());

      mHighlightPath.addLineSegment(juce::Line<float>(0, mBtnPadding, getWidth(), mBtnPadding), highlightWidth);
      mHighlightPath.closeSubPath();
      g.fillPath(mHighlightPath);
      break;
  }

  g.setColour(envColour);
  g.drawRect(juce::Rectangle<float>(0, FILTER_TYPE_BUTTON_HEIGHT + PADDING_SIZE, getWidth(),
                                    getHeight() - FILTER_TYPE_BUTTON_HEIGHT - PADDING_SIZE),
             2.0f);
}

void FilterControl::resized() {
  const float totalButtonWidth = getWidth() - 4 * PADDING_SIZE;
  const float filterTypeWidth = totalButtonWidth / 3;

  mLowPassRect = juce::Rectangle<float>(PADDING_SIZE, 1.0f, filterTypeWidth, FILTER_TYPE_BUTTON_HEIGHT);
  mLowPassTextRect = mLowPassRect.withHeight(FILTER_TYPE_BUTTON_HEIGHT);
  mHighPassRect = juce::Rectangle<float>(2 * PADDING_SIZE + filterTypeWidth, 1.0f, filterTypeWidth, FILTER_TYPE_BUTTON_HEIGHT);
  mHighPassTextRect = mHighPassRect.withHeight(FILTER_TYPE_BUTTON_HEIGHT);
  mBandPassRect = juce::Rectangle<float>(3 * PADDING_SIZE + 2 * filterTypeWidth, 1.0f, filterTypeWidth, FILTER_TYPE_BUTTON_HEIGHT);
  mBandPassTextRect = mBandPassRect.withHeight(FILTER_TYPE_BUTTON_HEIGHT);

  mBtnPadding = FILTER_TYPE_BUTTON_HEIGHT + 2 * PADDING_SIZE;
  mBtnResPadding = mBtnPadding + MAX_RES_HEIGHT;
}

void FilterControl::mouseMove(const juce::MouseEvent& event) {
  if ((event.y > FILTER_TYPE_BUTTON_HEIGHT + PADDING_SIZE) || (event.x < PADDING_SIZE) || (event.x > getWidth() - PADDING_SIZE)) {
    mCurHoverFilterType = Utils::NO_FILTER;
  } else {
    mFilter = ((event.getEventRelativeTo(this).getPosition().getX() / (float)getWidth()) * 3 + 1);  // Utils::FilterType value
    mCurHoverFilterType = static_cast<Utils::FilterType>(mFilter);
  }
  repaint();
}
void FilterControl::mouseExit(const juce::MouseEvent& event) {
  if (event.y > FILTER_TYPE_BUTTON_HEIGHT + PADDING_SIZE) return;
  mCurHoverFilterType = Utils::NO_FILTER;
  repaint();
}
void FilterControl::mouseUp(const juce::MouseEvent& event) {
  if (event.y > FILTER_TYPE_BUTTON_HEIGHT + PADDING_SIZE) return;
  if (event.eventComponent != this) return;
  mFilter = ((event.getEventRelativeTo(this).getPosition().getX() / (float)getWidth()) * 3 + 1);  // Utils::FilterType value
  Utils::FilterType newFilterType = static_cast<Utils::FilterType>(mFilter);
  if (newFilterType == mFilterType) {
    mFilterType = Utils::FilterType::NO_FILTER;
  } else {
    mFilterType = newFilterType;
    mCutoff = filterTypeToCutoff(mFilterType);
  }
  if (onFilterTypeChange != nullptr) onFilterTypeChange(mFilterType);
  repaint();
}

void FilterControl::setActive(bool isActive) {
  mIsActive = isActive;
  repaint();
}

void FilterControl::setCutoff(float cutoff) {
  mCutoff = cutoff;
  repaint();
}

void FilterControl::setResonance(float resonance) {
  mResonance = resonance;
  repaint();
}

void FilterControl::setColour(juce::Colour colour) {
  mColour = colour;
  repaint();
}
void FilterControl::setFilterType(int filterTypeIndex) {
  mFilterType = static_cast<Utils::FilterType>(filterTypeIndex);
  repaint();
}

float FilterControl::filterTypeToCutoff(Utils::FilterType filterType) {
  switch (filterType) {
    case (Utils::FilterType::LOWPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
    case (Utils::FilterType::HIGHPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_HP_CUTOFF_DEFAULT_HZ);
    case (Utils::FilterType::BANDPASS):
      return ParamRanges::CUTOFF.convertTo0to1(ParamDefaults::FILTER_BP_CUTOFF_DEFAULT_HZ);
  }
}
