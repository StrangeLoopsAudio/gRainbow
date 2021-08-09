/*
  ==============================================================================

    GranularSynth.cpp
    Created: 12 Apr 2021 11:22:41am
    Author:  brady

  ==============================================================================
*/

#include "GranularSynth.h"
#include "PluginEditor.h"

GranularSynth::GranularSynth()
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
      ,
      mFft(FFT_SIZE, HOP_SIZE),
      apvts(*this, nullptr, "ScaleNavParams", createParameterLayout()) {
  mTotalSamps = 0;
  mFormatManager.registerBasicFormats();

  mFft.onProcessingComplete =
      [this](std::vector<std::vector<float>>& spectrum) {
        if (onBufferProcessed != nullptr) {
          onBufferProcessed(&spectrum, Utils::SpecType::SPECTROGRAM);
        }
      };

  mPitchDetector.onPitchesUpdated =
      [this](std::vector<std::vector<float>>& hpcpBuffer,
             std::vector<std::vector<float>>& notesBuffer) {
        if (onBufferProcessed != nullptr) {
          onBufferProcessed(&hpcpBuffer, Utils::SpecType::HPCP);
          onBufferProcessed(&notesBuffer, Utils::SpecType::NOTES);
        }
        mPositionFinder.setPitches(&mPitchDetector.getPitches());
        mIsProcessingComplete = true;
      };

  mPitchDetector.onProgressUpdated = [this](float progress) {
    if (onProgressUpdated != nullptr) {
      onProgressUpdated(progress);
    }
  };

  resetParameters();
}

GranularSynth::~GranularSynth() { 
  // Get rid of image files
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  parentDir.getChildFile(Utils::FILE_SPECTROGRAM).deleteFile();
  parentDir.getChildFile(Utils::FILE_HPCP).deleteFile();
  parentDir.getChildFile(Utils::FILE_NOTES).deleteFile();
}

//==============================================================================
const juce::String GranularSynth::getName() const {
  return JucePlugin_Name;
}

bool GranularSynth::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool GranularSynth::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
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

void GranularSynth::setCurrentProgram(int index) {}

const juce::String GranularSynth::getProgramName(int index) {
  return {};
}

void GranularSynth::changeProgramName(int index,
                                               const juce::String& newName) {}

//==============================================================================
void GranularSynth::prepareToPlay(double sampleRate,
                                           int samplesPerBlock) {
  mSampleRate = sampleRate;
}

void GranularSynth::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GranularSynth::isBusesLayoutSupported(
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

void GranularSynth::processBlock(juce::AudioBuffer<float>& buffer,
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
  mKeyboardState.processNextMidiBuffer(aggregatedMidiBuffer, 0,
                                       buffer.getNumSamples(), true);

  // Add midi events from native buffer
  aggregatedMidiBuffer.addEvents(midiMessages, 0, buffer.getNumSamples(), 0);
  if (!aggregatedMidiBuffer.isEmpty()) {
    for (juce::MidiMessageMetadata md : aggregatedMidiBuffer) {
      // Trigger note on/off depending on event type
      Utils::PitchClass pc = (Utils::PitchClass)(
          md.getMessage().getNoteNumber() % Utils::PitchClass::COUNT);
      if (md.getMessage().isNoteOn()) {
        if (onNoteChanged != nullptr) onNoteChanged(pc, true);
        setNoteOn(pc);
      } else if (md.getMessage().isNoteOff()) {
        if (onNoteChanged != nullptr) onNoteChanged(pc, false);
        setNoteOff(pc);
      }
    }
  }

  // Add contributions from each note
  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    for (GrainNote& gNote : mActiveNotes) {
      float noteGain = gNote.ampEnv.getAmplitude(mTotalSamps);
      // Add contributions from each grain
      for (Grain& grain : gNote.grains) {
        float genGain =
            gNote.positions[grain.generator].ampEnv.getAmplitude(mTotalSamps);
        grain.process(mFileBuffer, buffer, noteGain, genGain, mTotalSamps);
      }
    }
    mTotalSamps++;
  }
  
  // Clip buffers to valid range
  for (int i = 0; i < buffer.getNumChannels(); i++) {
    juce::FloatVectorOperations::clip(buffer.getWritePointer(i),
                                      buffer.getReadPointer(i), -1.0f, 1.0f,
                                      buffer.getNumSamples());
  }

  handleGrainAddRemove(buffer.getNumSamples());

  // Reset timestamps if no grains active to keep numbers low
  if (mActiveNotes.isEmpty()) {
    mTotalSamps = 0;
  } else {
    // Normalize the block before sending onward
    // if grains is empty, don't want to divide by zero
    /*for (int i = 0; i < buffer.getNumSamples(); ++i) {
      for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
         float* channelBlock = buffer.getWritePointer(ch);
         channelBlock[i] /= mGrains.size();
      }
    } */
  }
}

//==============================================================================
bool GranularSynth::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GranularSynth::createEditor() {
  return new GRainbowAudioProcessorEditor(*this, apvts);
}

//==============================================================================
void GranularSynth::getStateInformation(juce::MemoryBlock& destData) {
  juce::ValueTree state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void GranularSynth::setStateInformation(const void* data,
                                                 int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new GranularSynth();
}

void GranularSynth::handleGrainAddRemove(int blockSize) {
  if (mIsProcessingComplete) {
    // Add one grain per active note
    for (GrainNote& gNote : mActiveNotes) {
      for (int i = 0; i < gNote.grainTriggers.size(); ++i) {
        if (gNote.grainTriggers[i] <= 0) {
          Utils::GeneratorParams params = mNoteSettings[gNote.pitchClass][i];
          float durMs =
              juce::jmap(params.duration, MIN_DURATION_MS, MAX_DURATION_MS);
          // Skip adding new grain if not enabled or full of grains
          if (i < gNote.positions.size()) {
            if (gNote.positions[i].isActive &&
                gNote.grains.size() < MAX_GRAINS) {
              juce::Random random;
              GrainPositionFinder::GrainPosition& gPos = gNote.positions[i];
              float durSamples =
                  mSampleRate * (durMs / 1000) * (1.0f / gPos.pbRate);
              float posSamples =
                  gPos.pitch.posRatio * mFileBuffer.getNumSamples();
              float posOffset = juce::jmap(
                  random.nextFloat(), 0.0f,
                  (gPos.pitch.duration * mFileBuffer.getNumSamples()) -
                      durSamples);
              posOffset +=
                  (params.posAdjust - 0.5f) * MAX_POS_ADJUST * durSamples;

              // TODO: normalize with this or something
              float rateGainFactor = juce::jmap(
                  1.0f - params.rate, MIN_RATE_RATIO * 2.0f, MAX_RATE_RATIO);
              float gain = params.gain;
              float pbRate = gPos.pbRate +
                             ((params.pitchAdjust - 0.5f) * MAX_PITCH_ADJUST);
              jassert(gPos.pbRate > 0.1f);
              auto grain =
                  Grain((Utils::GeneratorColour)i, params.grainEnv, durSamples,
                        pbRate, posSamples + posOffset, mTotalSamps, gain);
              gNote.grains.add(grain);
            }
          }
          // Reset trigger ts
          gNote.grainTriggers[i] +=
              mSampleRate / 1000 *
              juce::jmap(1.0f - params.rate, durMs * MIN_RATE_RATIO,
                         durMs * MAX_RATE_RATIO);
        } else {
          gNote.grainTriggers[i] -= blockSize;
        }
      }
    }
  }
  // Delete expired grains
  for (GrainNote& gNote : mActiveNotes) {
    gNote.grains.removeIf(
        [this](Grain& g) { return mTotalSamps > (g.trigTs + g.duration); });
  }

  // Delete expired notes
  mActiveNotes.removeIf([this](GrainNote& gNote) {
    return (gNote.ampEnv.noteOnTs < 0 && gNote.ampEnv.noteOffTs < 0);
  });
}

void GranularSynth::processFile(juce::File file) {
  resetParameters();
  mIsProcessingComplete = false;
  std::unique_ptr<juce::AudioFormatReader> reader(
      mFormatManager.createReaderFor(file));

  if (reader.get() != nullptr) {
    mFileBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);

    juce::AudioBuffer<float> tempBuffer = juce::AudioBuffer<float>(
        reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&tempBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    std::unique_ptr<juce::LagrangeInterpolator> resampler =
        std::make_unique<juce::LagrangeInterpolator>();
    double ratio = reader->sampleRate / mSampleRate;
    const float** inputs = tempBuffer.getArrayOfReadPointers();
    float** outputs = mFileBuffer.getArrayOfWritePointers();
    for (int c = 0; c < mFileBuffer.getNumChannels(); c++) {
      resampler->reset();
      resampler->process(ratio, inputs[c], outputs[c],
                         mFileBuffer.getNumSamples());
    }
  }
  mFft.processBuffer(&mFileBuffer);
  mPitchDetector.processBuffer(&mFileBuffer, mSampleRate);
}

void GranularSynth::updateCurPositions() {
  std::vector<GrainPositionFinder::GrainPosition> gPositions =
      mPositionFinder.findPositions(Utils::MAX_POSITIONS, mCurPitchClass);
  std::vector<GrainPositionFinder::GrainPosition> gPosToPlay;
  for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
    Utils::GeneratorParams& params = mNoteSettings[mCurPitchClass][i];
    int position = params.position;
    if (mNoteSettings[mCurPitchClass][i].state.shouldPlay() &&
        position < gPositions.size() && mIsProcessingComplete) {
      gPositions[position].isActive = true;
      // Copy amp env over to new position
      if (i < mCurPositions.size())
        gPositions[position].ampEnv = mCurPositions[i].ampEnv;
      gPositions[position].ampEnv.attack =
          mSampleRate *
          juce::jmap(params.attack, MIN_ATTACK_SEC, MAX_ATTACK_SEC);
      gPositions[position].ampEnv.decay =
          mSampleRate * juce::jmap(params.decay, MIN_DECAY_SEC, MAX_DECAY_SEC);
      gPositions[position].ampEnv.sustain = params.sustain;
      gPositions[position].ampEnv.release =
          mSampleRate *
          juce::jmap(params.release, MIN_RELEASE_SEC, MAX_RELEASE_SEC);
      gPosToPlay.push_back(gPositions[position]);
    } else {
      // Push back an inactive position for the synth
      gPosToPlay.push_back(
          GrainPositionFinder::GrainPosition(PitchDetector::Pitch(), 1.0));
    }
  }
  mCurPositions = gPosToPlay;
}

Utils::GeneratorParams GranularSynth::getGeneratorParams(
    Utils::GeneratorColour colour) {
  return mNoteSettings[mCurPitchClass][colour];
}

int GranularSynth::incrementPosition(int boxNum, bool lookRight) {
  int pos = mNoteSettings[mCurPitchClass][boxNum].position;
  int newPos = lookRight ? pos + 1 : pos - 1;
  newPos = (newPos + Utils::MAX_POSITIONS) % Utils::MAX_POSITIONS;
  mNoteSettings[mCurPitchClass][boxNum].position = newPos;
  updateCurPositions();
  // Find gNote corresponding to current note and update its positions
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == mCurPitchClass) {
      gNote.positions = mCurPositions;
    }
  }
  return newPos;
}

void GranularSynth::setNoteOn(Utils::PitchClass pitchClass) {
  mCurPitchClass = pitchClass;
  updateCurPositions();
  bool isPlayingAlready = false;
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == pitchClass) {
      isPlayingAlready = true;
      // Start envelope over
      gNote.ampEnv.noteOn(mTotalSamps);
      gNote.positions = mCurPositions;
      // Start position envs over as well
      for (GrainPositionFinder::GrainPosition& gPos : gNote.positions) {
        gPos.ampEnv.noteOn(mTotalSamps);
      }
    }
  }
  if (!isPlayingAlready) {
    mActiveNotes.add(GrainNote(
        pitchClass,
        std::vector<Utils::GeneratorParams>(mNoteSettings[pitchClass].begin(),
                                            mNoteSettings[pitchClass].end()),
        mCurPositions,
        Utils::EnvelopeADSR(
            mTotalSamps,
            mSampleRate * juce::jmap(mGlobalParams.attack, MIN_ATTACK_SEC,
                                     MAX_ATTACK_SEC),
            mSampleRate *
                juce::jmap(mGlobalParams.decay, MIN_DECAY_SEC, MAX_DECAY_SEC),
            mGlobalParams.sustain,
            mSampleRate * juce::jmap(mGlobalParams.release, MIN_RELEASE_SEC,
                                     MAX_RELEASE_SEC))));
  }
}

void GranularSynth::setNoteOff(Utils::PitchClass pitchClass) {
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == pitchClass) {
      // Set note off timestamp and set env state to release for each pos
      gNote.ampEnv.noteOff(mTotalSamps);
      for (GrainPositionFinder::GrainPosition& gPos : gNote.positions) {
        gPos.ampEnv.noteOff(mTotalSamps);
      }
      break;
    }
  }
}

void GranularSynth::updateGeneratorStates(std::vector<Utils::GeneratorState> genStates) {
  for (int i = 0; i < genStates.size(); ++i) {
    mNoteSettings[mCurPitchClass][i].state = genStates[i];
  }
  updateCurPositions();
  // Find gNote corresponding to current note and update its positions
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == mCurPitchClass) {
      gNote.positions = mCurPositions;
    }
  }
}

void GranularSynth::resetParameters() {
  mGlobalParams = Utils::GlobalParams(
      PARAM_GAIN_DEFAULT, PARAM_ATTACK_DEFAULT, PARAM_DECAY_DEFAULT,
      PARAM_SUSTAIN_DEFAULT, PARAM_RELEASE_DEFAULT);
  // Set same params for all notes
  for (int i = 0; i < mNoteSettings.size(); ++i) {
    for (int j = 0; j < mNoteSettings[i].size(); ++j) {
      Utils::GeneratorState state = Utils::GeneratorState(j == 0, false);
      mNoteSettings[i][j] = Utils::GeneratorParams(
          state, j, PARAM_PITCH_DEFAULT, PARAM_POSITION_DEFAULT,
          PARAM_SHAPE_DEFAULT, PARAM_TILT_DEFAULT, PARAM_RATE_DEFAULT, PARAM_DURATION_DEFAULT,
          PARAM_GAIN_DEFAULT, PARAM_ATTACK_DEFAULT, PARAM_DECAY_DEFAULT,
          PARAM_SUSTAIN_DEFAULT, PARAM_RELEASE_DEFAULT,
          getGrainEnvelope(PARAM_SHAPE_DEFAULT, PARAM_TILT_DEFAULT));
    }
  }
}

void GranularSynth::updateGeneratorParameter(Utils::GeneratorColour colour,
                                    ParameterType param, float value) {
  switch (param) {
    case ParameterType::PITCH_ADJUST:
      mNoteSettings[mCurPitchClass][colour].pitchAdjust = value;
      break;
    case ParameterType::POSITION_ADJUST:
      mNoteSettings[mCurPitchClass][colour].posAdjust = value;
      break;
    case ParameterType::SHAPE:
      mNoteSettings[mCurPitchClass][colour].shape = value;
      mNoteSettings[mCurPitchClass][colour].grainEnv =
          getGrainEnvelope(value, mNoteSettings[mCurPitchClass][colour].tilt);
      break;
    case ParameterType::TILT:
      mNoteSettings[mCurPitchClass][colour].tilt = value;
      mNoteSettings[mCurPitchClass][colour].grainEnv =
          getGrainEnvelope(mNoteSettings[mCurPitchClass][colour].shape, value);
      break;
    case ParameterType::RATE:
      mNoteSettings[mCurPitchClass][colour].rate = value;
      break;
    case ParameterType::DURATION:
      mNoteSettings[mCurPitchClass][colour].duration = value;
      break;
    case ParameterType::GAIN:
      mNoteSettings[mCurPitchClass][colour].gain = value;
      break;
    case ParameterType::ATTACK:
      mNoteSettings[mCurPitchClass][colour].attack = value;
      for (GrainNote& gNote : mActiveNotes) {
        if (gNote.pitchClass != mCurPitchClass) continue;
        gNote.positions[colour].ampEnv.attack =
            mSampleRate * juce::jmap(value, MIN_ATTACK_SEC, MAX_ATTACK_SEC);
      }
      break;
    case ParameterType::DECAY:
      mNoteSettings[mCurPitchClass][colour].decay = value;
      for (GrainNote& gNote : mActiveNotes) {
        if (gNote.pitchClass != mCurPitchClass) continue;
        gNote.positions[colour].ampEnv.decay =
            mSampleRate * juce::jmap(value, MIN_DECAY_SEC, MAX_DECAY_SEC);
      }
      break;
    case ParameterType::SUSTAIN:
      mNoteSettings[mCurPitchClass][colour].sustain = value;
      for (GrainNote& gNote : mActiveNotes) {
        if (gNote.pitchClass != mCurPitchClass) continue;
        gNote.positions[colour].ampEnv.sustain = value;
      }
      break;
    case ParameterType::RELEASE:
      mNoteSettings[mCurPitchClass][colour].release = value;
      for (GrainNote& gNote : mActiveNotes) {
        if (gNote.pitchClass != mCurPitchClass) continue;
        gNote.positions[colour].ampEnv.release =
            mSampleRate * juce::jmap(value, MIN_RELEASE_SEC, MAX_RELEASE_SEC);
      }
      break;
    default:
      break;
  }
}

void GranularSynth::updateGlobalParameter(ParameterType param, float value) {
  switch (param) {
    case ParameterType::GAIN:
      mGlobalParams.gain = value;
      break;
    case ParameterType::ATTACK:
      mGlobalParams.attack = value;
      for (GrainNote& gNote : mActiveNotes) {
        gNote.ampEnv.attack =
            mSampleRate * juce::jmap(value, MIN_ATTACK_SEC, MAX_ATTACK_SEC);
      }
      break;
    case ParameterType::DECAY:
      mGlobalParams.decay = value;
      for (GrainNote& gNote : mActiveNotes) {
        gNote.ampEnv.decay =
            mSampleRate * juce::jmap(value, MIN_DECAY_SEC, MAX_DECAY_SEC);
      }
      break;
    case ParameterType::SUSTAIN:
      mGlobalParams.sustain = value;
      for (GrainNote& gNote : mActiveNotes) {
        gNote.ampEnv.sustain = value;
      }
      break;
    case ParameterType::RELEASE:
      mGlobalParams.release = value;
      for (GrainNote& gNote : mActiveNotes) {
        gNote.ampEnv.release =
            mSampleRate * juce::jmap(value, MIN_RELEASE_SEC, MAX_RELEASE_SEC);
      }
      break;
    default:
      break;
  }
}

std::vector<float> GranularSynth::getGrainEnvelope(float shape, float tilt) {
  std::vector<float> grainEnv;
  float scaledShape = (shape * GRAIN_ENV_SIZE) / 2.0f;
  float scaledTilt = tilt * GRAIN_ENV_SIZE;
  int rampUpEndSample = juce::jmax(0.0f, scaledTilt - scaledShape);
  int rampDownStartSample = juce::jmin((float)GRAIN_ENV_SIZE, scaledTilt + scaledShape);
  for (int i = 0; i < GRAIN_ENV_SIZE; i++) {
    if (i < rampUpEndSample) {
      grainEnv.push_back((float)i / rampUpEndSample);
    } else if (i > rampDownStartSample) {
      grainEnv.push_back(1.0f - (float)(i - rampDownStartSample) /
                         (GRAIN_ENV_SIZE - rampDownStartSample));
    } else {
      grainEnv.push_back(1.0f);
    }
  }
  juce::FloatVectorOperations::clip(grainEnv.data(), grainEnv.data(), 0.0f,
                                    1.0f, grainEnv.size());
  return grainEnv;
}

juce::AudioProcessorValueTreeState::ParameterLayout
GranularSynth::createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  // TODO: BIG TODO: populate all params for apvts

  // Root combobox
  /*params.push_back(std::make_unique<juce::AudioParameterChoice>(
      "ROOT", "Root",
      juce::StringArray("C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A",
                        "A#", "B"),
      0)); */

  return {params.begin(), params.end()};
}