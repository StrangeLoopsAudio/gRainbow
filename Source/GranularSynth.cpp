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
  mGrains.ensureStorageAllocated(MAX_GRAINS);
  startThread();
}

GranularSynth::~GranularSynth() { stopThread(4000); }

void GranularSynth::run() {
  while (!threadShouldExit()) {

    float maxDurSamples = mSampleRate * (MAX_DURATION_MS / 1000.0);
    float durMs = juce::jmap(mDuration, MIN_DURATION_MS, MAX_DURATION_MS);
    
    if (mFileBuffer != nullptr) {
      // Add one grain per active note
      for (GrainNote& gNote : mActiveNotes) {
        if (mGrains.size() >= MAX_GRAINS) continue;
        int numPositions = gNote.positions.size();
        int posToPlay = -1;
        juce::Random random;
        int newPos = juce::jlimit(0, numPositions - 1, (int)(random.nextFloat() * numPositions));
        for (int i = 0; i < numPositions; ++i) {
          int pos = (newPos + i) % numPositions;
          if (gNote.positions[pos].isEnabled) posToPlay = pos;
        }
        if (posToPlay != -1) {
          GrainPositionFinder::GrainPosition gPos = gNote.positions[posToPlay];
          float durSamples =
              mSampleRate * (durMs / 1000) * (1.0f / gPos.pbRate);
          durSamples = juce::jmin(durSamples, maxDurSamples);
          float posSamples = gPos.pitch.posRatio * mFileBuffer->getNumSamples();
          float posOffset =
              juce::jmap(random.nextFloat(), 0.0f,
                         (gPos.pitch.duration * mFileBuffer->getNumSamples()) -
                             durSamples);
          auto grain = Grain(&mGaussianEnv, durSamples, gPos.pbRate,
                             posSamples + posOffset, mTotalSamps);
          mGrains.add(grain);
        }
      }
    }
    auto waitPeriod = juce::jmap(1.0f - mRate, durMs / 8, durMs / 2);
    wait(waitPeriod);
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