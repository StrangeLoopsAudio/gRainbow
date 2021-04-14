/*
  ==============================================================================

    GranularSynth.cpp
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#include "GranularSynth.h"

GranularSynth::GranularSynth(juce::MidiKeyboardState& midiState) : mMidiState(midiState), juce::Thread("granular thread")
{
  mTotalSamps = 0;
  startThread();
}

GranularSynth::~GranularSynth()
{
  stopThread(4000);
}

void GranularSynth::run()
{
  while (!threadShouldExit())
  {
    /* Delete expired grains */
    for (int i = mGrains.size() - 1; i >= 0; --i)
    {
      if (mTotalSamps > (mGrains[i].trigTs + mGrains[i].duration))
      {
        mGrains.remove(i);
      }
    }

    if (mFileBuffer != nullptr)
    {
      for (GrainNote &note : mActiveNotes)
      {
        auto durSamples = (mDuration * MAX_DURATION) * mSampleRate;
        mGrains.add(Grain(durSamples, note.positionRatios[note.curPos++] * mFileBuffer->getNumSamples(), mTotalSamps));
        note.curPos = note.curPos % note.positionRatios.size();
      }
    }
    wait((mRate * MAX_RATE) + 20);
  }
}

void GranularSynth::process(juce::AudioBuffer<float>* blockBuffer)
{
  for (int i = 0; i < blockBuffer->getNumSamples(); ++i)
  {
    for (int g = 0; g < mGrains.size(); ++g)
    {
      mGrains[g].process(*mFileBuffer, *blockBuffer, mTotalSamps);
    }
    mTotalSamps++;
  }
}

void GranularSynth::setFileBuffer(juce::AudioBuffer<float>* buffer, std::vector<std::vector<float>>* fftData)
{
  mFileBuffer = buffer;
  mFftData = fftData;
}

std::vector<float> GranularSynth::playNote(int midiNote)
{
  std::vector<float> foundPositions;
  if (mFftData == nullptr) return foundPositions;
  int k = (mDiversity * MAX_DIVERSITY) + 1;
  // look for times when frequency has high energy
  float noteFreq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
  bool foundK = false;
  int numSearches = 1;
  while (!foundK)
  {
    float variance = (noteFreq - juce::MidiMessage::getMidiNoteInHertz(midiNote - numSearches)) / 2.0f;
    juce::Range<float> freqRange = juce::Range<float>(noteFreq - variance, noteFreq + variance);
    for (int i = 0; i < mFftData->size(); ++i)
    {
      int maxVal = 0;
      int maxIndex = 0;
      std::vector<float> curCol = mFftData->at(i);
      for (int j = 0; j < curCol.size() / 4; ++j)
      {
        if (curCol[j] > maxVal)
        {
          maxVal = curCol[j];
          maxIndex = j;
        }
      }
      float maxFreq = (maxIndex * mSampleRate) / (curCol.size() / 2.0f);
      if (freqRange.contains(maxFreq))
      {
        foundPositions.push_back((float)i / mFftData->size());
      }
    }
    if (foundPositions.size() >= k) foundK = true;
    numSearches++;
    if (numSearches > 5) break;
  }

  if (foundPositions.size() > k)
  {
    std::sort(foundPositions.begin(), foundPositions.end());
    foundPositions = std::vector<float>(foundPositions.begin() + foundPositions.size() - k, foundPositions.end());
  }

  mActiveNotes.add(GrainNote(midiNote, foundPositions));
  
  return foundPositions;
}

void GranularSynth::stopNote(int midiNote)
{
  mActiveNotes.removeIf([midiNote](GrainNote& note)
    {
      return note.midiNote == midiNote;
    });
}