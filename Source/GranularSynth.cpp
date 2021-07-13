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

    GranularSynth::PositionParams params =
        mPositionSettings[mNextPositionToPlay];
    float durMs = juce::jmap(params.duration,
                             MIN_DURATION_MS, MAX_DURATION_MS);

    if (mFileBuffer != nullptr) {
      // Add one grain per active note
      for (GrainNote& gNote : mActiveNotes) {
        // Skip if not enabled or full of grains
        if (mGrains.size() >= MAX_GRAINS) continue;
        if (!gNote.positions[mNextPositionToPlay].isActive) continue;

        juce::Random random;
        GrainPositionFinder::GrainPosition& gPos =
            gNote.positions[mNextPositionToPlay];
        float durSamples = mSampleRate * (durMs / 1000) * (1.0f / gPos.pbRate);
        float posSamples = gPos.pitch.posRatio * mFileBuffer->getNumSamples();
        float posOffset = juce::jmap(
            random.nextFloat(), 0.0f,
            (gPos.pitch.duration * mFileBuffer->getNumSamples()) - durSamples);

        float gain = gPos.ampEnvLevel * params.gain;
        auto grain = Grain(mGaussianEnv, durSamples, gPos.pbRate,
                           posSamples + posOffset, mTotalSamps, gain);
        mGrains.add(grain);
        
        // Update amplitude envelope for position that just played
        float attackSamples =
            mSampleRate *
            juce::jmap(params.attack, MIN_ATTACK_SEC, MAX_ATTACK_SEC);
        float decaySamples =
            mSampleRate *
            juce::jmap(params.decay, MIN_DECAY_SEC, MAX_DECAY_SEC);
        switch (gPos.envState) { 
          case Utils::EnvelopeState::ATTACK: {
            gPos.ampEnvLevel = juce::jlimit(0.0f, 1.0f, (mTotalSamps - gNote.noteOnTs) / (float)attackSamples);
            if (gPos.ampEnvLevel >= 1.0f) {
              gPos.envState = Utils::EnvelopeState::DECAY;
            }
            break;
          }
          case Utils::EnvelopeState::DECAY: {
            gPos.ampEnvLevel = juce::jlimit(
                params.sustain, 1.0f,
                1.0f - ((mTotalSamps - attackSamples - gNote.noteOnTs) /
                        (float)(decaySamples)) *
                           (1.0f - params.sustain));
            if (gPos.ampEnvLevel <= params.sustain) {
              gPos.envState = Utils::EnvelopeState::SUSTAIN;
            }
            break;
          }
          case Utils::EnvelopeState::SUSTAIN: {
            gPos.ampEnvLevel = params.sustain;
            // Note: setting note off sets state to release, we don't need to
            // here
            break;
          }
          case Utils::EnvelopeState::RELEASE: {
            float releaseSamples =
                mSampleRate *
                juce::jmap(params.release, MIN_RELEASE_SEC, MAX_RELEASE_SEC);
            gPos.ampEnvLevel = juce::jlimit(
                0.0f, params.sustain,
                params.sustain * (1.0f - (mTotalSamps - gNote.noteOffTs) /
                                             (float)releaseSamples));
            break;
          }
        }
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
  if (mActiveNotes.isEmpty() && mGrains.isEmpty()) {
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

  // Delete expired notes
  long maxReleaseTime = getMaxReleaseTime();
  mActiveNotes.removeIf([this, maxReleaseTime](GrainNote& gNote) {
    return (mTotalSamps - gNote.noteOffTs) > maxReleaseTime && gNote.noteOffTs > 0;
  });
}

void GranularSynth::setFileBuffer(juce::AudioBuffer<float>* buffer, double sr) {
  mFileBuffer = buffer;
  mSampleRate = sr;
}

void GranularSynth::setNoteOn(
    PitchDetector::PitchClass pitchClass,
    std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  bool isPlayingAlready = false;
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == pitchClass) {
      isPlayingAlready = true;
      gNote.positions = gPositions;
      gNote.noteOnTs = mTotalSamps;
      gNote.noteOffTs = -1;
    }
  }
  if (!isPlayingAlready) {
    mActiveNotes.add(GrainNote(pitchClass, gPositions, mTotalSamps));
  }
}

void GranularSynth::setNoteOff(PitchDetector::PitchClass pitchClass) {
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == pitchClass) {
      // Set note off timestamp and set env state to release for each pos
      gNote.noteOffTs = mTotalSamps;
      for (int i = 0; i < gNote.positions.size(); ++i) {
        gNote.positions[i].envState = Utils::EnvelopeState::RELEASE;
      }
      break;
    }
  }
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
    case ParameterType::ATTACK:
      mPositionSettings[colour].attack = value;
      break;
    case ParameterType::DECAY:
      mPositionSettings[colour].decay = value;
      break;
    case ParameterType::SUSTAIN:
      mPositionSettings[colour].sustain = value;
      break;
    case ParameterType::RELEASE:
      mPositionSettings[colour].release = value;
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

long GranularSynth::getMaxReleaseTime() {
  int curMaxSamples = 0;
  for (int i = 0; i < mPositionSettings.size(); ++i) {
    int releaseSamples = mSampleRate * juce::jmap(mPositionSettings[i].release,
                                                  MIN_RELEASE_SEC,
                                                  MAX_RELEASE_SEC);
    if (releaseSamples > curMaxSamples) {
      curMaxSamples = releaseSamples;
    }
  }
  return curMaxSamples;
}