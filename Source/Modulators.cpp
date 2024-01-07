/*
 ==============================================================================
 
 Modulators.cpp
 Created: 01 Jan 2024 8:34:54pm
 Author:  brady
 
 Influenced by but not copied from Surge XT's modulation code
 
 ==============================================================================
 */

#include "Modulators.h"
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
  // Calculate sine wave value for the LFO
  mOutput = LFO_SHAPES[shape->getIndex()].calc(2.0f * M_PI * rate->get() * mCurPhase / mSampleRate);
  if (!bipolar->get()) mOutput = (mOutput + 1.0f) / 2.0f; // Make uniploar if needed
  mOutput *= depth->get(); // Scale by depth
  
  // Update phase for the next block
  mCurPhase += static_cast<double>(mBlockSize);
  
  // Wrap phase to keep it in the range [0, mSampleRate)
  if (mCurPhase >= mSampleRate) mCurPhase -= mSampleRate;
}
