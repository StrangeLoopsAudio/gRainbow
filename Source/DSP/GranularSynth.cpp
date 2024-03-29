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
#include "Utils/Files.h"
#include "Utils/Presets.h"
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
juce::Thread("pitch detection"),
mFft(FFT_SIZE, HOP_SIZE) {
  
  Utils::FILE_RECENT_FILES.create(); // Creates recent files if it doesn't exist

  mParameters.note.addParams(*this);
  mParameters.global.addParams(*this);

  mTotalSamps = 0;
  mProcessedSpecs.fill(nullptr);

  mKeyboardState.addListener(this);

  mFormatManager.registerBasicFormats();

  mReferenceTone.setAmplitude(0.0f);

  mParameters.resetParams();

  juce::MemoryBlock block;
  Utils::getBlockForPreset(Utils::PRESETS[0], block);
  loadPreset(block);
}

GranularSynth::~GranularSynth() {
  stopThread(10000);
}

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

void GranularSynth::run() {
  // Resample the audio buffer for feeding into the pitch detector
  Utils::resampleAudioBuffer(mAudioBuffer, mDownsampledAudio, mSampleRate, BASIC_PITCH_SAMPLE_RATE);

  // Then use BasicPitch ML to extract note events
  mPitchDetector.transcribeToMIDI(mDownsampledAudio.getWritePointer(0),
                                  mDownsampledAudio.getNumSamples());

  if (threadShouldExit()) return;

  createCandidates(); // Create candidates from MIDI events

  if (threadShouldExit()) return;

  // Finally, calc FFT and HPCP
  mProcessedSpecs[ParamUI::SpecType::SPECTROGRAM] = mFft.process(&mAudioBuffer);
  mProcessedSpecs[ParamUI::SpecType::HPCP] = mHPCP.process(mFft.getSpectrum(), mSampleRate);
  makePitchSpec();
  mProcessedSpecs[ParamUI::SpecType::DETECTED] = &mPitchSpecBuffer;

  if (threadShouldExit()) return;

  //mPitchDetector.reset();

  mParameters.ui.specComplete = false; // Make arc spec render the specs into images
  // Arc spec turns off isLoading once images are rendered as well
}

//==============================================================================
void GranularSynth::prepareToPlay(double sampleRate, int samplesPerBlock) {
  // Make a temporary buffer copy for resampling
  if (mInputBuffer.getNumSamples() > 0) {
    // Resample input buffer (untrimmed)
    juce::AudioSampleBuffer inputBuffer = mInputBuffer;
    Utils::resampleAudioBuffer(inputBuffer, mInputBuffer, mSampleRate, sampleRate);
  }
  if (mAudioBuffer.getNumSamples() > 0) {
    // Resample main buffer
    juce::AudioSampleBuffer inputBuffer = mAudioBuffer;
    Utils::resampleAudioBuffer(inputBuffer, mAudioBuffer, mSampleRate, sampleRate);
  }

  mSampleRate = sampleRate;
  
  const juce::dsp::ProcessSpec filtConfig = {sampleRate, (juce::uint32)samplesPerBlock, (unsigned int)getTotalNumOutputChannels()};
  mMeterSource.resize(getTotalNumOutputChannels(), sampleRate * 0.1 / samplesPerBlock);
  mReferenceTone.prepareToPlay(samplesPerBlock, sampleRate);
  mParameters.prepareModSources(samplesPerBlock, sampleRate);
}

void GranularSynth::releaseResources() {
  mReferenceTone.releaseResources();
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

  // Update mod source values once per block
  mParameters.processModSources();

  mKeyboardState.processNextMidiBuffer(midiMessages, 0, bufferNumSample, true);
  for (const auto& messageMeta : midiMessages) {
    juce::MidiMessage msg = messageMeta.getMessage();
    if (msg.isController() && msg.getControllerNumber() == 1) {
      // Update macro 1 based on mod wheel input
      ParamHelper::setParam(mParameters.global.macros[0].macro, msg.getControllerValue() / 127.0f);
    } else if (msg.isPitchWheel()) {
      // Update local pitch bend value
      mCurPitchBendSemitones = ((msg.getPitchWheelValue() / 8192.0f) - 1.0f) * MAX_PITCH_BEND_SEMITONES;
    }
  }

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
    buffer.clear(i, 0, bufferNumSample);
  }

  // Reference tone
  mReferenceTone.getNextAudioBlock(juce::AudioSourceChannelInfo(&buffer, 0, buffer.getNumSamples()));

  // Playback from trim selection panel
  if (mParameters.ui.playingTrimSelection) {
    int numPlaybackSamples = bufferNumSample;
    if (mParameters.ui.trimPlaybackSample + bufferNumSample >= mInputBuffer.getNumSamples()) {
      // Done playing, reset status
      mParameters.ui.playingTrimSelection = false;
    } else {
      // if output buffer is stereo and the input in mono, duplicate into both channels
      // if output buffer is mono and the input in stereo, just play one channel for simplicity
      for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const int inputChannel = juce::jmin(ch, mInputBuffer.getNumChannels() - 1);
        buffer.copyFrom(ch, 0, mInputBuffer, inputChannel, mParameters.ui.trimPlaybackSample, numPlaybackSamples);
      }
      mParameters.ui.trimPlaybackSample += numPlaybackSamples;
    }
  }

  // Add contributions from each note
  auto bufferChannels = buffer.getArrayOfWritePointers();
  for (int i = 0; i < bufferNumSample; ++i) {
    // Don't use a for(auto x : mActiveNotes) loop here as mActiveNotes can be added outside this function. If it is partially added
    // it might to use it and the undefined data will cause a crash eventually
    const int activeNoteSize = mActiveNotes.size();
    for (int noteIndex = 0; noteIndex < activeNoteSize; noteIndex++) {
      GrainNote* gNote = mActiveNotes[noteIndex];
      // Fix velocity scale (with a slight skew)
      float velocityGain = juce::jmin(1.0f, juce::Decibels::decibelsToGain( ParamRanges::GAIN.convertFrom0to1(juce::jmin(1.0, log10(gNote->velocity + 0.1) + 1))));
      // Add contributions from the grains in this generator
      for (size_t genIdx = 0; genIdx < NUM_GENERATORS; ++genIdx) {
        ParamGenerator* paramGenerator = mParameters.note.notes[gNote->pitchClass]->generators[genIdx].get();
        const float gain = juce::Decibels::decibelsToGain(mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GAIN, true));
        const float attack = mParameters.getFloatParam(mParameters.global.ampEnvAttack, true);
        const float decay = mParameters.getFloatParam(mParameters.global.ampEnvDecay, true);
        const float sustain = juce::Decibels::decibelsToGain(mParameters.getFloatParam(mParameters.global.ampEnvSustain, true));
        const float release = mParameters.getFloatParam(mParameters.global.ampEnvRelease, true);
        const float grainGain =
            gNote->genAmpEnvs[genIdx].getAmplitude(mTotalSamps, attack * mSampleRate, decay * mSampleRate, sustain,
                                                  release * mSampleRate) * gain * velocityGain;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
          float genSample = 0.0f;
          for (Grain* grain : gNote->genGrains[genIdx]) {
            genSample += grain->process(ch / (float)(buffer.getNumChannels() - 1), mAudioBuffer, grainGain, mTotalSamps);
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
  
  double bpm = DEFAULT_BPM;
  int beatsPerBar = DEFAULT_BEATS_PER_BAR;
  if (juce::AudioPlayHead* playhead = getPlayHead()) {
    if (playhead->getPosition()->getBpm()) {
      bpm = *(playhead->getPosition()->getBpm());
    }
    if (playhead->getPosition()->getTimeSignature()) {
      beatsPerBar = playhead->getPosition()->getTimeSignature()->numerator;
    }
  }
  // Update sync rate of lfos
  mBarsPerSec = (1.0f / bpm) * 60.0f * beatsPerBar;
  for (auto& lfo : mParameters.global.modLFOs) {
    lfo.setSyncRate(mBarsPerSec);
  }

  handleGrainAddRemove(bufferNumSample);

  // Reset timestamps if no grains active to keep numbers low
  if (mGrainPool.getNumUsedGrains() == 0 && mActiveNotes.size() == 0) {
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
  Utils::Result r = savePreset(destData);
  if (!r.success) DBG(juce::String("Error during getStateInformation(): ") + r.message);
}

void GranularSynth::setStateInformation(const void* data, int sizeInBytes) {
  if (this->wrapperType == GranularSynth::WrapperType::wrapperType_Standalone) {
    return;  // reloadPluginState() can try loading old/bad/stale info and crash at launch
  }

  juce::MemoryBlock block(data, sizeInBytes);
  Utils::Result r = loadPreset(block);
  if (!r.success) DBG(juce::String("Error during getStateInformation(): ") + r.message);
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
  xml.addChildElement(mParameters.getModulationsXml());

  copyXmlToBinary(xml, destData);
}

// Sets parameters based on memory stored in a preset
void GranularSynth::setPresetParamsXml(const void* data, int sizeInBytes) {
  // Reset all params and candidates
  mParameters.resetParams();
  mParameters.note.clearCandidates();

  auto xml = getXmlFromBinary(data, sizeInBytes);
  if (xml != nullptr) {
    auto params = xml->getChildByName("AudioParams");
    if (params != nullptr) {
      for (auto& param : getParameters()) {
        param->setValueNotifyingHost(params->getDoubleAttribute(ParamHelper::getParamID(param), param->getValue()));
      }
    }

    // Set isUsed for each common parameter based on param values
    for (int i = 0; i < ParamCommon::Type::NUM_COMMON; ++i) {
      for (auto& note: mParameters.note.notes) {
        if (note->common[i]->getValue() != COMMON_RANGES[i].convertTo0to1(COMMON_DEFAULTS[i])) {
          note->isUsed[i] = true;
        }
        for (auto& gen : note->generators) {
          if (gen->common[i]->getValue() != COMMON_RANGES[i].convertTo0to1(COMMON_DEFAULTS[i])) {
            gen->isUsed[i] = true;
          }
        }
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
    
    params = xml->getChildByName("ParamModulations");
    if (params != nullptr) {
      mParameters.setModulationsXml(params);
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
  if (mParameters.ui.specComplete) {
    // Add one grain per active note
    for (GrainNote* gNote : mActiveNotes) {
      for (size_t i = 0; i < gNote->grainTriggers.size(); ++i) {
        if (gNote->grainTriggers[i] <= 0) {
          ParamGenerator* paramGenerator = mParameters.note.notes[gNote->pitchClass]->generators[i].get();
          ParamCandidate* paramCandidate = mParameters.note.notes[gNote->pitchClass]->getCandidate(i);
          float durSec;
          const float gain = juce::Decibels::decibelsToGain(mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GAIN, true));
          const float grainRate = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GRAIN_RATE, true);
          const float grainDuration = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GRAIN_DURATION, true);
          const bool grainSync = mParameters.getBoolParam(paramGenerator, ParamCommon::Type::GRAIN_SYNC);
          const float pitchAdjust = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::PITCH_ADJUST, true);
          const float pitchSpray = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::PITCH_SPRAY, true);
          const float posAdjust = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::POS_ADJUST, true);
          const float posSpray = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::POS_SPRAY, true);
          const float panAdjust = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::PAN_ADJUST, true);
          const float panSpray = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::PAN_SPRAY, true);
          const float shape = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GRAIN_SHAPE, true);
          const float tilt = mParameters.getFloatParam(paramGenerator, ParamCommon::Type::GRAIN_TILT, true);
          const bool reverse = mParameters.getBoolParam(paramGenerator, ParamCommon::Type::REVERSE);
          const float octaveAdjust = mParameters.getIntParam(paramGenerator, ParamCommon::Type::OCTAVE_ADJUST);

          if (grainSync) {
            float div = std::pow(2, juce::roundToInt(ParamRanges::SYNC_DIV_MAX * (1.0f - ParamRanges::GRAIN_DURATION.convertTo0to1(grainDuration))));
            // Find synced duration using bpm
            durSec = mBarsPerSec / div;
          } else {
            durSec = grainDuration;
          }
          // Skip adding new grain if not enabled or full of grains
          if (paramCandidate != nullptr && mParameters.note.notes[gNote->pitchClass]->shouldPlayGenerator(i)) {
            // Get the next available grain from the pool (if there is one)
            Grain* grain = mGrainPool.getNextAvailableGrain();
            if (grain != nullptr) {
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
              float pitchBendOffset = std::pow(Utils::TIMESTRETCH_RATIO, mCurPitchBendSemitones) - 1;
              float pbRate = paramCandidate->pbRate + pitchAdjust + pitchSprayOffset + pitchBendOffset;
              if (reverse) {
                pbRate = -pbRate; // Flip playback rate if going in reverse
                posSamples += durSamples; // Start at the end
              }
              /* Octave correction (centered around 0) */
              pbRate *= std::pow(2, (gNote->pitch / 12) - paramCandidate->octave + octaveAdjust); // From candidate and octave adjust
              jassert(paramCandidate->pbRate > 0.1f);

              /* Add grain */
              grain->set(durSamples, pbRate, posSamples, mTotalSamps, gain, panOffset, shape, tilt);
              gNote->genGrains[i].add(grain);

              /* Trigger grain in arcspec */
              float totalGain = gain * gNote->genAmpEnvs[i].amplitude;
              mParameters.note.grainCreated(gNote->pitchClass, i, durSec / pbRate, totalGain);
            }
          }
          // Reset trigger ts
          if (grainSync) {
            float div = std::pow(2, juce::roundToInt(ParamRanges::SYNC_DIV_MAX * ParamRanges::GRAIN_RATE.convertTo0to1(grainRate)));
            float rateSec = mBarsPerSec / div;
            // Find synced rate interval using bpm
            float intervalSamples = mSampleRate * rateSec;
            gNote->grainTriggers[i] += intervalSamples;
          } else {
            gNote->grainTriggers[i] += mSampleRate / grainRate;
          }
        } else {
          gNote->grainTriggers[i] -= blockSize;
        }
      }
    }
  }
  // Delete expired grains
  mGrainPool.reclaimExpiredGrains(mTotalSamps);
  for (GrainNote* gNote : mActiveNotes) {
    for (int genIdx = 0; genIdx < NUM_GENERATORS; ++genIdx) {
      gNote->genGrains[genIdx].removeIf([](Grain* g) { return !g->isActive; });
    }
  }

  // Delete expired notes
  std::vector<GrainNote*> notesToRemove;
  for (auto* gNote : mActiveNotes) {
    if (gNote->removeTs != -1 && mTotalSamps >= gNote->removeTs) {
      for (int genIdx = 0; genIdx < NUM_GENERATORS; ++genIdx) {
        for (auto* grain : gNote->genGrains[genIdx]) {
          if (grain) grain->isActive = false;
        }
      }
      notesToRemove.push_back(gNote);
    }
  }
  for (auto* gNote : notesToRemove) {
    for (Utils::MidiNote* it = mMidiNotes.begin(); it != mMidiNotes.end(); it++) {
      if (Utils::getPitchClass(it->pitch) == gNote->pitchClass) {
        mMidiNotes.remove(it);
        break;  // will only be at most 1 note (TODO assuming mouse and midi aren't set at same time)
      }
    }
    mActiveNotes.removeObject(gNote);
  }
}

Utils::Result GranularSynth::loadAudioFile(juce::File file) {
  juce::AudioFormatReader* formatReader = mFormatManager.createReaderFor(file);
  if (formatReader == nullptr) return {false, "Opening failed: unsupported file format"};

  juce::AudioBuffer<float> fileAudioBuffer;
  const int length = static_cast<int>(formatReader->lengthInSamples);
  fileAudioBuffer.setSize(1, length);
  formatReader->read(&fileAudioBuffer, 0, length, 0, true, false);

  // .mp3 files, unlike .wav files, can contain PCM values greater than abs(1.0) (aka, clipping) which will produce awful
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

  Utils::resampleAudioBuffer(fileAudioBuffer, mInputBuffer, formatReader->sampleRate, mSampleRate);
  
  // should not need the file anymore as we want to work with the resampled input buffer across the rest of the plugin
  delete (formatReader);

  return {true, ""};
}

Utils::Result GranularSynth::loadPreset(juce::File file) {
  juce::FileInputStream input(file);
  if (input.openedOk()) {
    juce::MemoryBlock block;
    input.readIntoMemoryBlock(block);
    Utils::Result r = loadPreset(block);
    if (r.success) {
      mParameters.ui.fileName = file.getFileName();
      mParameters.ui.loadedFileName = mParameters.ui.fileName;
      Utils::addRecentFile(file.getFullPathName()); // Save recent file to list
    }
    return r;
  } else {
    juce::String error = "The file failed to open with message: " + input.getStatus().getErrorMessage();
    return {false, error};
  }
}

Utils::Result GranularSynth::loadPreset(juce::MemoryBlock& block) {
  int curBlockPos = 0;
  Preset::Header header;
  block.copyTo(&header, curBlockPos, sizeof(header));
  curBlockPos += sizeof(header);

  if (header.magic != Preset::MAGIC) {
    return {false, "The file is not recognized as a valid .gbow preset file."};
  }

  juce::AudioBuffer<float> fileAudioBuffer;
  double sampleRate;

  // Currently there is only a VERSION_MAJOR of 0
  if (header.versionMajor == 0) {
    // Get Audio Buffer blob
    fileAudioBuffer.setSize(header.audioBufferChannel, header.audioBufferNumberOfSamples);
    block.copyTo(fileAudioBuffer.getWritePointer(0), curBlockPos, header.audioBufferSize);
    curBlockPos += header.audioBufferSize;
    sampleRate = header.audioBufferSamplerRate;

    // The image data is saved in the XML, for backward compatibility might need to ignore duplciated image data
    curBlockPos += header.specImageSpectrogramSize;
    curBlockPos += header.specImageHpcpSize;
    curBlockPos += header.specImageDetectedSize;

    // juce::FileInputStream uses 'int' to read
    int xmlSize = static_cast<int>(block.getSize() - curBlockPos);
    void* xmlData = malloc(xmlSize);
    jassert(xmlData != nullptr);
    block.copyTo(xmlData, curBlockPos, xmlSize);
    curBlockPos += xmlSize;
    setPresetParamsXml(xmlData, xmlSize);
    mParameters.ui.specComplete = true;
    free(xmlData);
  } else {
    juce::String error = "The file is .gbow version " + juce::String(header.versionMajor) + "." +
                          juce::String(header.versionMinor) +
                          " and is not supported. This copy of gRainbow can open files up to version " +
                          juce::String(Preset::VERSION_MAJOR) + "." + juce::String(Preset::VERSION_MINOR);
    return {false, error};
  }

  mInputBuffer.clear();
  mAudioBuffer.clear();
  Utils::resampleAudioBuffer(fileAudioBuffer, mAudioBuffer, sampleRate, mSampleRate);
  jassert(!mParameters.ui.isLoading);
  return {true, ""};
}

Utils::Result GranularSynth::savePreset(juce::File file) {
  file.deleteFile();  // clear file if replacing
  juce::String lastFileName = mParameters.ui.loadedFileName;
  // Set ui fields so they're correctly saved
  mParameters.ui.fileName = file.getFileName();
  mParameters.ui.loadedFileName = mParameters.ui.fileName;
  juce::MemoryBlock block;
  Utils::Result r = savePreset(block);
  if (r.success) {
    file.replaceWithData(block.getData(), block.getSize());
    Utils::addRecentFile(file.getFullPathName()); // Save recent file to list
  } else {
    mParameters.ui.loadedFileName = lastFileName;  // Reset filename if failed
  }
  return r;
}

Utils::Result GranularSynth::savePreset(juce::MemoryBlock& block) {
  Preset::Header header;
  header.magic = Preset::MAGIC;
  header.versionMajor = Preset::VERSION_MAJOR;
  header.versionMinor = Preset::VERSION_MINOR;
  // Audio buffer data is grabbed from current synth
  header.audioBufferSamplerRate = mSampleRate;
  header.audioBufferNumberOfSamples = mAudioBuffer.getNumSamples();
  header.audioBufferChannel = mAudioBuffer.getNumChannels();
  header.audioBufferSize = header.audioBufferNumberOfSamples * header.audioBufferChannel * sizeof(float);

  // For backward compatibility keep values and just have them be zero
  header.specImageSpectrogramSize = 0;
  header.specImageHpcpSize = 0;
  header.specImageDetectedSize = 0;

  // XML structure of preset contains all audio related information
  // These include not just AudioParams but also other params not exposes to
  // the DAW or UI directly
  juce::MemoryBlock xmlMemoryBlock;
  getPresetParamsXml(xmlMemoryBlock);

  // Write data out section by section
  juce::MemoryOutputStream blockStream(block, true);
  blockStream.write(&header, sizeof(header));
  if (mAudioBuffer.getNumSamples() > 0) {
    blockStream.write(reinterpret_cast<const void*>(mAudioBuffer.getReadPointer(0)), header.audioBufferSize);
  }
  blockStream.write(xmlMemoryBlock.getData(), xmlMemoryBlock.getSize());
  blockStream.flush();

  return {true, ""};
}

void GranularSynth::trimAndExtractPitches(juce::Range<double> range) {
  mParameters.ui.playingTrimSelection = false;
  mParameters.ui.trimRange = range;

  // Trim
  const double sampleLength = static_cast<double>(mInputBuffer.getNumSamples());
  const double secondLength = sampleLength / mSampleRate;
  juce::int64 start = static_cast<juce::int64>(sampleLength * (range.getStart() / secondLength));
  juce::int64 end = static_cast<juce::int64>(sampleLength * (range.getEnd() / secondLength));
  Utils::trimAudioBuffer(mInputBuffer, mAudioBuffer, juce::Range<juce::int64>(start, end));
  mInputBuffer.clear();
  
  // Extract pitches
  stopThread(10000);
  mParameters.ui.isLoading = false;
  mProcessedSpecs.fill(nullptr);
  mParameters.ui.isLoading = true;
  mParameters.note.clearCandidates();
  mParameters.setSelectedParams(&mParameters.global);
  startThread();
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
  auto foundNote = std::find_if(mActiveNotes.begin(), mActiveNotes.end(), [this, midiNoteNumber](const GrainNote* gNote) { return gNote->pitch == midiNoteNumber; });
  if (foundNote == mActiveNotes.end()) {
    // New note, start 'er up
    mMidiNotes.add(Utils::MidiNote(mLastPitchClass, velocity));
    mActiveNotes.add(new GrainNote(midiNoteNumber, velocity, mTotalSamps));
  } else {
    // Already playing note, just reset the envelope
    (*foundNote)->velocity = velocity;
    (*foundNote)->noteOn(mTotalSamps);
  }

  // Retrigger modulators
  for (auto& lfo : mParameters.global.modLFOs) {
    lfo.checkRetrigger();
  }
  for (auto& env : mParameters.global.modEnvs) {
    env.handleNoteOn(mTotalSamps);
  }
}

void GranularSynth::handleNoteOff(juce::MidiKeyboardState*, int, int midiNoteNumber, float) {
  const Utils::PitchClass pitchClass = Utils::getPitchClass(midiNoteNumber);

  for (GrainNote* gNote : mActiveNotes) {
    if (gNote->pitch == midiNoteNumber && gNote->removeTs == -1) {
      // Set timestamp to delete note based on release time and set note off for all generators
      float release = mParameters.getFloatParam(mParameters.global.ampEnvRelease, true);;
      for (size_t i = 0; i < NUM_GENERATORS; ++i) {
        gNote->genAmpEnvs[i].noteOff(mTotalSamps);
      }
      gNote->removeTs = mTotalSamps + static_cast<int>(release * mSampleRate);
      break;
    }
  }
  // Update mod envelopes
  for (auto& env : mParameters.global.modEnvs) {
    env.handleNoteOff(mTotalSamps);
  }
}

void GranularSynth::makePitchSpec() {
  mPitchSpecBuffer.clear();
  const int numFrames = mPitchDetector.getNumFrames();
  const std::vector<Notes::Event>& events = mPitchDetector.getNoteEvents();
  // Find max and min pitches
  auto pitchRange = juce::Range<int>(-1, -1);
  for (auto& evt : events) {
    if (evt.pitch > pitchRange.getEnd() || pitchRange.getEnd() == -1) pitchRange.setEnd(evt.pitch);
    if (evt.pitch < pitchRange.getStart() || pitchRange.getStart() == -1) pitchRange.setStart(evt.pitch);
  }
  // Init buffer size
  for (size_t frame = 0; frame < numFrames; ++frame) {
    mPitchSpecBuffer.emplace_back(std::vector<float>(pitchRange.getLength() + 1, 0.0f));
  }
  for (auto& evt : events) {
    const int duration = (evt.endFrame - evt.startFrame);
    for (float k = evt.startFrame; k < evt.endFrame; ++k) {
      int pitchIdx = pitchRange.getLength() - (evt.pitch - pitchRange.getStart());
      mPitchSpecBuffer[k][pitchIdx] = evt.amplitude;
    }
  }
}

void GranularSynth::createCandidates() {
  // Add candidates for each pitch class
  const std::vector<Notes::Event>& events = mPitchDetector.getNoteEvents();
  const int numFrames = mPitchDetector.getNumFrames();
  for (auto&& note : mParameters.note.notes) {
    // Look for detected pitches with correct pitch and good gain
    bool foundAll = false;
    int numFound = 0;
    int numSearches = 0;

    while (!foundAll) {
      // Check between note ranges, expanding search until we're full
      int noteMin = note->noteIdx - numSearches;
      int noteMax = note->noteIdx + numSearches;
      for (auto& evt : events) {
        int pitchClass = evt.pitch % 12;
        if (pitchClass == noteMin || pitchClass == noteMax) {
          if (evt.amplitude < MIN_CANDIDATE_SALIENCE) continue;
          // Got through the checks, found a valid candidate
          // Calc scaled PB rate for this candidate to play at the right pitch, negating if higher than desired pitch to slow down
          float pbRate = std::pow(Utils::TIMESTRETCH_RATIO, (pitchClass > note->noteIdx) ? -numSearches : numSearches);
          int octave = evt.pitch / 12;
          float posRatio = (float)evt.startFrame / numFrames;
          float duration = (float)(evt.endFrame - evt.startFrame) / numFrames;
          note->candidates.emplace_back(ParamCandidate(posRatio, octave, pbRate, duration, evt.amplitude));
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
