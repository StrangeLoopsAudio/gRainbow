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
  generateGaussianEnvelope();
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

    // Reset timestamps if no grains active to keep numbers low
    if (mGrains.isEmpty()) {
      mTotalSamps = 0;
      mNextGrainTs = 0;
    }

    if (mFileBuffer != nullptr && mTotalSamps >= mNextGrainTs)
    {
      mNextGrainTs = mTotalSamps + (mRate * MAX_RATE);
      /* Add one grain per active note */
      for (GrainNote &note : mActiveNotes)
      {
        auto durSamples = (mDuration * MAX_DURATION) * mSampleRate;
        mGrains.add(Grain(mGaussianEnv, durSamples, note.positionRatios[note.curPos++] * mFileBuffer->getNumSamples(), mTotalSamps));
        note.curPos = note.curPos % note.positionRatios.size();
      }
    }
    wait(20);
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
  std::vector<GrainPosition> grainPositions;
  if (mFftData == nullptr) return std::vector<float>();
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
      float maxVal = 0;
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
        grainPositions.push_back(GrainPosition((float)i / mFftData->size(), maxVal));
      }
    }
    if (grainPositions.size() >= k) foundK = true;
    numSearches++;
    if (numSearches > 5) break;
  }

  if (grainPositions.empty()) return std::vector<float>();

  if (grainPositions.size() > k)
  {

    std::sort(grainPositions.begin(), grainPositions.end());
    grainPositions = std::vector<GrainPosition>(grainPositions.begin() + grainPositions.size() - k, grainPositions.end());
  }

  std::vector<float> positions = getPositionFloats(grainPositions);

  mActiveNotes.add(GrainNote(midiNote, positions));
  
  return positions;
}

void GranularSynth::stopNote(int midiNote)
{
  mActiveNotes.removeIf([midiNote](GrainNote& note)
    {
      return note.midiNote == midiNote;
    });
}

std::vector<float> GranularSynth::getPositionFloats(std::vector<GrainPosition> gPositions)
{
  std::vector<float> positions;
  for (GrainPosition gPos : gPositions)
  {
    positions.push_back(gPos.posRatio);
  }
  return positions;
}

void GranularSynth::generateGaussianEnvelope()
{
  for (int i = 0; i < mGaussianEnv.size(); i++)
  {
    mGaussianEnv[i] = std::exp(-0.5f * std::pow((i - ((511) / 2.0)) / (0.4 * ((511) / 2.0)), 2.0));
  }
}