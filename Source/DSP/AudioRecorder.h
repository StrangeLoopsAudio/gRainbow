/*
  ==============================================================================

    AudioRecorder.h
    Created: 22 Jun 2021 8:15:10pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include "juce_audio_devices/juce_audio_devices.h"

//==============================================================================
/** A simple class that acts as an AudioIODeviceCallback and writes the
    incoming audio data to a WAV file.
*/
class AudioRecorder : public juce::AudioIODeviceCallback {
 public:
  AudioRecorder() { mBackgroundThread.startThread(); }

  ~AudioRecorder() override { stop(); }

  std::function<void(double progress)> onBlockAdded = nullptr;

  //==============================================================================
  void startRecording(const juce::File& file);

  void stop();

  bool isRecording() const;

  //==============================================================================
  void audioDeviceAboutToStart(juce::AudioIODevice* device) override;

  void audioDeviceStopped() override;

  void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels, float* const* outputChannelData,
                                        int numOutputChannels, int numSamples,
                                        const juce::AudioIODeviceCallbackContext& context) override;

 private:
  juce::TimeSliceThread mBackgroundThread{"Audio Recorder Thread"};          // the thread that will write our audio data to
                                                                             // disk
  std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> mThreadedWriter;  // the FIFO used to buffer the incoming data
  double mSampleRate = 0.0;

  juce::CriticalSection mWriterLock;
  std::atomic<juce::AudioFormatWriter::ThreadedWriter*> mActiveWriter{nullptr};
};