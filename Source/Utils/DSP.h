#pragma once

#include "juce_audio_basics/juce_audio_basics.h"

namespace Utils {

// 2D vector container for spectra
typedef std::vector<std::vector<float>> SpecBuffer;

// Audio buffer processing
static void resampleAudioBuffer(juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer,
                                        double inputSampleRate, double outputSampleRate, bool clearInput = false) {
  const double ratioToInput = inputSampleRate / outputSampleRate;   // input / output
  const double ratioToOutput = outputSampleRate / inputSampleRate;  // output / input
  // The output buffer needs to be size that matches the new sample rate
  const int resampleSize = static_cast<int>(static_cast<double>(inputBuffer.getNumSamples()) * ratioToOutput);
  outputBuffer.setSize(inputBuffer.getNumChannels(), resampleSize);
  
  const float* const* inputs = inputBuffer.getArrayOfReadPointers();
  float* const* outputs = outputBuffer.getArrayOfWritePointers();
  
  std::unique_ptr<juce::LagrangeInterpolator> resampler = std::make_unique<juce::LagrangeInterpolator>();
  for (int c = 0; c < outputBuffer.getNumChannels(); c++) {
    resampler->reset();
    resampler->process(ratioToInput, inputs[c], outputs[c], outputBuffer.getNumSamples());
  }
  if (clearInput) inputBuffer.setSize(1, 1);
}

static void trimAudioBuffer(juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer,
                                    juce::Range<juce::int64> range, bool clearInput = false) {
  if (range.isEmpty()) {
    outputBuffer.setSize(inputBuffer.getNumChannels(), inputBuffer.getNumSamples());
    outputBuffer.makeCopyOf(inputBuffer);
  } else {
    juce::AudioBuffer<float> fileAudioBuffer;
    const int sampleLength = static_cast<int>(range.getLength());
    outputBuffer.setSize(inputBuffer.getNumChannels(), sampleLength);
    for (int c = 0; c < inputBuffer.getNumChannels(); c++) {
      outputBuffer.copyFrom(c, 0, inputBuffer, c, range.getStart(), sampleLength);
    }
  }
  if (clearInput) inputBuffer.setSize(1, 1);
}



}  // namespace Utils
