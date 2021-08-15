/*
  ==============================================================================

    AudioRecorder.cpp
    Created: 22 Jun 2021 8:15:10pm
    Author:  brady

  ==============================================================================
*/

#include "AudioRecorder.h"

void AudioRecorder::startRecording(const juce::File& file) {
  stop();

  if (mSampleRate > 0) {
    // Create an OutputStream to write to our destination file...
    file.deleteFile();

    if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream())) {
      // Now create a WAV writer object that writes to our output stream...
      juce::WavAudioFormat wavFormat;

      if (auto writer = wavFormat.createWriterFor(fileStream.get(), mSampleRate, 1, 16, {}, 0)) {
        fileStream.release();  // (passes responsibility for deleting the stream
                               // to the writer object that is now using it)

        // Now we'll create one of these helper objects which will act as a
        // FIFO buffer, and will write the data to disk on our background
        // thread.
        mThreadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer, mBackgroundThread, 32768));

        // And now, swap over our active writer pointer so that the audio
        // callback will start using it..
        const juce::ScopedLock sl(mWriterLock);
        mActiveWriter = mThreadedWriter.get();
      }
    }
  }
}

void AudioRecorder::stop() {
  // First, clear this pointer to stop the audio callback from using our
  // writer object..
  {
    const juce::ScopedLock sl(mWriterLock);
    mActiveWriter = nullptr;
  }

  // Now we can delete the writer object. It's done in this order because the
  // deletion could take a little time while remaining data gets flushed to
  // disk, so it's best to avoid blocking the audio callback while this
  // happens.
  mThreadedWriter.reset();
}

bool AudioRecorder::isRecording() const { return mActiveWriter.load() != nullptr; }

//==============================================================================
void AudioRecorder::audioDeviceAboutToStart(juce::AudioIODevice* device) { mSampleRate = device->getCurrentSampleRate(); }

void AudioRecorder::audioDeviceStopped() { mSampleRate = 0; }

void AudioRecorder::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels, float** outputChannelData,
                                          int numOutputChannels, int numSamples) {
  const juce::ScopedLock sl(mWriterLock);

  if (mActiveWriter.load() != nullptr) {
    mActiveWriter.load()->write(inputChannelData, numSamples);
  }

  // We need to clear the output buffers, in case they're full of junk..
  for (int i = 0; i < numOutputChannels; ++i)
    if (outputChannelData[i] != nullptr) juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
}