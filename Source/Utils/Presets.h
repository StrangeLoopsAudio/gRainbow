#pragma once

#include <juce_core/juce_core.h>
#include "BinaryData.h"

namespace Utils {

typedef struct {
  juce::String name;
  const char* data;
  const int   size;
} PresetEntry;

static const std::array<const PresetEntry, 2> PRESETS = {
  {
    {"chromatic saw", BinaryData::chromatic_saw_gbow, BinaryData::chromatic_saw_gbowSize},
//    {"billie", BinaryData::billie_gbow, BinaryData::billie_gbowSize}
  }
};

static inline void getBlockForPreset(const PresetEntry& preset, juce::MemoryBlock& block) {
  juce::MemoryInputStream(preset.data, preset.size, false).readIntoMemoryBlock(block);
}

static const PresetEntry* getPresetWithName (juce::String name) {
  auto found = std::find_if(PRESETS.begin(), PRESETS.end(), [name](const PresetEntry& preset) { return preset.name == name; });
  return (found != PRESETS.end()) ? &(*found) : nullptr;
}

static void printAllPresets() {
  for (const auto& preset : PRESETS)
    std::cout << preset.name << std::endl;
}

}  // namespace Utils
