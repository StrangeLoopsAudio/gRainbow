/*
  ==============================================================================

    GranularSynth.cpp
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "GranularSynth.h"
#include "Preset.h"
#include "PluginEditor.h"
#include "Components/Settings.h"

GranularSynth::GranularSynth()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         )
#endif
      ,
      // only care about tracking the processing of the DSP, not the spectrogram
      mFft(FFT_SIZE, HOP_SIZE, 0, 0),
      mPitchDetector(0.01, 1.0) {
  mParameters.note.addParams(*this);
  mParameters.global.addParams(*this);

  mTotalSamps = 0;
  mProcessedSpecs.fill(nullptr);

  mKeyboardState.addListener(this);

  mFormatManager.registerBasicFormats();

  mFft.onProcessingComplete = [this](Utils::SpecBuffer& spectrum) {
    mProcessedSpecs[ParamUI::SpecType::SPECTROGRAM] = &spectrum;
    mFft.clear(false);
  };

  mPitchDetector.onHarmonicProfileReady = [this](Utils::SpecBuffer& hpcpBuffer) {
    mProcessedSpecs[ParamUI::SpecType::HPCP] = &hpcpBuffer;
  };

  mPitchDetector.onPitchesReady = [this](PitchDetector::PitchMap& pitchMap, Utils::SpecBuffer& pitchSpec) {
    mProcessedSpecs[ParamUI::SpecType::DETECTED] = &pitchSpec;
    createCandidates(pitchMap);
    mPitchDetector.clear();
  };

  mPitchDetector.onProgressUpdated = [this](float progress) { mLoadingProgress = progress; };

  resetParameters();
}

GranularSynth::~GranularSynth() {}

//==============================================================================
const juce::String GranularSynth::getName() const { return JucePlugin_Name; }

bool GranularSynth::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool GranularSynth::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  // Because we are allowing the mouse to inject midi input,
  // this should be true, otherwise we assert in VST3 code
  return true;
#else
  return false;
#endif
}

bool GranularSynth::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double GranularSynth::getTailLengthSeconds() const { return 0.0; }

int GranularSynth::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int GranularSynth::getCurrentProgram() { return 0; }

void GranularSynth::setCurrentProgram(int) {}

const juce::String GranularSynth::getProgramName(int) { return {}; }

void GranularSynth::changeProgramName(int, const juce::String&) {}

//==============================================================================
void GranularSynth::prepareToPlay(double sampleRate, int samplesPerBlock) {
    if (mNeedsResample) {
    // File loaded from state but couldn't resample and trim until now
    // Make a temporary buffer copy for resampling
    juce::AudioSampleBuffer inputBuffer = mInputBuffer;
    resampleAudioBuffer(inputBuffer, mInputBuffer, mSampleRate, sampleRate);

    // Convert time to sample range
    const double sampleLength = static_cast<double>(mInputBuffer.getNumSamples());
    const double secondLength = sampleLength / sampleRate;
    juce::int64 start = static_cast<juce::int64>(sampleLength * (mParameters.ui.trimRange.getStart() / secondLength));
    juce::int64 end = static_cast<juce::int64>(sampleLength * (mParameters.ui.trimRange.getEnd() / secondLength));
    trimAudioBuffer(mInputBuffer, mAudioBuffer, juce::Range<juce::int64>(start, end));
    mInputBuffer.clear();
    mLoadingProgress = 1.0f;
    mNeedsResample = false;
  }

  mSampleRate = sampleRate;

  const juce::dsp::ProcessSpec filtConfig = {sampleRate, (juce::uint32)samplesPerBlock, (unsigned int)getTotalNumOutputChannels()};
  mParameters.global.filter.prepare(filtConfig);
  for (auto&& note : mParameters.note.notes) {
    note->filter.prepare(filtConfig);
    for (auto&& gen : note->generators) {
      gen->filter.prepare(filtConfig);
      gen->sampleRate = sampleRate;
    }
  }
  mMeterSource.resize(getTotalNumOutputChannels(), sampleRate * 0.1 / samplesPerBlock);
}

void GranularSynth::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GranularSynth::isBusesLayoutSupported(const BusesLayout& layouts) const {
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
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) return false;
#endif

  return true;
#endif
}
#endif

void GranularSynth::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  const int bufferNumSample = buffer.getNumSamples();

  mKeyboardState.processNextMidiBuffer(midiMessages, 0, bufferNumSample, true);

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
    buffer.clear(i, 0, bufferNumSample);
  }

  if (mParameters.ui.trimPlaybackOn) {
    int numSample = bufferNumSample;
    if (mParameters.ui.trimPlaybackSample + bufferNumSample >= mParameters.ui.trimPlaybackMaxSample) {
      mParameters.ui.trimPlaybackOn = false;
      numSample = mParameters.ui.trimPlaybackMaxSample - mParameters.ui.trimPlaybackSample;
    }

    // if output buffer is stereo and the input in mono, duplicate into both channels
    // if output buffer is mono and the input in stereo, just play one channel for simplicity
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
      const int inputChannel = juce::jmin(ch, mInputBuffer.getNumChannels() - 1);
      buffer.copyFrom(ch, 0, mInputBuffer, inputChannel, mParameters.ui.trimPlaybackSample, numSample);
    }
    mParameters.ui.trimPlaybackSample += numSample;
  }

  // Add contributions from each note
  auto bufferChannels = buffer.getArrayOfWritePointers();
  for (int i = 0; i < bufferNumSample; ++i) {
    // Don't use a for(auto x : mActiveNotes) loop here as mActiveNotes can be added outside this function. If it is partially added
    // it might to use it and the undefined data will cause a crash eventually
    const int activeNoteSize = mActiveNotes.size();
    for (int noteIndex = 0; noteIndex < activeNoteSize; noteIndex++) {
      GrainNote* gNote = &mActiveNotes.getReference(noteIndex);

      // Add contributions from the grains in this generator

      for (size_t genIdx = 0; genIdx < NUM_GENERATORS; ++genIdx) {
        ParamGenerator* paramGenerator = mParameters.note.notes[gNote->pitchClass]->generators[genIdx].get();
        const float gain = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GAIN);
        const float attack = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::ATTACK);
        const float decay = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::DECAY);
        const float sustain = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::SUSTAIN);
        const float release = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::RELEASE);
        const float grainGain =
            gNote->genAmpEnvs[genIdx].getAmplitude(mTotalSamps, attack * mSampleRate, decay * mSampleRate, sustain,
                                                  release * mSampleRate) * gain;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
          float genSample = 0.0f;
          for (Grain& grain : gNote->genGrains[genIdx]) {
            genSample += grain.process(ch / (float)(buffer.getNumChannels() - 1), mAudioBuffer, grainGain, mTotalSamps);
          }
          // Process filter and optionally use for output
          const float filterOutput = mParameters.getFilterOutput(paramGenerator, ch, genSample);
          // If filter type isn't "none", use its output
          const int filtType = mParameters.getChoiceParam(paramGenerator, ParamCommon::Type::FILT_TYPE);
          if (filtType != Utils::FilterType::NO_FILTER) {
            genSample = filterOutput;
          }

          bufferChannels[ch][i] += genSample;
        }
      }
    }
    mTotalSamps++;
  }

  // Clip buffers to valid range
  for (int i = 0; i < buffer.getNumChannels(); i++) {
    juce::FloatVectorOperations::clip(buffer.getWritePointer(i), buffer.getReadPointer(i), -1.0f, 1.0f, bufferNumSample);
  }

  handleGrainAddRemove(bufferNumSample);

  // Reset timestamps if no grains active to keep numbers low
  if (mActiveNotes.isEmpty()) {
    mTotalSamps = 0;
  } else {
    // Normalize the block before sending onward
    // if grains is empty, don't want to divide by zero
    /*for (int i = 0; i < bufferNumSample; ++i) {
      for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
         float* channelBlock = buffer.getWritePointer(ch);
         channelBlock[i] /= mGrains.size();
      }
    } */
  }

  mMeterSource.measureBlock(buffer);
}

//==============================================================================
bool GranularSynth::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GranularSynth::createEditor() {
  juce::AudioProcessorEditor* editor = new GRainbowAudioProcessorEditor(*this);

  // Init anything in main editor post construction
  reinterpret_cast<GRainbowAudioProcessorEditor*>(editor)->fastDebugMode();

  return editor;
}

//==============================================================================
void GranularSynth::getStateInformation(juce::MemoryBlock& destData) {
  juce::XmlElement xml("PluginState");

  juce::XmlElement* params = new juce::XmlElement("AudioParams");
  for (auto& param : getParameters()) {
    params->setAttribute(ParamHelper::getParamID(param), param->getValue());
  }

  xml.addChildElement(params);
  xml.addChildElement(mParameters.note.getXml());
  xml.addChildElement(mParameters.ui.getXml());

  copyXmlToBinary(xml, destData);
}

void GranularSynth::setStateInformation(const void* data, int sizeInBytes) {
  auto xml = getXmlFromBinary(data, sizeInBytes);

  if (xml != nullptr) {
    // Set audio parameters
    auto params = xml->getChildByName("AudioParams");
    if (params != nullptr) {
      for (auto& param : getParameters()) {
        param->setValueNotifyingHost(params->getDoubleAttribute(ParamHelper::getParamID(param), param->getValue()));
      }
    }

    // Load candidates
    params = xml->getChildByName("NotesParams");
    if (params != nullptr) {
      mParameters.note.setXml(params);
    }

    // Set UI state
    params = xml->getChildByName("ParamUI");
    if (params != nullptr) {
      mParameters.ui.setXml(params);
    }

    // Load the file if we haven't yet
    if (mAudioBuffer.getNumSamples() == 0 && mParameters.ui.loadedFileName.isNotEmpty()) {
      juce::File file = juce::File(mParameters.ui.loadedFileName);
      if (file.getFileExtension() == ".gbow") loadPreset(file);
      else loadAudioFile(juce::File(mParameters.ui.loadedFileName), false);
    }
  }
}

// These are slightly different then the get/setStateInformation. These are for
// 'user params' which include items that are related to audio, but not actually
// juce::AudioParam items that the DAW can use
void GranularSynth::getPresetParamsXml(juce::MemoryBlock& destData) {
  juce::XmlElement xml("UserState");

  juce::XmlElement* audioParams = new juce::XmlElement("AudioParams");
  for (auto& param : getParameters()) {
    audioParams->setAttribute(ParamHelper::getParamID(param), param->getValue());
  }
  xml.addChildElement(audioParams);
  xml.addChildElement(mParameters.note.getXml());
  xml.addChildElement(mParameters.ui.getXml());

  copyXmlToBinary(xml, destData);
}

void GranularSynth::setPresetParamsXml(const void* data, int sizeInBytes) {
  auto xml = getXmlFromBinary(data, sizeInBytes);

  if (xml != nullptr) {
    auto params = xml->getChildByName("AudioParams");
    if (params != nullptr) {
      for (auto& param : getParameters()) {
        param->setValueNotifyingHost(params->getDoubleAttribute(ParamHelper::getParamID(param), param->getValue()));
      }
    }

    params = xml->getChildByName("NotesParams");
    if (params != nullptr) {
      mParameters.note.setXml(params);
    }

    params = xml->getChildByName("ParamUI");
    if (params != nullptr) {
      mParameters.ui.setXml(params);
    }
  }
}

//==============================================================================
// This creates new instances of the plugin.
// For all intents and purposes "main()" for standalone and/or any plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  // There is no known reason for pre-6.0.8 not being valid, just lack of
  // testing with older versions. Some older versions will not compile due to
  // missing JUCE functions
  if ((JUCE_MAJOR_VERSION < 6) || (JUCE_MAJOR_VERSION == 6 && JUCE_MINOR_VERSION == 0 && JUCE_BUILDNUMBER < 8)) {
    printf(
        "[ERROR] Using JUCE version %s but gRainbow requires JUCE 6.0.8 or "
        "greater\n",
        juce::SystemStats::getJUCEVersion().toRawUTF8());
    jassertfalse;
  }

  GranularSynth* synth = new GranularSynth();
  PowerUserSettings::get().setSynth(synth);
  return synth;
}

void GranularSynth::handleGrainAddRemove(int blockSize) {
  if (mLoadingProgress == 1.0) {
    // Add one grain per active note
    for (GrainNote& gNote : mActiveNotes) {
      for (size_t i = 0; i < gNote.grainTriggers.size(); ++i) {
        if (gNote.grainTriggers[i] <= 0) {
          ParamGenerator* paramGenerator = mParameters.note.notes[gNote.pitchClass]->generators[i].get();
          ParamCandidate* paramCandidate = mParameters.note.notes[gNote.pitchClass]->getCandidate(i);
          float durSec;
          const float gain = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GAIN);
          const float grainRate = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GRAIN_RATE);
          const float grainDuration = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GRAIN_DURATION);
          const bool grainSync = mParameters.getBoolParam(paramGenerator, ParamCommon::Type::GRAIN_SYNC);
          const float pitchAdjust = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::PITCH_ADJUST);
          const float pitchSpray = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::PITCH_SPRAY);
          const float posAdjust = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::POS_ADJUST);
          const float posSpray = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::POS_SPRAY);
          const float panAdjust = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::PAN_ADJUST);
          const float panSpray = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::PAN_SPRAY);
          const std::vector<float> grainEnv = mParameters.getGrainEnv(paramGenerator);

          if (grainSync) {
            float div = std::pow(2, (int)(ParamRanges::SYNC_DIV_MAX * ParamRanges::GRAIN_DURATION.convertTo0to1(grainDuration)));
            double bpm = DEFAULT_BPM;
            int beatsPerBar = 4;
            if (juce::AudioPlayHead* playhead = getPlayHead()) {
              juce::Optional<juce::AudioPlayHead::PositionInfo> info = playhead->getPosition();
              juce::Optional<double> newBpm = info->getBpm();
              if (newBpm) {
                bpm = *newBpm;
              }
              juce::Optional<juce::AudioPlayHead::TimeSignature> newTimeSignature = info->getTimeSignature();
              if (newTimeSignature) {
                beatsPerBar = (*newTimeSignature).numerator;
              }
            }
            // Find synced duration using bpm
            durSec = (1.0f / bpm) * 60.0f * (beatsPerBar / div);
          } else {
            durSec = grainDuration;
          }
          // Skip adding new grain if not enabled or full of grains
          if (paramCandidate != nullptr && mParameters.note.notes[gNote.pitchClass]->shouldPlayGenerator(i) &&
              gNote.genGrains.size() < MAX_GRAINS) {
            float durSamples = mSampleRate * durSec * (1.0f / paramCandidate->pbRate);
            /* Position calculation */
            juce::Random random;
            float posSprayOffset = juce::jmap(random.nextFloat(), ParamRanges::POSITION_SPRAY.start, posSpray) * mSampleRate;
            if (random.nextFloat() > 0.5f) posSprayOffset = -posSprayOffset;
            float posOffset = posAdjust * durSamples + posSprayOffset;
            float posSamples = paramCandidate->posRatio * mAudioBuffer.getNumSamples() + posOffset;

            /* Pan offset */
            float panSprayOffset = random.nextFloat() * panSpray;
            if (random.nextFloat() > 0.5f) panSprayOffset = -panSprayOffset;
            const float panOffset = juce::jlimit(ParamRanges::PAN_ADJUST.start, ParamRanges::PAN_ADJUST.end, panAdjust + panSprayOffset);

            /* Pitch calculation */
            float pitchSprayOffset = juce::jmap(random.nextFloat(), 0.0f, pitchSpray);
            if (random.nextFloat() > 0.5f) pitchSprayOffset = -pitchSprayOffset;
            float pbRate = paramCandidate->pbRate + pitchAdjust + pitchSprayOffset;
            jassert(paramCandidate->pbRate > 0.1f);

            /* Add grain */
            auto grain = Grain(grainEnv, durSamples, pbRate, posSamples, mTotalSamps, gain, panOffset);
            gNote.genGrains[i].add(grain);

            /* Trigger grain in arcspec */
            float totalGain = gain * gNote.genAmpEnvs[i].amplitude * gNote.velocity;
            mParameters.note.grainCreated(gNote.pitchClass, i, durSec / pbRate, totalGain);
          }
          // Reset trigger ts
          if (grainSync) {
            float div = std::pow(2, (int)(ParamRanges::SYNC_DIV_MAX * ParamRanges::GRAIN_RATE.convertTo0to1(grainRate)));
            // Find synced rate interval using bpm
            float intervalSamples = mSampleRate * durSec / div;
            gNote.grainTriggers[i] += intervalSamples;
          } else {
            gNote.grainTriggers[i] += mSampleRate * juce::jmap(ParamRanges::GRAIN_RATE.convertTo0to1(grainRate),
                                         durSec * MIN_RATE_RATIO, durSec * MAX_RATE_RATIO);
          }
        } else {
          gNote.grainTriggers[i] -= blockSize;
        }
      }
    }
  }
  // Delete expired grains
  for (GrainNote& gNote : mActiveNotes) {
    for (int genIdx = 0; genIdx < NUM_GENERATORS; ++genIdx) {
      gNote.genGrains[genIdx].removeIf([this](Grain& g) { return mTotalSamps > (g.trigTs + g.duration); });
    }
  }

  // Delete expired notes
  mActiveNotes.removeIf([this](GrainNote& gNote) { return gNote.removeTs != -1 && mTotalSamps >= gNote.removeTs; });
}

Utils::Result GranularSynth::loadAudioFile(juce::File file, bool process) {
  juce::AudioFormatReader* formatReader = mFormatManager.createReaderFor(file);
  if (formatReader == nullptr) return {false, "Opening failed: unsupported file format"};

  juce::AudioBuffer<float> fileAudioBuffer;
  const int length = static_cast<int>(formatReader->lengthInSamples);
  fileAudioBuffer.setSize(1, length);
  formatReader->read(&fileAudioBuffer, 0, length, 0, true, false);

  // .mp3 files, unlike .wav files, can contain PCM values greater than abs(1.0) (aka, clipping) which will produce aweful
  // sounding grains, so normalize the gain of any mp3 file clipping before using anywhere
  if (file.getFileExtension() == ".mp3") {
    float absMax = 0.0f;
    for (int i = 0; i < fileAudioBuffer.getNumChannels(); i++) {
      juce::Range<float> range =
          juce::FloatVectorOperations::findMinAndMax(fileAudioBuffer.getReadPointer(i), fileAudioBuffer.getNumSamples());
      absMax = juce::jmax(absMax, std::abs(range.getStart()), std::abs(range.getEnd()));
    }
    if (absMax > 1.0) {
      fileAudioBuffer.applyGain(1.0f / absMax);
    }
  }

  if (process) {
    resampleAudioBuffer(fileAudioBuffer, mInputBuffer, formatReader->sampleRate, mSampleRate);
    // processAudioBuffer() will be called after trimming in UI, so we don't have to do it here
  }
  else {
    if (mSampleRate != INVALID_SAMPLE_RATE) {
      resampleAudioBuffer(fileAudioBuffer, mAudioBuffer, formatReader->sampleRate, mSampleRate);
      mLoadingProgress = 1.0f;
    }
    else {
      mInputBuffer = fileAudioBuffer;  // Save for resampling once prepareToPlay() has been called
      mSampleRate = formatReader->sampleRate;  // A bit hacky, but we need to store the file's sample rate for resampling
      mNeedsResample = true;
    }
  }
  // should not need the file anymore as we want to work with the resampled input buffer across the rest of the plugin
  delete (formatReader);

  return {true, ""};
}

Utils::Result GranularSynth::loadPreset(juce::File file) {
  Preset::Header header;
  juce::FileInputStream input(file);
  if (input.openedOk()) {
    input.read(&header, sizeof(header));

    if (header.magic != Preset::MAGIC) {
      return {false, "The file is not recognized as a valid .gbow preset file."};
    }

    juce::AudioBuffer<float> fileAudioBuffer;
    double sampleRate;

    // Currently there is only a VERSION_MAJOR of 0
    if (header.versionMajor == 0) {
      // Get Audio Buffer blob
      fileAudioBuffer.setSize(header.audioBufferChannel, header.audioBufferNumberOfSamples);
      input.read(fileAudioBuffer.getWritePointer(0), header.audioBufferSize);
      sampleRate = header.audioBufferSamplerRate;

      // Get offsets and load all png for spec images
      uint32_t maxSpecImageSize =
          juce::jmax(header.specImageSpectrogramSize, header.specImageHpcpSize, header.specImageDetectedSize);
      void* specImageData = malloc(maxSpecImageSize);
      jassert(specImageData != nullptr);
      input.read(specImageData, header.specImageSpectrogramSize);
      mParameters.ui.specImages[ParamUI::SpecType::SPECTROGRAM] =
          juce::PNGImageFormat::loadFrom(specImageData, header.specImageSpectrogramSize);
      input.read(specImageData, header.specImageHpcpSize);
      mParameters.ui.specImages[ParamUI::SpecType::HPCP] = juce::PNGImageFormat::loadFrom(specImageData, header.specImageHpcpSize);
      input.read(specImageData, header.specImageDetectedSize);
      mParameters.ui.specImages[ParamUI::SpecType::DETECTED] =
          juce::PNGImageFormat::loadFrom(specImageData, header.specImageDetectedSize);
      mParameters.ui.specComplete = true;
      free(specImageData);

      // juce::FileInputStream uses 'int' to read
      int xmlSize = static_cast<int>(input.getTotalLength() - input.getPosition());
      void* xmlData = malloc(xmlSize);
      jassert(xmlData != nullptr);
      input.read(xmlData, xmlSize);
      setPresetParamsXml(xmlData, xmlSize);
      free(xmlData);
    } else {
      juce::String error = "The file is .gbow version " + juce::String(header.versionMajor) + "." +
                           juce::String(header.versionMinor) +
                           " and is not supported. This copy of gRainbow can open files up to version " +
                           juce::String(Preset::VERSION_MAJOR) + "." + juce::String(Preset::VERSION_MINOR);
      return {false, error};
    }

    if (mSampleRate != INVALID_SAMPLE_RATE) {
      resampleAudioBuffer(fileAudioBuffer, mAudioBuffer, sampleRate, mSampleRate);
    } else {
      mInputBuffer = fileAudioBuffer;  // Save for resampling once prepareToPlay() has been called
      mSampleRate = sampleRate;        // A bit hacky, but we need to store the file's sample rate for resampling
      mNeedsResample = true;
    }
  } else {
    juce::String error = "The file failed to open with message: " + input.getStatus().getErrorMessage();
    return {false, error};
  }
  mLoadingProgress = 1.0f;
  return {true, ""};
}

void GranularSynth::resampleAudioBuffer(juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer,
                                        double inputSampleRate, double outputSampleRate, bool clearInput) {
  // resamples the buffer from the file sampler rate to the the proper sampler
  // rate set from the DAW in prepareToPlay.
  const double ratioToInput = inputSampleRate / outputSampleRate;   // input / output
  const double ratioToOutput = outputSampleRate / inputSampleRate;  // output / input
  // The output buffer needs to be size that matches the new sample rate
  const int resampleSize = static_cast<int>(static_cast<double>(inputBuffer.getNumSamples()) * ratioToOutput);
  mParameters.ui.trimPlaybackMaxSample = resampleSize;
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

void GranularSynth::trimAudioBuffer(juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer,
                                    juce::Range<juce::int64> range, bool clearInput) {
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

void GranularSynth::extractPitches() {
  // Cancel processing if in progress
  mPitchDetector.cancelProcessing();
  mLoadingProgress = 0.0;
  mPitchDetector.process(&mAudioBuffer, mSampleRate);
}

void GranularSynth::extractSpectrograms() {
  // Cancel processing if in progress
  mFft.stopThread(4000);
  mProcessedSpecs.fill(nullptr);
  mFft.process(&mAudioBuffer);
}

std::vector<ParamCandidate*> GranularSynth::getActiveCandidates() {
  std::vector<ParamCandidate*> candidates;
  for (int i = 0; i < NUM_GENERATORS; ++i) {
    if (mParameters.note.notes[mLastPitchClass]->shouldPlayGenerator(i)) {
      candidates.push_back(mParameters.note.notes[mLastPitchClass]->getCandidate(i));
    } else {
      candidates.push_back(nullptr);
    }
  }
  return candidates;
}

void GranularSynth::handleNoteOn(juce::MidiKeyboardState*, int, int midiNoteNumber, float velocity) {
  mLastPitchClass = Utils::getPitchClass(midiNoteNumber);
  mMidiNotes.add(Utils::MidiNote(mLastPitchClass, velocity));
  GrainNote* gNote = std::find_if(mActiveNotes.begin(), mActiveNotes.end(), [this](GrainNote& g) { return g.pitchClass == mLastPitchClass; });
  if (gNote != mActiveNotes.end()) gNote->retrigger(velocity, mTotalSamps);
  else mActiveNotes.add(GrainNote(mLastPitchClass, velocity, Utils::EnvelopeADSR(mTotalSamps)));
}

void GranularSynth::handleNoteOff(juce::MidiKeyboardState*, int, int midiNoteNumber, float) {
  const Utils::PitchClass pitchClass = Utils::getPitchClass(midiNoteNumber);

  for (Utils::MidiNote* it = mMidiNotes.begin(); it != mMidiNotes.end(); it++) {
    if (it->pitch == pitchClass) {
      mMidiNotes.remove(it);
      break;  // will only be at most 1 note (TODO assuming mouse and midi aren't set at same time)
    }
  }

  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == pitchClass && gNote.removeTs == -1) {
      // Set timestamp to delete note based on release time and set note off for all generators
      float maxRelease = 0;
      for (size_t i = 0; i < NUM_GENERATORS; ++i) {
        gNote.genAmpEnvs[i].noteOff(mTotalSamps);
        // Update max release time
        float release =
            mParameters.getFloatParam(mParameters.note.notes[gNote.pitchClass]->generators[i].get(), ParamCommon::Type::RELEASE);
        if (release >= maxRelease) maxRelease = release;
      }
      gNote.removeTs = mTotalSamps + (maxRelease * mSampleRate);

      break;
    }
  }
}

void GranularSynth::resetParameters(bool fullClear) {
  mParameters.note.resetParams(fullClear);
  mParameters.global.resetParams();
}

void GranularSynth::createCandidates(juce::HashMap<Utils::PitchClass, std::vector<PitchDetector::Pitch>>& detectedPitches) {
  // Add candidates for each pitch class
  for (auto&& note : mParameters.note.notes) {
    // Look for detected pitches with correct pitch and good gain
    bool foundAll = false;
    int numFound = 0;
    int numSearches = 0;

    while (!foundAll) {
      int noteMin = note->noteIdx - numSearches;
      int noteMax = note->noteIdx + numSearches;
      // Check low note
      std::vector<PitchDetector::Pitch>& pitchVec = detectedPitches.getReference((Utils::PitchClass)(noteMin % 12));
      float pbRate = std::pow(Utils::TIMESTRETCH_RATIO, numSearches);
      for (size_t i = 0; i < pitchVec.size(); ++i) {
        if (pitchVec[i].gain < MIN_CANDIDATE_SALIENCE) continue;
        note->candidates.push_back(ParamCandidate(pitchVec[i].posRatio, pbRate, pitchVec[i].duration, pitchVec[i].gain));
        numFound++;
        if (numFound >= MAX_CANDIDATES) {
          foundAll = true;
          break;
        }
      }
      // Check high note if we haven't filled up the list yet
      if (!foundAll && numSearches > 0) {
        std::vector<PitchDetector::Pitch>& pitchVec2 = detectedPitches.getReference((Utils::PitchClass)(noteMax % 12));
        float pbRate2 = std::pow(Utils::TIMESTRETCH_RATIO, -numSearches);
        for (size_t i = 0; i < pitchVec2.size(); ++i) {
          if (pitchVec2[i].gain < MIN_CANDIDATE_SALIENCE) continue;
          note->candidates.push_back(ParamCandidate(pitchVec2[i].posRatio, pbRate2, pitchVec2[i].duration, pitchVec2[i].gain));
          numFound++;
          if (numFound >= MAX_CANDIDATES) {
            foundAll = true;
            break;
          }
        }
      }
      numSearches++;
      if (numSearches >= 6 || foundAll) break;
    }

    note->setStartingCandidatePosition();
  }
}
