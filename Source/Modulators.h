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
  ModSource(): mOutput(0.0f), mBlockSize(512), mSampleRate(48000) {}
  
  void prepare(int blockSize, double sampleRate) { mBlockSize = blockSize; mSampleRate = sampleRate; }
  virtual void processBlock() {}
  virtual float getOutput() { return mOutput; }
  
protected:
  double mSampleRate;
  int mBlockSize;
  float mOutput;
};

// Simple LFO modulator source
class LFOModSource : ModSource {
public:
  typedef struct Shape {
    juce::String name;
    std::function<float (float x)> calc;
  } Shape;
  
  // LFO shape calculation functions (all bipolar, can change to unipolar with (x + 1)/2 )
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
  
  static constexpr int NUM_LFO_SHAPES = 4; // Increment when adding more shapes
  static const std::array<const Shape, NUM_LFO_SHAPES> LFO_SHAPES;
  
  LFOModSource() {}
  
  void processBlock() override;
  
  juce::AudioParameterChoice* shape;
  juce::AudioParameterFloat* rate;
  juce::AudioParameterFloat* depth;
  juce::AudioParameterFloat* phase;
  juce::AudioParameterBool* sync;
  juce::AudioParameterBool* bipolar;
  
private:
  double mCurPhase = 0.0;
};
