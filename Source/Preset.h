/*
  ==============================================================================

    Preset.h
    Created: 13 Aug 2021 7:51:51pm
    Author:  sfricke

  ==============================================================================
*/

#pragma once

// All information about the preset file layout. Done in seperate file for
// future when possible different versions have different file structures and
// want a single location to document it all.
namespace Preset {

// ascii for 'gbow'
const uint32_t MAGIC = 0x67626f77;

// current version
const uint32_t VERSION_MAJOR = 0;
const uint32_t VERSION_MINOR = 0;

struct Header {
  uint32_t magic;
  uint32_t versionMajor;
  uint32_t versionMinor;

  // Audio Buffer info
  uint32_t audioBufferSize;  // bytes of raw data
  double audioBufferSamplerRate;
  int32_t audioBufferNumberOfSamples;
  int32_t audioBufferChannel;

  // UI Spec image info
  // for now instead of creating linked list structure, just list the images
  // used
  uint32_t specImageSpectrogramSize;
  uint32_t specImageHpcpSize;
  uint32_t specImageDetectedSize;

  uint32_t reserved[32];
};

// Version 0.0 layout
// ------------------
// - Header
// - Encoded audio buffer blob
// - List of UI spec images as png blob
// - XML of user param (binary form)

}  // namespace Preset