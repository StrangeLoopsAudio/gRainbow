/*
  ==============================================================================

    GeneratorTabs.h
    Created: 21 Jul 2021 6:22:07pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Utils.h"
#include "Parameters.h"

//==============================================================================
/*
 */
class GeneratorTabs : public juce::Component {
 public:
  GeneratorTabs(NoteParams& noteParams);
  ~GeneratorTabs() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void mouseMove(const juce::MouseEvent& event) override;
  void mouseExit(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event) override;
  
  std::function<void(Utils::GeneratorColour tab, bool isSelected,
                     bool isEnabled)>
      onTabChanged = nullptr;


 private:
  static constexpr auto TOGGLE_SIZE = 16;

  /* Parameters */
  NoteParams& mNoteParams;

  /* UI Components */
  std::array<juce::ToggleButton, Utils::GeneratorColour::NUM_GEN> mBtnsEnabled;

  /* Bookkeeping */
  Utils::PitchClass mCurPitchClass = Utils::PitchClass::C;
  Utils::GeneratorColour mCurSelectedTab = Utils::GeneratorColour::BLUE;
  int mCurHoverTab = -1;

  void tabChanged(Utils::GeneratorColour tab, bool isSelected, bool isEnabled);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratorTabs)
};
