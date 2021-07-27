/*
  ==============================================================================

    PositionTabs.cpp
    Created: 21 Jul 2021 6:22:07pm
    Author:  brady

  ==============================================================================
*/

#include "PositionTabs.h"

#include <JuceHeader.h>

//==============================================================================
PositionTabs::PositionTabs() {
  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    mBtnsEnabled[i].setToggleState(mCurSelectedTab == i, juce::dontSendNotification);
    juce::Colour tabColour = mBtnsEnabled[i].getToggleState()
                                 ? juce::Colour(Utils::POSITION_COLOURS[i])
                                 : juce::Colours::darkgrey;
    mBtnsEnabled[i].setColour(juce::ToggleButton::ColourIds::tickColourId,
                              tabColour);
    mBtnsEnabled[i].onClick = [this, i] {
      tabChanged((Utils::PositionColour)i, mCurSelectedTab == i, mBtnsEnabled[i].getToggleState());
      juce::Colour tabColour = mBtnsEnabled[i].getToggleState()
                                   ? juce::Colour(Utils::POSITION_COLOURS[i])
                                   : juce::Colours::darkgrey;
      mBtnsEnabled[i].setColour(juce::ToggleButton::ColourIds::tickColourId,
                                tabColour);
    };
    mBtnsEnabled[i].addMouseListener(this, false);
    addAndMakeVisible(mBtnsEnabled[i]);
  }
}

PositionTabs::~PositionTabs() {}

void PositionTabs::paint(juce::Graphics& g) {
  float tabWidth = getWidth() / Utils::PositionColour::NUM_POS - 2.0f;
  float curStart = 1.0f;
  for (int i = 0; i < Utils::PositionColour::NUM_POS; ++i) {
    juce::Colour tabColour = mBtnsEnabled[i].getToggleState()
                                 ? juce::Colour(Utils::POSITION_COLOURS[i])
                                 : juce::Colours::darkgrey;
    float tabHeight = (mCurSelectedTab == i) ? getHeight() + 20.0f : getHeight() - 2.0f;
    juce::Rectangle<float> tabRect =
        juce::Rectangle<float>(curStart, 1.0f, tabWidth, tabHeight);
    if (mCurHoverTab == i && mCurSelectedTab != i) {
      g.setColour(tabColour.withAlpha(0.3f));
      g.fillRoundedRectangle(tabRect, 10.0f);
    }
    g.setColour(tabColour);
    g.drawRoundedRectangle(tabRect, 10.0f, 2.0f);

    g.setColour(juce::Colours::white);
    g.drawText(juce::String(i + 1), tabRect.withHeight(getHeight() - 2.0f), juce::Justification::centred);
    curStart += tabWidth + 2.0f;
  }
}

void PositionTabs::mouseMove(const juce::MouseEvent& event) {
  int tabHover = (event.getEventRelativeTo(this).getPosition().getX() / (float)getWidth()) *
                 Utils::PositionColour::NUM_POS;
  mCurHoverTab = tabHover;
  repaint();
}

void PositionTabs::mouseExit(const juce::MouseEvent& event) {
  mCurHoverTab = -1;
  repaint();
}

void PositionTabs::mouseUp(const juce::MouseEvent& event) {
  if (event.eventComponent != this) return;
  int tabClick = (event.getEventRelativeTo(this).getPosition().getX() /
                  (float)getWidth()) *
                 Utils::PositionColour::NUM_POS;
  if (tabClick != mCurSelectedTab) {
    tabChanged(mCurSelectedTab, false,
               mBtnsEnabled[mCurSelectedTab].getToggleState());
    tabChanged((Utils::PositionColour)tabClick, true,
               mBtnsEnabled[tabClick].getToggleState());
  }
  mCurSelectedTab = (Utils::PositionColour)tabClick;
  repaint();
}

void PositionTabs::resized() {
  juce::Rectangle<int> r = getLocalBounds();

  // Enable/disable buttons
  juce::Rectangle<int> btnRect = juce::Rectangle<int>(TOGGLE_SIZE, TOGGLE_SIZE);
  float tabWidth = getWidth() / Utils::PositionColour::NUM_POS - 2.0f;
  float curStart = 1.0f;
  for (int i = 0; i < mBtnsEnabled.size(); ++i) {
    mBtnsEnabled[i].setBounds(btnRect.withCentre(juce::Point<int>(curStart + (TOGGLE_SIZE / 2) + 5, getHeight() / 2)));
    curStart += tabWidth + 2.0f;
  }
}

void PositionTabs::tabChanged(Utils::PositionColour tab, bool isSelected, bool isEnabled) {
  if (onTabChanged != nullptr) {
    onTabChanged(tab, isSelected, isEnabled);
  }
  repaint();
}