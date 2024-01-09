/*
 ==============================================================================
 
 Modulators.h
 Created: 01 Jan 2024 8:34:54pm
 Author:  brady
 
 Influenced by but not copied from Surge XT's modulation code
 
 ==============================================================================
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

enum ModSourceType {
  LFO_1,
  NUM_MOD_SOURCES
};

// Base class for modulator sources.. processBlock() should be called once per block and the output can be grabbed with getOutput()
class ModSource {
public:
  ModSource(): mSampleRate(48000), mBlockSize(512), mOutput(0.0f) {}
  
  virtual void processBlock() = 0;
  virtual juce::Range<float> getRange() = 0;
  
  void prepare(int blockSize, double sampleRate) {
    mBlockSize = blockSize;
    mSampleRate = sampleRate;
    mRadPerBlock = juce::MathConstants<double>::twoPi * (static_cast<double>(mBlockSize) / mSampleRate);
  }
  float getOutput() { return mOutput; }
  
  juce::Colour colour; // Colour of mod source (shown on sliders when modulations are applied)
  
protected:
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
  float depth;
} Modulation;

// Simple LFO modulator source
class LFOModSource : public ModSource {
public:
  typedef struct Shape {
    juce::String name;
    std::function<float (float x)> calc;
  } Shape;
  
  static constexpr int NUM_LFO_SHAPES = 4; // Increment when adding more shapes
  static const std::array<const Shape, NUM_LFO_SHAPES> LFO_SHAPES;
  
  LFOModSource() { colour = juce::Colour(0xffd8ddef); }
  
  void processBlock() override;
  juce::Range<float> getRange() override;
  
  // Sets the sync rate of in blocks/bar using 1/(bars/sec * samp/block * sec/samp)
  void setSyncRate(float barsPerSec) { mBarsPerSec = barsPerSec; }
  
  juce::AudioParameterChoice* shape;
  juce::AudioParameterFloat* rate;
  juce::AudioParameterFloat* phase;
  juce::AudioParameterBool* sync;
  juce::AudioParameterBool* bipolar;
  
private:
  // LFO shape calculation functions (all bipolar -1.0 to 1.0)
  static float calcSine(float x) {
    return std::sin(x);
  }
  static float calcTri(float x) {
    return (2.0f / M_PI) * std::fabs(std::fmodf(x, juce::MathConstants<float>::twoPi) - M_PI) - 1;
  }
  static float calcSquare(float x) {
    return std::fmodf(x, juce::MathConstants<float>::twoPi) < M_PI ? 1.0f : -1.0f;
  }
  static float calcSaw(float x) {
    const float scaledX = (x - M_PI) / juce::MathConstants<float>::twoPi;
    return 2.0f * (scaledX - std::floorf(0.5f + scaledX));
  }
  
  float mBarsPerSec = 1.0f; // Rate of blocks/bar when synced to host bpm
  double mCurPhase = 0.0; // Phase in radians
};
