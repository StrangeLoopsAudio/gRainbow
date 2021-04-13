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

    double testDuration = 1.0;

    if (mFileBuffer != nullptr && mShouldPlayTest/*&& mActiveNotes.size() > 0*/)
    {
      auto durSamples = testDuration * mSampleRate;
      mGrains.add(Grain(durSamples, 0.1 * mFileBuffer->getNumSamples(), mTotalSamps));
    }
    wait(testDuration * 1000);
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
  /*if (mFftData == nullptr) return;
  for (int i = 0; i < fftData->size(); ++i)
  {
    float curMax = 0.0f;
    int maxIndex = 0;
    std::vector<float> curCol = fftData->at(i);
    for (int j = 0; j < curCol.size(); ++j)
    {
      if (curCol[j] > curMax)
      {
        curMax = curCol[j];
        maxIndex = j;
      }
    }
  }*/
}

std::vector<int> GranularSynth::playNote(int midiNote, int k)
{
  std::vector<int> foundPositions;
  if (mFftData == nullptr) return foundPositions;
  // look for times when frequency has high energy
  float noteFreq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
  float variance = (noteFreq - juce::MidiMessage::getMidiNoteInHertz(midiNote - 1)) / 2.0f;
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
      DBG("Looking for freq: " << noteFreq << ", found freq: " << maxFreq << " at time index: " << i);
      foundPositions.push_back(i);
    }
  }
  if (foundPositions.size() > k)
  {
    std::sort(foundPositions.begin(), foundPositions.end());
    return std::vector<int>(foundPositions.begin() + foundPositions.size() - k, foundPositions.end());
  }
  
  return foundPositions;
}