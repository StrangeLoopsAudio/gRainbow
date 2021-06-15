/*
  ==============================================================================

    GranularSynth.cpp
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#include "GranularSynth.h"

GranularSynth::GranularSynth()
    : juce::Thread("granular thread") {
  generateGaussianEnvelope();
  mTotalSamps = 0;
  startThread();
}

GranularSynth::~GranularSynth() { stopThread(4000); }

void GranularSynth::run() {
  while (!threadShouldExit()) {

    float maxDurSamples = mSampleRate * (MAX_DURATION_MS / 1000.0);
    
    if (mFileBuffer != nullptr) {
      // Add one grain per active note
      for (GrainNote& gNote : mActiveNotes) {
        if (mGrains.size() > MAX_GRAINS) continue;
        int numPositions = gNote.positions.size();
        int posToPlay = -1;
        juce::Random random;
        int newPos = juce::roundToInt(random.nextFloat() * (numPositions - 1));
        for (int i = 0; i < numPositions; ++i) {
          if (gNote.positions[(newPos + i) % numPositions].isEnabled)
            posToPlay = newPos;
        }
        if (posToPlay != -1) {
          GrainPositionFinder::GrainPosition gPos = gNote.positions[posToPlay];          
          auto durSamples =
              gPos.pitch.duration * mFileBuffer->getNumSamples() * (1.0f / gPos.pbRate);
          durSamples = juce::jmin(durSamples, maxDurSamples);
          auto grain = Grain(&mGaussianEnv, durSamples, gPos.pbRate,
                             gPos.pitch.posRatio * mFileBuffer->getNumSamples(),
                             mTotalSamps);
          mGrains.add(grain);
        }
      }
    }
    auto waitPeriod = 1.0f / juce::jmap(mRate, MIN_RATE, MAX_RATE);
    wait(waitPeriod * 1000);
  }
}

void GranularSynth::process(juce::AudioBuffer<float>* blockBuffer) {
  // Add contributions from each grain
  juce::Array<Grain> grains = mGrains; // Copy to avoid threading issues
  for (int i = 0; i < blockBuffer->getNumSamples(); ++i) {
    for (int g = 0; g < grains.size(); ++g) {
      grains[g].process(*mFileBuffer, *blockBuffer, mTotalSamps);
    }
    mTotalSamps++;
  }

  // Reset timestamps if no grains active to keep numbers low
  if (mGrains.isEmpty()) {
    mTotalSamps = 0;
  }

  // Normalize the block before sending onward
  for (int i = 0; i < blockBuffer->getNumSamples(); ++i) {
    for (int ch = 0; ch < blockBuffer->getNumChannels(); ++ch) {
      float* channelBlock = blockBuffer->getWritePointer(ch);
      channelBlock[i] /= mGrains.size();
    }
  }

  // Delete expired grains
  mGrains.removeIf([this](Grain& g) { return mTotalSamps > (g.trigTs + g.duration); });
}

void GranularSynth::setFileBuffer(juce::AudioBuffer<float>* buffer, double sr) {
  mFileBuffer = buffer;
  mSampleRate = sr;
}

void GranularSynth::setPositions(
    PitchDetector::PitchClass pitchClass,
    std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  mActiveNotes.add(GrainNote(pitchClass, gPositions));
}

void GranularSynth::stopNote(PitchDetector::PitchClass pitchClass) {
  mActiveNotes.removeIf(
      [pitchClass](GrainNote& note) { return note.pitchClass == pitchClass; });
}

void GranularSynth::generateGaussianEnvelope() {
  for (int i = 0; i < mGaussianEnv.size(); i++) {
    mGaussianEnv[i] = std::exp(
        -1.0f * std::pow((i - ((511) / 2.0)) / (0.4 * ((511) / 2.0)), 2.0));
  }
}