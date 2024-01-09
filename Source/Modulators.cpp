/*
 ==============================================================================
 
 Modulators.cpp
 Created: 01 Jan 2024 8:34:54pm
 Author:  brady
 
 Influenced by but not copied from Surge XT's modulation code
 
 ==============================================================================
 */

#include "Modulators.h"
#include "Parameters.h"
#include <cmath>

const std::array<const LFOModSource::Shape, LFOModSource::NUM_LFO_SHAPES> LFOModSource::LFO_SHAPES = {
  {
    { "sine", LFOModSource::calcSine },
    { "tri", LFOModSource::calcTri },
    { "square", LFOModSource::calcSquare },
    { "saw", LFOModSource::calcSaw }
  }
};

void LFOModSource::processBlock() {
  // Calculate LFO output
  mOutput = LFO_SHAPES[shape->getIndex()].calc(mCurPhase) / 2.0f;
  if (!bipolar->get()) mOutput = mOutput + 0.5f; // Make unipolar if needed
  
  // Update phase for the next block
  if (sync->get()) {
    const float divInBars = std::pow(2, juce::roundToInt(ParamRanges::SYNC_DIV_MAX * rate->convertTo0to1(rate->get())));
    mCurPhase += mRadPerBlock / (mBarsPerSec / divInBars);
  } else {
    mCurPhase += mRadPerBlock * rate->get();
  }
  
  // Wrap phase to keep it in the range [0, 2PI)
  if (mCurPhase >= juce::MathConstants<double>::twoPi) mCurPhase -= juce::MathConstants<double>::twoPi;
}

juce::Range<float> LFOModSource::getRange() {
  if (bipolar->get()) {
    return juce::Range<float>(-0.5f, 0.5f);
  } else {
    return juce::Range<float>(0.0f, 1.0f);
  }
}

void EnvModSource::processBlock() {
  // Calculate envelope value
  mOutput = mEnv.getAmplitude(mCurTs, attack->get() * mSampleRate, decay->get() * mSampleRate, sustain->get(), release->get() * mSampleRate);
  mCurTs += mBlockSize;
}

juce::Range<float> EnvModSource::getRange() {
  return juce::Range<float>(0.0f, 1.0f);
}

void MacroModSource::processBlock() {
  // Use macro value as output
  mOutput = macro->get();
}

juce::Range<float> MacroModSource::getRange() {
  return juce::Range<float>(0.0f, 1.0f);
}
