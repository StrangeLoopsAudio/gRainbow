/*
 ==============================================================================
 
 Files.h
 Created: 28 Jan 2024 3:27:51pm
 Author:  brady
 
 ==============================================================================
 */

#pragma once

#include <juce_core/juce_core.h>

namespace Utils {

// Paths for bookkeeping files
static const juce::File FILE_DATA_BASE = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("StrangeLoops").getChildFile("gRainbow");
static const juce::File FILE_RECENT_FILES = FILE_DATA_BASE.getChildFile("_recentFiles.json");
static constexpr int MAX_RECENT_FILES = 20;

static juce::var getRecentFiles() {
  // Save loaded file in recent files list
  juce::var filesJson;
  {
    juce::FileInputStream input(FILE_RECENT_FILES);
    if (input.openedOk()) {
      filesJson = juce::JSON::parse(input);
      if (!filesJson.isArray()) filesJson = juce::var(juce::Array<juce::var>());
      return filesJson;
    }
  }
  return filesJson;
}

static void addRecentFile(juce::String newFile) {
  auto recentFiles = getRecentFiles();
  juce::Array<juce::var>* filesArray = recentFiles.getArray();
  filesArray->removeAllInstancesOf(newFile);
  if (filesArray->size() > MAX_RECENT_FILES) filesArray->remove(filesArray->begin());
  filesArray->addIfNotAlreadyThere(newFile);
  juce::FileOutputStream output(FILE_RECENT_FILES);
  if (output.openedOk()) {
    output.setPosition(0);
    output.truncate();
    juce::JSON::writeToStream(output, recentFiles);
  }
}

}  // namespace Utils
