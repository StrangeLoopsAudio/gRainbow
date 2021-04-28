/*
  ==============================================================================

    GranularSynth.cpp
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#include "GranularSynth.h"

GranularSynth::GranularSynth(juce::MidiKeyboardState& midiState)
    : mMidiState(midiState), juce::Thread("granular thread") {
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
      for (GrainNote& note : mActiveNotes) {
        GrainPositionFinder::GrainPosition gPos = note.positions[note.curPos];
        float duration = juce::jmap(mDuration, MIN_DURATION, MAX_DURATION);
        auto durSamples = duration * mSampleRate * (1.0f / gPos.pbRate);
        mGrains.add(Grain(&mGaussianEnv, durSamples, gPos.pbRate,
                          gPos.posRatio * mFileBuffer->getNumSamples(),
                          mTotalSamps));
        note.curPos++;
        note.curPos = note.curPos % note.positions.size();
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

void GranularSynth::setFileBuffer(juce::AudioBuffer<float>* buffer,
                                  double sr) {
  mFileBuffer = buffer;
  mSampleRate = sr;
}

void GranularSynth::setPositions(int midiNote, std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  mActiveNotes.add(GrainNote(midiNote, gPositions));
}

void GranularSynth::stopNote(int midiNote) {
  mActiveNotes.removeIf(
      [midiNote](GrainNote& note) { return note.midiNote == midiNote; });
}

void GranularSynth::generateGaussianEnvelope() {
  for (int i = 0; i < mGaussianEnv.size(); i++) {
    mGaussianEnv[i] = std::exp(
        -1.0f * std::pow((i - ((511) / 2.0)) / (0.4 * ((511) / 2.0)), 2.0));
  }
}