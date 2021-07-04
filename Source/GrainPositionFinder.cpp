/*
  ==============================================================================

    GrainPositionFinder.cpp
    Created: 28 Apr 2021 8:55:40pm
    Author:  brady

  ==============================================================================
*/

#include "GrainPositionFinder.h"

std::vector<GrainPositionFinder::GrainPosition>
GrainPositionFinder::findPositions(int numPosToFind, PitchDetector::PitchClass pitchClass) {
  std::vector<GrainPosition> grainPositions;
  if (mPitches == nullptr) return grainPositions;

  // TODO: only return numPosToFind positions below
  if (mGPositions[pitchClass].size() > 0) return mGPositions[pitchClass];

  // Look for detected pitches with correct pitch and good gain
  bool foundAll = false;
  int numSearches = 0;

  while (!foundAll) {
    int noteMin = pitchClass - numSearches;
    int noteMax = pitchClass + numSearches;
    // Check low note
    std::vector<PitchDetector::Pitch> &pitchVec =
        mPitches->getReference((PitchDetector::PitchClass)(noteMin % 12));
    float pbRate = std::pow(TIMESTRETCH_RATIO, numSearches);
    for (int i = 0; i < pitchVec.size(); ++i) {
      grainPositions.push_back(GrainPosition(pitchVec[i], pbRate));
      if (grainPositions.size() >= numPosToFind) {
        foundAll = true;
        break;
      }
    }
    // Check high note if we haven't filled up the list yet
    if (!foundAll && numSearches > 0) {
      std::vector<PitchDetector::Pitch> &pitchVec =
          mPitches->getReference((PitchDetector::PitchClass)(noteMax % 12));
      float pbRate = std::pow(TIMESTRETCH_RATIO, -numSearches);
      for (int i = 0; i < pitchVec.size(); ++i) {
        grainPositions.push_back(GrainPosition(pitchVec[i], pbRate));
        if (grainPositions.size() >= numPosToFind) {
          foundAll = true;
          break;
        }
      }
    }
    numSearches++;
    if (numSearches >= 11 || foundAll) break;
  }

  pushPositions(pitchClass, grainPositions);

  return grainPositions;
}

void GrainPositionFinder::pushPositions(
    PitchDetector::PitchClass pitchClass,
    std::vector<GrainPositionFinder::GrainPosition> gPositions) {
  if (mGPositions[pitchClass].size() == 0) {
    mGPositions.set(pitchClass, gPositions);
  } else {
    mGPositions.getReference(pitchClass)
        .insert(mGPositions.getReference(pitchClass).end(), gPositions.begin(),
                gPositions.end());
  }
}

void GrainPositionFinder::updatePosition(int midiNote,
  GrainPositionFinder::GrainPosition gPos) {
  auto notePositions = mGPositions[midiNote];
  for (int i = 0; i < notePositions.size(); ++i) {
    if (notePositions[i].pitch.posRatio == gPos.pitch.posRatio) {
      notePositions[i] = gPos;
    }
  }
  mGPositions.set(midiNote, notePositions);
}