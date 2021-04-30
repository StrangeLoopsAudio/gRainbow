/*
  ==============================================================================

    GrainPositionFinder.cpp
    Created: 28 Apr 2021 8:55:40pm
    Author:  brady

  ==============================================================================
*/

#include "GrainPositionFinder.h"

std::vector<GrainPositionFinder::GrainPosition>
GrainPositionFinder::findPositions(int k, int midiNote) {
  std::vector<GrainPosition> grainPositions;
  if (mPitches == nullptr) return grainPositions;
  if (mGPositions[midiNote].size() >= k) return mGPositions[midiNote];
  // look for times when frequency has high energy
  float noteFreq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
  bool foundK = false;
  int numSearches = 0;
  while (!foundK) {
    int noteMin = midiNote - numSearches;
    int noteMax = midiNote + numSearches;
    float varianceMin = (juce::MidiMessage::getMidiNoteInHertz(noteMin) -
                         juce::MidiMessage::getMidiNoteInHertz(noteMin - 1)) /
                        2.0f;
    float varianceMax = (juce::MidiMessage::getMidiNoteInHertz(noteMax) -
                         juce::MidiMessage::getMidiNoteInHertz(noteMax - 1)) /
                        2.0f;
    juce::Range<float> freqRangeMin = juce::Range<float>(
        juce::MidiMessage::getMidiNoteInHertz(noteMin) - varianceMin,
        juce::MidiMessage::getMidiNoteInHertz(noteMin) + varianceMin);
    juce::Range<float> freqRangeMax = juce::Range<float>(
        juce::MidiMessage::getMidiNoteInHertz(noteMax) - varianceMax,
        juce::MidiMessage::getMidiNoteInHertz(noteMax) + varianceMax);
    for (int i = 0; i < mPitches->size(); ++i) {
      float pitch = mPitches->at(i).freq;
      if (freqRangeMin.contains(pitch) || freqRangeMax.contains(pitch)) {
        int semiOffset = numSearches;
        float freqDiff;
        if (pitch > noteFreq) {
          semiOffset *= -1;
          freqDiff = juce::jmap(pitch, freqRangeMax.getStart(),
                                freqRangeMax.getEnd(), -0.5f, 0.5f);
        } else {
          freqDiff = juce::jmap(pitch, freqRangeMin.getStart(),
                                freqRangeMin.getEnd(), -0.5f, 0.5f);
        }
        freqDiff *= -1.0f;

        float pbRate = std::pow(TIMESTRETCH_RATIO, semiOffset + freqDiff);
        grainPositions.push_back(GrainPosition(mPitches->at(i).posRatio, pbRate,
                                               mPitches->at(i).gain));
      }
    }
    if (grainPositions.size() >= k) foundK = true;
    numSearches++;
    if (numSearches > 5) break;
  }

  if (grainPositions.empty()) return grainPositions;

  if (grainPositions.size() > k) {
    std::sort(grainPositions.begin(), grainPositions.end());
    grainPositions = std::vector<GrainPosition>(
        grainPositions.begin() + grainPositions.size() - k,
        grainPositions.end());
  }

  pushPositions(midiNote, grainPositions);
  return grainPositions;
}

void GrainPositionFinder::pushPositions(
    int midiNote, std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  if (mGPositions[midiNote].size() == 0) {
    mGPositions.set(midiNote, gPositions);
  } else {
    mGPositions.getReference(midiNote).insert(
        mGPositions.getReference(midiNote).end(), gPositions.begin(), gPositions.end());
  }
}

void GrainPositionFinder::updatePosition(int midiNote,
  GrainPositionFinder::GrainPosition gPos) {
  auto notePositions = mGPositions[midiNote];
  for (int i = 0; i < notePositions.size(); ++i) {
    if (notePositions[i].posRatio == gPos.posRatio) {
      notePositions[i] = gPos;
    }
  }
  mGPositions.set(midiNote, notePositions);
}