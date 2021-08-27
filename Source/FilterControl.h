/*
  ==============================================================================

    FilterControl.h
    Created: 3 Aug 2021 4:46:28pm
    Author:  nrmvl

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Utils.h"
#include "Parameters.h"

//==============================================================================
/*
*/
class FilterControl : public juce::Component {
 public:
  FilterControl();
  ~FilterControl() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void mouseMove(const juce::MouseEvent& event) override;
  void mouseExit(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event) override;

  //static enum FilterType { NONE, LOWPASS, HIGHPASS, BANDPASS };
  void setActive(bool isActive);
  void setCutoff(float cutoff);
  void setResonance(float Resonance);
  void setColour(juce::Colour colour);
  void setFilterType(Utils::FilterType filterType);

  
 private:

   /* Parameters */
  bool mIsActive = false;
  float mCutoff = 50000.0f;
  float mResonance = ParamDefaults::Resonance_DEFAULT;
  Utils::FilterType mFilterType = Utils::FilterType::NO_FILTER;
  static constexpr auto FILTER_TYPE_BUTTON_HEIGHT = 40;
  juce::Colour mColour;
  
  juce::Rectangle<float> mLowPassRect;
  juce::Rectangle<float> mHighPassRect;
  juce::Rectangle<float> mBandPassRect;
  int mFilter = 0;
  Utils::FilterType mCurHoverFilterType = Utils::NO_FILTER;
  Utils::FilterType mCurSelectedFilterType = Utils::NO_FILTER;
  static constexpr auto PADDING_SIZE = 6;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterControl)
};
