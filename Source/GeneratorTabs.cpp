/*
  ==============================================================================

    GeneratorTabs.cpp
    Created: 21 Jul 2021 6:22:07pm
    Author:  brady

  ==============================================================================
*/

#include "GeneratorTabs.h"

#include <JuceHeader.h>

//==============================================================================
GeneratorTabs::GeneratorTabs(NoteParams& noteParams) : mNoteParams(noteParams)  {
  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    GeneratorParams* gen = mNoteParams.notes[mCurPitchClass]->generators[i].get();
    mBtnsEnabled[i].setToggleState(gen->enable->get(), juce::dontSendNotification);
    juce::Colour tabColour = gen->enable->get()
                                 ? juce::Colour(Utils::POSITION_COLOURS[i])
                                 : juce::Colours::darkgrey;
    mBtnsEnabled[i].setColour(juce::ToggleButton::ColourIds::tickColourId,
                              tabColour);
    mBtnsEnabled[i].onClick = [this, i] {
      if (!mNoteParams.notes[mCurPitchClass]->generators[i]->enable->get()) {
        tabChanged(mCurSelectedTab, false, mStates[mCurSelectedTab].isEnabled);
        mCurSelectedTab = (Utils::GeneratorColour)i;
        tabChanged(mCurSelectedTab, true, true);
      } else {
        tabChanged((Utils::GeneratorColour)i, mCurSelectedTab == i, false);
      }
    };
    mBtnsEnabled[i].addMouseListener(this, false);
    addAndMakeVisible(mBtnsEnabled[i]);
  }
}

GeneratorTabs::~GeneratorTabs() {}

void GeneratorTabs::paint(juce::Graphics& g) {
  float tabWidth = getWidth() / Utils::GeneratorColour::NUM_GEN - 2.0f;
  float curStart = 1.0f;
  for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
    GeneratorParams* gen =
        mNoteParams.notes[mCurPitchClass]->generators[i].get();
    juce::Colour tabColour =
        Utils::shouldPlay(gen->enable->get(), gen->solo->get(),
                          gen->waiting->get())
            ? juce::Colour(Utils::POSITION_COLOURS[i])
            : juce::Colours::darkgrey;
    float tabHeight =
        (mCurSelectedTab == i) ? getHeight() + 20.0f : getHeight() - 2.0f;
    juce::Rectangle<float> tabRect =
        juce::Rectangle<float>(curStart, 1.0f, tabWidth, tabHeight);
    if (mCurHoverTab == i && mCurSelectedTab != i) {
      g.setColour(tabColour.withAlpha(0.3f));
      g.fillRoundedRectangle(tabRect, 10.0f);
    }
    g.setColour(tabColour);
    g.drawRoundedRectangle(tabRect, 10.0f, 2.0f);

    juce::Rectangle<int> textRect =
        juce::Rectangle<int>(mBtnsEnabled[i].getRight(), 0,
                             tabRect.getRight() - mBtnsEnabled[i].getRight() - 6, getHeight());
    if (gen->solo->get()) {
      g.setColour(juce::Colours::blue);
      g.fillRect(textRect.withSizeKeepingCentre(textRect.getWidth() / 2.0f, textRect.getHeight() / 2.0f));
    }
    g.setColour(juce::Colours::white);
    g.drawFittedText(juce::String("g") + juce::String(i + 1), textRect,
                     juce::Justification::centred, 1);
    curStart += tabWidth + 2.0f;
  }
}

void GeneratorTabs::mouseMove(const juce::MouseEvent& event) {
  int tabHover = (event.getEventRelativeTo(this).getPosition().getX() / (float)getWidth()) *
                 Utils::GeneratorColour::NUM_GEN;
  mCurHoverTab = tabHover;
  repaint();
}

void GeneratorTabs::mouseExit(const juce::MouseEvent& event) {
  mCurHoverTab = -1;
  repaint();
}

void GeneratorTabs::mouseUp(const juce::MouseEvent& event) {
  if (event.eventComponent != this) return;
  int tabClick = (event.getEventRelativeTo(this).getPosition().getX() /
                  (float)getWidth()) *
                 Utils::GeneratorColour::NUM_GEN;
  if (tabClick != mCurSelectedTab) {
    tabChanged(mCurSelectedTab, false,
               mStates[mCurSelectedTab].isEnabled);
    tabChanged((Utils::GeneratorColour)tabClick, true,
               mStates[tabClick].isEnabled);
  }
  mCurSelectedTab = (Utils::GeneratorColour)tabClick;
  repaint();
}

void GeneratorTabs::resized() {
  juce::Rectangle<int> r = getLocalBounds();

  // Enable/disable buttons
  juce::Rectangle<int> btnRect = juce::Rectangle<int>(TOGGLE_SIZE, TOGGLE_SIZE);
  float tabWidth = getWidth() / Utils::GeneratorColour::NUM_GEN - 2.0f;
  float curStart = 1.0f;
  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    mBtnsEnabled[i].setBounds(btnRect.withCentre(juce::Point<int>(curStart + (TOGGLE_SIZE / 2) + 5, getHeight() / 2)));
    curStart += tabWidth + 2.0f;
  }
}

void GeneratorTabs::tabChanged(Utils::GeneratorColour tab, bool isSelected,
                              bool isEnabled) {
  if (onTabChanged != nullptr) {
    onTabChanged(tab, isSelected, isEnabled);
  }
  repaint();
}