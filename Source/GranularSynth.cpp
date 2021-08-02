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
  , juce::Thread("granular thread") {
  mTotalSamps = 0;
  mGrains.ensureStorageAllocated(MAX_GRAINS);

  resetParameters();

  startThread();
}

GranularSynth::~GranularSynth() { stopThread(4000); }

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

  // Add contributions from each grain
  juce::Array<Grain> grains = mGrains;  // Copy to avoid threading issues
  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    for (int g = 0; g < grains.size(); ++g) {
      grains[g].process(*mFileBuffer, buffer, mTotalSamps);
    }
    mTotalSamps++;
  }

  // Reset timestamps if no grains active to keep numbers low
  if (mActiveNotes.isEmpty() && mGrains.isEmpty()) {
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

  // Delete expired grains
  mGrains.removeIf(
      [this](Grain& g) { return mTotalSamps > (g.trigTs + g.duration); });

  // Delete expired notes
  mActiveNotes.removeIf([this](GrainNote& gNote) {
    long maxReleaseTime = getMaxReleaseTime(gNote);
    return (mTotalSamps - gNote.noteOffTs) > maxReleaseTime &&
           gNote.noteOffTs > 0;
  });
}

//==============================================================================
bool GranularSynth::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GranularSynth::createEditor() {
  return new GRainbowAudioProcessorEditor(*this);
}

//==============================================================================
void GranularSynth::getStateInformation(juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void GranularSynth::setStateInformation(const void* data,
                                                 int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new GranularSynth();
}


void GranularSynth::run() {
  float waitMs = MIN_RATE_RATIO * MIN_DURATION_MS;
  while (!threadShouldExit()) {
    if (mFileBuffer != nullptr) {
      // Add one grain per active note
      for (GrainNote& gNote : mActiveNotes) {
        for (int i = 0; i < gNote.grainTriggersMs.size(); ++i) {
          if (gNote.grainTriggersMs[i] <= 0) {
            Utils::GeneratorParams params =
                mNoteSettings[gNote.pitchClass][i];
            float durMs =
                juce::jmap(params.duration, MIN_DURATION_MS, MAX_DURATION_MS);
            // Skip adding new grain if not enabled or full of grains
            if (gNote.positions[i].isActive &&
                mGrains.size() < MAX_GRAINS) {
              juce::Random random;
              GrainPositionFinder::GrainPosition& gPos =
                  gNote.positions[i];
              float durSamples =
                  mSampleRate * (durMs / 1000) * (1.0f / gPos.pbRate);
              float posSamples =
                  gPos.pitch.posRatio * mFileBuffer->getNumSamples();
              float posOffset = juce::jmap(
                  random.nextFloat(), 0.0f,
                  (gPos.pitch.duration * mFileBuffer->getNumSamples()) -
                      durSamples);
              posOffset +=
                  (params.posAdjust - 0.5f) * MAX_POS_ADJUST * durSamples;

              float rateGainFactor =
                  juce::jmap(1.0f - params.rate, MIN_RATE_RATIO * 2.0f, MAX_RATE_RATIO);
              float gain = gNote.ampEnvLevel * gPos.ampEnvLevel * params.gain * rateGainFactor;
              float pbRate = gPos.pbRate +
                             ((params.pitchAdjust - 0.5f) * MAX_PITCH_ADJUST);
              jassert(gPos.pbRate > 0.1f);
              auto grain =
                  Grain(generateGrainEnvelope(params.shape), durSamples, pbRate,
                        posSamples + posOffset, mTotalSamps, gain);
              mGrains.add(grain);
            }
            // Reset trigger ts
            gNote.grainTriggersMs[i] += juce::jmap(1.0f - params.rate, durMs * MIN_RATE_RATIO,
                           durMs * MAX_RATE_RATIO);
          } else {
            gNote.grainTriggersMs[i] -= waitMs;
          }
        }

        // Update amplitude envelope globally and for note
        updateEnvelopeState(gNote);
      }
    }

    wait(waitMs);
  }
}

void GranularSynth::setFileBuffer(juce::AudioBuffer<float>* buffer, double sr) {
  mFileBuffer = buffer;
  mSampleRate = sr;
}

void GranularSynth::setPitches(
    juce::HashMap<Utils::PitchClass, std::vector<PitchDetector::Pitch>>*
        pitches) {
  mPositionFinder.setPitches(pitches);
}

void GranularSynth::updateCurPositions() {
  if (mFileBuffer == nullptr) return;
  std::vector<GrainPositionFinder::GrainPosition> gPositions =
      mPositionFinder.findPositions(Utils::MAX_POSITIONS, mCurPitchClass);
  std::vector<GrainPositionFinder::GrainPosition> gPosToPlay;
  for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
    int position = mNoteSettings[mCurPitchClass][i].position;
    if (mNoteSettings[mCurPitchClass][i].isActive &&
        (gPositions.size() - 1) >= position) {
      gPositions[position].isActive = true;
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
  newPos = newPos % Utils::MAX_POSITIONS;
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
      gNote.envState = Utils::EnvelopeState::ATTACK;
      gNote.positions = mCurPositions;
      gNote.noteOnTs = mTotalSamps;
      gNote.noteOffTs = -1;
    }
  }
  if (!isPlayingAlready) {
    mActiveNotes.add(GrainNote(
        pitchClass,
        std::vector<Utils::GeneratorParams>(mNoteSettings[pitchClass].begin(),
                                            mNoteSettings[pitchClass].end()),
        mCurPositions, mTotalSamps));
  }
}

void GranularSynth::setNoteOff(Utils::PitchClass pitchClass) {
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == pitchClass) {
      // Set note off timestamp and set env state to release for each pos
      gNote.noteOffTs = mTotalSamps;
      gNote.envState = Utils::EnvelopeState::RELEASE;
      for (int i = 0; i < gNote.positions.size(); ++i) {
        gNote.positions[i].envState = Utils::EnvelopeState::RELEASE;
      }
      break;
    }
  }
}

void GranularSynth::updateGeneratorStates(std::vector<bool> genStates) {
  for (int i = 0; i < genStates.size(); ++i) {
    mNoteSettings[mCurPitchClass][i].isActive = genStates[i];
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
      mNoteSettings[i][j] = Utils::GeneratorParams(
          j == 0, j, PARAM_PITCH_DEFAULT, PARAM_POSITION_DEFAULT,
          PARAM_SHAPE_DEFAULT, PARAM_RATE_DEFAULT, PARAM_DURATION_DEFAULT,
          PARAM_GAIN_DEFAULT, PARAM_ATTACK_DEFAULT, PARAM_DECAY_DEFAULT,
          PARAM_SUSTAIN_DEFAULT, PARAM_RELEASE_DEFAULT);
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
      break;
    case ParameterType::DECAY:
      mNoteSettings[mCurPitchClass][colour].decay = value;
      break;
    case ParameterType::SUSTAIN:
      mNoteSettings[mCurPitchClass][colour].sustain = value;
      break;
    case ParameterType::RELEASE:
      mNoteSettings[mCurPitchClass][colour].release = value;
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
      break;
    case ParameterType::DECAY:
      mGlobalParams.decay = value;
      break;
    case ParameterType::SUSTAIN:
      mGlobalParams.sustain = value;
      break;
    case ParameterType::RELEASE:
      mGlobalParams.release = value;
      break;
    default:
      break;
  }
}

std::vector<float> GranularSynth::generateGrainEnvelope(float shape) {
  std::vector<float> grainEnv;
  // Each half: f(x) = 3nx(1-x)^2 + 3nx^2(1-x) + x^3
  for (int i = 0; i < 512; i++) {
    float x = (float)i / 512;
    float t = 3.0f * shape * x * std::pow(1.0f - x, 2.0f) +
              3.0f * shape * std::pow(x, 2.0f) * (1.0f - x) + std::pow(x, 3.0f);
    if (t <= 0.5f) {
      t *= 2.0f;
    } else {
      t = (1.0f - t) * 2.0f;
    }
    grainEnv.push_back(3.0f * shape * t * std::pow(1.0f - t, 2.0f) +
                       3.0f * shape * std::pow(t, 2.0f) * (1.0f - t) +
                       std::pow(t, 3.0f));
  }
  return grainEnv;
}

void GranularSynth::updateEnvelopeState(GrainNote& gNote) {
  // Update global env state
  float attackSamples =
      mSampleRate *
      juce::jmap(mGlobalParams.attack, MIN_ATTACK_SEC, MAX_ATTACK_SEC);
  float decaySamples = mSampleRate * juce::jmap(mGlobalParams.decay,
                                                MIN_DECAY_SEC, MAX_DECAY_SEC);
  switch (gNote.envState) {
    case Utils::EnvelopeState::ATTACK: {
      gNote.ampEnvLevel = juce::jlimit(
          0.0f, 1.0f, (mTotalSamps - gNote.noteOnTs) / (float)attackSamples);
      if (gNote.ampEnvLevel >= 1.0f) {
        gNote.envState = Utils::EnvelopeState::DECAY;
      }
      break;
    }
    case Utils::EnvelopeState::DECAY: {
      gNote.ampEnvLevel =
          juce::jlimit(mGlobalParams.sustain, 1.0f,
                       1.0f - ((mTotalSamps - attackSamples - gNote.noteOnTs) /
                               (float)(decaySamples)) *
                                  (1.0f - mGlobalParams.sustain));
      if (gNote.ampEnvLevel <= mGlobalParams.sustain) {
        gNote.envState = Utils::EnvelopeState::SUSTAIN;
      }
      break;
    }
    case Utils::EnvelopeState::SUSTAIN: {
      gNote.ampEnvLevel = mGlobalParams.sustain;
      // Note: setting note off sets state to release, we don't need to
      // here
      break;
    }
    case Utils::EnvelopeState::RELEASE: {
      float releaseSamples =
          mSampleRate *
          juce::jmap(mGlobalParams.release, MIN_RELEASE_SEC, MAX_RELEASE_SEC);
      gNote.ampEnvLevel = juce::jlimit(
          0.0f, mGlobalParams.sustain,
          mGlobalParams.sustain *
              (1.0f - (mTotalSamps - gNote.noteOffTs) / (float)releaseSamples));
      break;
    }
  }

  for (int i = 0; i < gNote.positions.size(); ++i) {
    // Update generator env state
    GrainPositionFinder::GrainPosition& gPos =
        gNote.positions[i];
    Utils::GeneratorParams genParams =
        mNoteSettings[gNote.pitchClass][i];
    attackSamples = mSampleRate * juce::jmap(genParams.attack, MIN_ATTACK_SEC,
                                             MAX_ATTACK_SEC);
    decaySamples =
        mSampleRate * juce::jmap(genParams.decay, MIN_DECAY_SEC, MAX_DECAY_SEC);
    switch (gPos.envState) {
      case Utils::EnvelopeState::ATTACK: {
        gPos.ampEnvLevel = juce::jlimit(
            0.0f, 1.0f, (mTotalSamps - gNote.noteOnTs) / (float)attackSamples);
        if (gPos.ampEnvLevel >= 1.0f) {
          gPos.envState = Utils::EnvelopeState::DECAY;
        }
        break;
      }
      case Utils::EnvelopeState::DECAY: {
        gPos.ampEnvLevel = juce::jlimit(
            genParams.sustain, 1.0f,
            1.0f - ((mTotalSamps - attackSamples - gNote.noteOnTs) /
                    (float)(decaySamples)) *
                       (1.0f - genParams.sustain));
        if (gPos.ampEnvLevel <= genParams.sustain) {
          gPos.envState = Utils::EnvelopeState::SUSTAIN;
        }
        break;
      }
      case Utils::EnvelopeState::SUSTAIN: {
        gPos.ampEnvLevel = genParams.sustain;
        // Note: setting note off sets state to release, we don't need to
        // here
        break;
      }
      case Utils::EnvelopeState::RELEASE: {
        float releaseSamples =
            mSampleRate *
            juce::jmap(genParams.release, MIN_RELEASE_SEC, MAX_RELEASE_SEC);
        gPos.ampEnvLevel = juce::jlimit(
            0.0f, genParams.sustain,
            genParams.sustain * (1.0f - (mTotalSamps - gNote.noteOffTs) /
                                            (float)releaseSamples));
        break;
      }
    }
  }
  
}

long GranularSynth::getMaxReleaseTime(GrainNote& gNote) {
  int curMaxSamples = 0;
  for (int i = 0; i < mNoteSettings[gNote.pitchClass].size(); ++i) {
    int releaseSamples =
        mSampleRate * juce::jmap(mNoteSettings[gNote.pitchClass][i].release,
                                 MIN_RELEASE_SEC, MAX_RELEASE_SEC);
    if (releaseSamples > curMaxSamples) {
      curMaxSamples = releaseSamples;
    }
  }
  return curMaxSamples;
}