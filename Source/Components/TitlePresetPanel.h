/*
  ==============================================================================

    TitlePresetPanel.h
    Created: 15 Dec 2023 12:02:12am
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
 */
class TitlePresetPanel : public juce::Component {
 public:
  TitlePresetPanel();
  ~TitlePresetPanel();

  void paint(juce::Graphics&) override;
  void resized() override;
  
  // Public components to be used by the plugin editor
  juce::ImageButton btnOpenFile;
  juce::ImageButton btnSavePreset;
  juce::Label labelFileName;

 private:
  // Bookkeeping
  juce::Image mProductLogo;
  juce::Image mCompanyLogo;

  // Components
  juce::Label mLabelTitle;
  juce::Label mLabelVersion;
  juce::Label mLabelCompany;
  
  // Rectangles remade on resize()
  juce::Rectangle<float> mRectProductLogo;
  juce::Rectangle<float> mRectCompanyLogo;
  juce::Rectangle<float> mRectPreset;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TitlePresetPanel)
};
