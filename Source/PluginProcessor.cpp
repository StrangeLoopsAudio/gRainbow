/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "PluginEditor.h"

//==============================================================================
GRainbowAudioProcessor::GRainbowAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
}

GRainbowAudioProcessor::~GRainbowAudioProcessor() {}

//==============================================================================
const juce::String GRainbowAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool GRainbowAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool GRainbowAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool GRainbowAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double GRainbowAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int GRainbowAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int GRainbowAudioProcessor::getCurrentProgram() { return 0; }

void GRainbowAudioProcessor::setCurrentProgram(int index) {}

const juce::String GRainbowAudioProcessor::getProgramName(int index) {
  return {};
}

void GRainbowAudioProcessor::changeProgramName(int index,
                                               const juce::String& newName) {}

//==============================================================================
void GRainbowAudioProcessor::prepareToPlay(double sampleRate,
                                           int samplesPerBlock) {
  mSampleRate = sampleRate;
}

void GRainbowAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GRainbowAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void GRainbowAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // Fill midi buffer with UI keyboard events
  juce::MidiBuffer aggregatedMidiBuffer;
  mKeyboardState.processNextMidiBuffer(aggregatedMidiBuffer, 0, buffer.getNumSamples(),
                                       true);

  // Add midi events from native buffer
  aggregatedMidiBuffer.addEvents(midiMessages, 0, buffer.getNumSamples(), 0);
  if (!aggregatedMidiBuffer.isEmpty()) {
    for (juce::MidiMessageMetadata md : aggregatedMidiBuffer) {
      // Trigger note on/off depending on event type
      Utils::PitchClass pc = (Utils::PitchClass)(
            md.getMessage().getNoteNumber() % Utils::PitchClass::COUNT);
      if (md.getMessage().isNoteOn()) {
        if (onNoteChanged != nullptr) onNoteChanged(pc, true);
        synth.setNoteOn(pc);
      } else if (md.getMessage().isNoteOff()) {
        if (onNoteChanged != nullptr) onNoteChanged(pc, false);
        synth.setNoteOff(pc);
      }
    }
  }

  synth.process(buffer);
}

//==============================================================================
bool GRainbowAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GRainbowAudioProcessor::createEditor() {
  return new GRainbowAudioProcessorEditor(*this);
}

//==============================================================================
void GRainbowAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void GRainbowAudioProcessor::setStateInformation(const void* data,
                                                 int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new GRainbowAudioProcessor();
}
