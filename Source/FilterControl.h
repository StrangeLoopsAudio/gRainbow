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

  //static enum FilterType { NONE, LOWPASS, HIGHPASS, BANDPASS };
  void setActive(bool isActive);
  void setCutoff(float cutoff);
  void setStrength(float strength);
  void setColour(juce::Colour colour);
  void setFilterType(Utils::FilterType filterType);

 private:

   /* Parameters */
  bool mIsActive = false;
  float mCutoff = 0.5f;
  float mStrength = ParamDefaults::STRENGTH_DEFAULT;
  Utils::FilterType mFilterType = Utils::FilterType::EMPTY;
  static constexpr auto FILTER_TYPE_BUTTON_HEIGHT = 40;
  juce::Colour mColour;
  juce::Path filterPath;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterControl)
};
