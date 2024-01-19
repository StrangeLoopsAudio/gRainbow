/*
  ==============================================================================

    TitlePresetPanel.cpp
    Created: 15 Dec 2023 12:02:12am
    Author:  brady

  ==============================================================================
*/

#include "TitlePresetPanel.h"
#include "Utils/Utils.h"
#include "Utils/Colour.h"
#include "BinaryData.h"
#include "Version.h"

//==============================================================================
TitlePresetPanel::TitlePresetPanel() {
  mProductLogo = juce::PNGImageFormat::loadFrom(BinaryData::logo_png, BinaryData::logo_pngSize);
  
  // Title and version labels
  mLabelTitle.setText("gRainbow", juce::dontSendNotification);
  mLabelTitle.setFont(Utils::getTitleFont());
  mLabelTitle.setJustificationType(juce::Justification::topRight);
  addAndMakeVisible(mLabelTitle);
  mLabelVersion.setText(juce::String("v") + CURRENT_VERSION, juce::dontSendNotification);
  mLabelVersion.setFont(Utils::getFont());
  mLabelVersion.setJustificationType(juce::Justification::bottomRight);
  addAndMakeVisible(mLabelVersion);
  
  // Company label
  mLabelCompany.setText("by Strange Loops Audio", juce::dontSendNotification);
  mLabelCompany.setFont(Utils::getFont());
  mLabelCompany.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mLabelCompany);
  
  // Open file button (clicks handled by PluginEditor)
  juce::Image normal = juce::PNGImageFormat::loadFrom(BinaryData::openFileNormal_png, BinaryData::openFileNormal_pngSize);
  juce::Image over = juce::PNGImageFormat::loadFrom(BinaryData::openFileOver_png, BinaryData::openFileOver_pngSize);
  btnOpenFile.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, over, 1.0f,
                         juce::Colours::transparentBlack, over, 1.0f, juce::Colours::transparentBlack);
  btnOpenFile.setTooltip("Load new sample from file or preset");
  addAndMakeVisible(btnOpenFile);
  
  // Save button (clicks handled by PluginEditor)
  normal = juce::PNGImageFormat::loadFrom(BinaryData::saveNormal_png, BinaryData::saveNormal_pngSize);
  over = juce::PNGImageFormat::loadFrom(BinaryData::saveOver_png, BinaryData::saveOver_pngSize);
  btnSavePreset.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, over, 1.0f,
                           juce::Colours::transparentBlack, over, 1.0f, juce::Colours::transparentBlack);
  btnSavePreset.setTooltip("Save everything as a preset");
  addAndMakeVisible(btnSavePreset);
  // if reloading and images are done, then enable right away
//  mBtnSavePreset.setEnabled(mParameters.ui.specComplete);
  
  // Recording button
//  normal = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
//  over = juce::PNGImageFormat::loadFrom(BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
//  mBtnRecord.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, over, 1.0f,
//                       juce::Colours::transparentBlack, over, 1.0f, juce::Colours::transparentBlack);
//  mBtnRecord.onClick = [this] {
//    if (mRecorder.isRecording()) {
//      stopRecording();
//    } else {
//      startRecording();
//    }
//  };
//  mBtnRecord.setTooltip("Record to add new sample");
//  addAndMakeVisible(mBtnRecord);
//  
//  // Plugin info button
//  normal = juce::PNGImageFormat::loadFrom(BinaryData::infoNormal_png, BinaryData::infoNormal_pngSize);
//  over = juce::PNGImageFormat::loadFrom(BinaryData::infoOver_png, BinaryData::infoOver_pngSize);
//  mBtnInfo.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, over, 1.0f,
//                     juce::Colours::transparentBlack, over, 1.0f, juce::Colours::transparentBlack);
//  mBtnInfo.onClick = [] { juce::URL(MANUAL_URL).launchInDefaultBrowser(); };
//  mBtnInfo.setTooltip("Open gRainbow manual");
//  addAndMakeVisible(mBtnInfo);
  
  // File info label
  //mLabelFileName.setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
  labelFileName.setFont(Utils::getFont());
  labelFileName.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(labelFileName);
}

TitlePresetPanel::~TitlePresetPanel() {

}

void TitlePresetPanel::paint(juce::Graphics& g) {
  // Fill panel
  g.setColour(Utils::PANEL_COLOUR);
  g.fillRoundedRectangle(getLocalBounds().toFloat().expanded(0, 20).translated(0, -20), 10);
  
  // Fill preset area
  g.setColour(Utils::BG_COLOUR);
  g.fillRoundedRectangle(mRectPreset, 5);
  
  // Draw product logo
  g.drawImage(mProductLogo, mRectProductLogo,
              juce::RectanglePlacement(juce::RectanglePlacement::yBottom | juce::RectanglePlacement::yTop), false);
}

void TitlePresetPanel::resized() {
  juce::Rectangle<int> r = getLocalBounds().reduced(Utils::PADDING, Utils::PADDING);
  
  // Preset area
  int panelWidth = r.getWidth() / 3;
  auto titleLeft = r.removeFromLeft(panelWidth);
  auto titleRight = r.removeFromRight(panelWidth);

  // Product logo
  mRectProductLogo = titleLeft.removeFromLeft(titleLeft.getHeight()).toFloat();
  mRectPreset = r.toFloat();
  
  // Title and version
  titleLeft.removeFromRight(30);
  // Positioning handled by justification
  mLabelTitle.setBounds(titleLeft);
  mLabelVersion.setBounds(titleLeft);
  
  // Preset area
  auto presetArea = r.reduced(Utils::PADDING, Utils::PADDING);
  const int btnWidth = presetArea.getHeight();
  btnOpenFile.setBounds(presetArea.removeFromLeft(btnWidth));
  presetArea.removeFromLeft(Utils::PADDING);
  btnSavePreset.setBounds(presetArea.removeFromLeft(btnWidth).reduced(4, 4));
  presetArea.removeFromLeft(Utils::PADDING);
  // remaining space on sides remaing is for file information
  labelFileName.setBounds(presetArea);
  
  // Company logo
  mLabelCompany.setBounds(titleRight);
  
}
