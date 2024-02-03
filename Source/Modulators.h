/*
 ==============================================================================

 Modulators.h
 Created: 01 Jan 2024 8:34:54pm
 Author:  brady

 Influenced by (but not copied from) Surge XT's modulation code

 Made abstract enough to be reasonably portable to other applications

 ==============================================================================
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include "Utils/Envelope.h"
#include <math.h>

enum ModSourceType {
  LFO,
  ENV,
  MACRO
};

// Base class for modulator sources.. processBlock() should be called once per block and the output can be grabbed with getOutput()
class ModSource {
public:
  ModSource(int idx, juce::Colour _colour): mIdx(idx), colour(_colour), mSampleRate(48000), mBlockSize(512), mOutput(0.0f) {}

  int getIdx() { return mIdx; }
  virtual ModSourceType getType() = 0;
  virtual void processBlock() = 0;
  virtual juce::Range<float> getRange() = 0;
  // Mod sources can override this to let UI elements show the source's progression
  virtual float getPhase() { return 0.0f; }

  void prepare(int blockSize, double sampleRate) {
    mBlockSize = blockSize;
    mSampleRate = sampleRate;
    mRadPerBlock = juce::MathConstants<double>::twoPi * (static_cast<double>(mBlockSize) / mSampleRate);
  }
  float getOutput() { return mOutput; }

  juce::Colour colour; // Colour of mod source (shown on sliders when modulations are applied)

protected:
  int mIdx; // Index of the mod source among elements of the same ModSourceType (e.g., LFO idx goes from 0 to 2, ENV goes from 0 - 1)
  double mSampleRate;
  int mBlockSize;
  float mOutput;
  float mRadPerBlock = juce::MathConstants<double>::twoPi * (static_cast<double>(mBlockSize) / mSampleRate);
};

/* A single modulation
 - destination parameter is encoded in hashmap key using parameter id
 */
typedef struct Modulation {
  Modulation(): source(nullptr), depth(0.0f) {}
  Modulation(ModSource* _source, float _depth): source(_source), depth(_depth) {}
  ModSource* source; // Would like to make as a ref but since it's hashed it needs a default constructor
  float depth; // Can be positive (to the right) or negative (to the left)
} Modulation;

// LFO modulation source
class LFOModSource : public ModSource {
public:
  typedef struct Shape {
    juce::String name;
    std::function<float (float x)> calc;
  } Shape;

  static constexpr int NUM_LFO_SHAPES = 4; // Increment when adding more shapes
  static const std::array<const Shape, NUM_LFO_SHAPES> LFO_SHAPES;

  LFOModSource(int idx, juce::Colour _colour): ModSource(idx, _colour) {}

  ModSourceType getType() override { return ModSourceType::LFO; }
  void processBlock() override;
  juce::Range<float> getRange() override;
  float getPhase() override;

  // Sets the sync rate of in blocks/bar using 1/(bars/sec * samp/block * sec/samp)
  void setSyncRate(float barsPerSec) { mBarsPerSec = barsPerSec; }
  void checkRetrigger() { if (retrigger && retrigger->get()) mCurPhase = phase->get(); }

  // Must be initialized externally (in this app done in Parameters.cpp)
  juce::AudioParameterChoice* shape;
  juce::AudioParameterFloat* rate;
  juce::AudioParameterFloat* phase;
  juce::AudioParameterBool* sync;
  juce::AudioParameterBool* bipolar;
  juce::AudioParameterBool* retrigger;

private:
  // LFO shape calculation functions (all bipolar -1.0 to 1.0)
  static float calcSine(float x) {
    return std::sin(x);
  }
  static float calcTri(float x) {
    return (2.0f / M_PI) * fabs(fmodf(x - M_PI_2 + juce::MathConstants<float>::twoPi, juce::MathConstants<float>::twoPi) - M_PI) - 1;
  }
  static float calcSquare(float x) {
    return fmodf(x, juce::MathConstants<float>::twoPi) < M_PI ? 1.0f : -1.0f;
  }
  static float calcSaw(float x) {
    const float scaledX = (x - M_PI) / juce::MathConstants<float>::twoPi;
    return 2.0f * (scaledX - floorf(0.5f + scaledX));
  }

  float mBarsPerSec = 1.0f; // Rate of blocks/bar when synced to host bpm
  double mCurPhase = 0.0; // Phase in radians
};

// Envelope modulation source
class EnvModSource : public ModSource {
public:
  EnvModSource(int idx, juce::Colour _colour): ModSource(idx, _colour) {}

  ModSourceType getType() override { return ModSourceType::ENV; }
  void processBlock() override;
  juce::Range<float> getRange() override;
  float getPhase() override { return (float)mEnv.state; }

  void handleNoteOn(int ts) {
    mEnv.noteOn(ts);
    mCurTs = ts;
  }
  void handleNoteOff(int ts) {
    mEnv.noteOff(ts);
    mCurTs = ts;
  }

  // Must be initialized externally (in this app done in Parameters.cpp)
  juce::AudioParameterFloat* attack;
  juce::AudioParameterFloat* decay;
  juce::AudioParameterFloat* sustain;
  juce::AudioParameterFloat* release;

private:
  Utils::EnvelopeADSR mEnv;
  int mCurTs = 0;
};

// Macro modulation source
class MacroModSource : public ModSource {
public:
  MacroModSource(int idx, juce::Colour _colour): ModSource(idx, _colour) {}

  ModSourceType getType() override { return ModSourceType::MACRO; }
  void processBlock() override;
  juce::Range<float> getRange() override;

  // Must be initialized externally (in this app done in Parameters.cpp)
  juce::AudioParameterFloat* macro;
};
