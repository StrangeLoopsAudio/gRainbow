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
    /* Delete expired grains */
    for (int i = mGrains.size() - 1; i >= 0; --i) {
      if (mTotalSamps > (mGrains[i].trigTs + mGrains[i].duration)) {
        mGrains.remove(i);
      }
    }

    // Reset timestamps if no grains active to keep numbers low
    if (mGrains.isEmpty()) {
      mTotalSamps = 0;
      mNextGrainTs = 0;
    }

    if (mFileBuffer != nullptr && mTotalSamps >= mNextGrainTs) {
      float gInterval =
          (1.0f / juce::jmap(mRate, MIN_RATE, MAX_RATE)) * mSampleRate;
      mNextGrainTs = mTotalSamps + gInterval;
      /* Add one grain per active note */
      for (GrainNote& gNote : mActiveNotes) {
        if (gNote.curPos != -1) {
          GrainPositionFinder::GrainPosition gPos =
              gNote.positions[gNote.curPos];
          float duration = juce::jmap(mDuration, MIN_DURATION, MAX_DURATION);
          auto durSamples = duration * mSampleRate * (1.0f / gPos.pbRate);
          mGrains.add(Grain(&mGaussianEnv, durSamples, gPos.pbRate,
                            gPos.posRatio * mFileBuffer->getNumSamples(),
                            mTotalSamps));
          gNote.curPos = getNextPosition(gNote);
        }
      }
    }
    wait(20);
  }
}

void GranularSynth::process(juce::AudioBuffer<float>* blockBuffer) {
  for (int i = 0; i < blockBuffer->getNumSamples(); ++i) {
    for (int g = 0; g < mGrains.size(); ++g) {
      mGrains[g].process(*mFileBuffer, *blockBuffer, mTotalSamps);
    }
    mTotalSamps++;
  }
}

void GranularSynth::setFileBuffer(juce::AudioBuffer<float>* buffer, double sr) {
  mFileBuffer = buffer;
  mSampleRate = sr;
}

void GranularSynth::setPositions(
    PitchDetector::PitchClass pitchClass,
    std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  int startPos = getStartPosition(gPositions);
  mActiveNotes.add(GrainNote(pitchClass, startPos, gPositions));
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

int GranularSynth::getNextPosition(GrainNote& gNote) {
  int firstEnabled = -1;
  for (int i = 0; i < gNote.positions.size(); ++i) {
    int index = (gNote.curPos + 1 + i) % gNote.positions.size();
    if (gNote.positions[index].solo) return index;
    if (gNote.positions[index].isEnabled && firstEnabled == -1) {
      firstEnabled = index;
    }
  }
  return firstEnabled;
}

int GranularSynth::getStartPosition(std::vector<GrainPositionFinder::GrainPosition>& gPositions) {
  int firstEnabled = -1;
  for (int i = 0; i < gPositions.size(); ++i) {
    if (gPositions[i].solo) return i;
    if (gPositions[i].isEnabled && firstEnabled == -1) firstEnabled = i;
  }
  return firstEnabled;
}