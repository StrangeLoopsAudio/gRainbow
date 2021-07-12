/*
  ==============================================================================

    GranularSynth.cpp
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#include "GranularSynth.h"

GranularSynth::GranularSynth() : juce::Thread("granular thread") {
  generateGaussianEnvelope();
  mTotalSamps = 0;
  mGrains.ensureStorageAllocated(MAX_GRAINS);

  // Initialize grain triggering timestamps
  for (int i = 0; i < mGrainTriggersMs.size(); ++i) {
    float durMs = juce::jmap(mPositionSettings[i].duration, MIN_DURATION_MS,
                             MAX_DURATION_MS);
    mGrainTriggersMs[i] =
        juce::jmap(1.0f - mPositionSettings[i].rate, durMs / 8, durMs / 2);
  }

  startThread();
}

GranularSynth::~GranularSynth() { stopThread(4000); }

void GranularSynth::run() {
  while (!threadShouldExit()) {

    float durMs = juce::jmap(mPositionSettings[mNextPositionToPlay].duration,
                             MIN_DURATION_MS, MAX_DURATION_MS);

    if (mFileBuffer != nullptr) {
      // Add one grain per active note
      for (GrainNote& gNote : mActiveNotes) {
        // Skip if not enabled or full of grains
        if (mGrains.size() >= MAX_GRAINS) continue;
        if (!gNote.positions[mNextPositionToPlay].isActive) continue;

        juce::Random random;
        GrainPositionFinder::GrainPosition gPos =
            gNote.positions[mNextPositionToPlay];
        float durSamples = mSampleRate * (durMs / 1000) * (1.0f / gPos.pbRate);
        float posSamples = gPos.pitch.posRatio * mFileBuffer->getNumSamples();
        float posOffset = juce::jmap(
            random.nextFloat(), 0.0f,
            (gPos.pitch.duration * mFileBuffer->getNumSamples()) - durSamples);
        auto grain = Grain(mGaussianEnv, durSamples, gPos.pbRate,
                           posSamples + posOffset, mTotalSamps,
                           mPositionSettings[mNextPositionToPlay].gain);
        mGrains.add(grain);
      }
    }

    // Get next position to play
    // TODO: reflect actual rate like below in envelops viz
    mGrainTriggersMs[mNextPositionToPlay] =
        juce::jmap(1.0f - mPositionSettings[mNextPositionToPlay].rate,
                   durMs * MIN_RATE_RATIO, durMs * MAX_RATE_RATIO);
    auto nextGrainPosition = mNextPositionToPlay;
    float nextGrainWaitTime = mGrainTriggersMs[mNextPositionToPlay];
    for (int i = 0; i < mGrainTriggersMs.size(); ++i) {
      if (mGrainTriggersMs[i] < nextGrainWaitTime) {
        nextGrainWaitTime = mGrainTriggersMs[i];
        nextGrainPosition = (Utils::PositionColour)i;
      }
    }
    
    mNextPositionToPlay = nextGrainPosition;

    // Subtract wait time from each trigger
    for (int i = 0; i < mGrainTriggersMs.size(); ++i) {
      mGrainTriggersMs[i] -= nextGrainWaitTime;
    }

    wait(juce::jmax(1.0f, nextGrainWaitTime));
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
  } else {
    // Normalize the block before sending onward
    // if grains is empty, don't want to divid by zero
    for (int i = 0; i < blockBuffer->getNumSamples(); ++i) {
      for (int ch = 0; ch < blockBuffer->getNumChannels(); ++ch) {
        float* channelBlock = blockBuffer->getWritePointer(ch);
        channelBlock[i] /= mGrains.size();
      }
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

void GranularSynth::updateParameters(Utils::PositionColour colour,
  PositionParams params) {
  mPositionSettings[colour] = params;
  // Initialize grain triggering timestamps
  for (int i = 0; i < mGrainTriggersMs.size(); ++i) {
    float durMs = juce::jmap(mPositionSettings[i].duration, MIN_DURATION_MS,
                             MAX_DURATION_MS);
    mGrainTriggersMs[i] =
        juce::jmap(1.0f - mPositionSettings[i].rate, durMs / 8, durMs / 2);
  }
}

void GranularSynth::updateParameter(Utils::PositionColour colour,
                                    ParameterType param, float value) {
  switch (param) {
    case ParameterType::RATE:
      mPositionSettings[colour].rate = value;
      break;
    case ParameterType::DURATION:
      mPositionSettings[colour].duration = value;
      break;
    case ParameterType::GAIN:
      mPositionSettings[colour].gain = value;
      break;
    default:
      break;
  }
}

void GranularSynth::generateGaussianEnvelope() {
  for (int i = 0; i < mGaussianEnv.size(); i++) {
    mGaussianEnv[i] = std::exp(
        -1.0f * std::pow((i - ((511) / 2.0)) / (0.4 * ((511) / 2.0)), 2.0));
  }
}