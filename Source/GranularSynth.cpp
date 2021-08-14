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
      mFft(FFT_SIZE, HOP_SIZE) {
  mNoteParams.addParams(*this);
  mGlobalParams.addParams(*this);

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
        createCandidates(mPitchDetector.getPitches());
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
      float noteGain = gNote.ampEnv.getAmplitude(
          mTotalSamps, mGlobalParams.attack->get() * mSampleRate,
          mGlobalParams.decay->get() * mSampleRate,
          mGlobalParams.sustain->get(),
          mGlobalParams.release->get() * mSampleRate);
      // Add contributions from each grain
      for (Grain& grain : gNote.grains) {
        GeneratorParams* genParams = mNoteParams.notes[gNote.pitchClass]
                                         ->generators[grain.generator]
                                         .get();
        float genGain = gNote.genAmpEnvs[grain.generator].getAmplitude(
            mTotalSamps, genParams->attack->get() * mSampleRate,
            genParams->decay->get() * mSampleRate, genParams->sustain->get(),
            genParams->release->get() * mSampleRate);
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
  juce::AudioProcessorEditor* editor = new GRainbowAudioProcessorEditor(*this);

  // Init anything in main editor post construction
  reinterpret_cast<GRainbowAudioProcessorEditor*>(editor)->fastDebugMode();

  return editor;
}

//==============================================================================
void GranularSynth::getStateInformation(juce::MemoryBlock& destData) {
  juce::XmlElement xml("PluginState");

  juce::XmlElement* params = new juce::XmlElement("AudioParams");
  for (auto& param : getParameters())
    params->setAttribute(ParamHelper::getParamID(param), param->getValue());

  xml.addChildElement(params);
  xml.addChildElement(mUIParams.getXml());

  copyXmlToBinary(xml, destData);
}

void GranularSynth::setStateInformation(const void* data,
                                                 int sizeInBytes) {
  auto xml = getXmlFromBinary(data, sizeInBytes);

  if (xml != nullptr) {
    auto params = xml->getChildByName("AudioParams");
    if (params != nullptr) {
      for (auto& param : getParameters())
        param->setValueNotifyingHost(params->getDoubleAttribute(
            ParamHelper::getParamID(param), param->getValue()));
    }
    mUIParams = UIParams(xml->getChildByName("UIParams"));
  }
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
          GeneratorParams* genParams = mNoteParams.notes[gNote.pitchClass]->generators[i].get();
          CandidateParams* candidateParams =
              mNoteParams.notes[gNote.pitchClass]
                  ->candidates[genParams->candidate->get()].get();
            float durMs = genParams->grainDuration->get();
          // Skip adding new grain if not enabled or full of grains
            if (candidateParams->valid->get() &&
                mNoteParams.notes[gNote.pitchClass]->shouldPlayGenerator(i) &&
                gNote.grains.size() < MAX_GRAINS) {
            float durSamples = mSampleRate * (durMs / 1000) *
                               (1.0f / candidateParams->pbRate->get());
            /* Commented out random spray for now to test consistency
            juce::Random random;

             = juce::jmap(random.nextFloat(), 0.0f,
                                         (candidateParams->duration->get() *
                                          mFileBuffer.getNumSamples()) -
                                             durSamples); */
            float posSamples =
                candidateParams->posRatio->get() * mFileBuffer.getNumSamples();
            float posOffset = genParams->positionAdjust->get() * durSamples;

            // TODO: normalize the gain or something
            float gain = genParams->grainGain->get();
            float pbRate =
                candidateParams->pbRate->get() + genParams->pitchAdjust->get();
            jassert(candidateParams->pbRate->get() > 0.1f);
            auto grain =
                Grain((Utils::GeneratorColour)i, genParams->grainEnv, durSamples,
                      pbRate, posSamples + posOffset, mTotalSamps, gain);
            gNote.grains.add(grain);
          }
          // Reset trigger ts
          gNote.grainTriggers[i] +=
              mSampleRate / 1000 *
              juce::jmap(1.0f - genParams->grainRate->get(),
                         durMs * MIN_RATE_RATIO, durMs * MAX_RATE_RATIO);
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

int GranularSynth::incrementPosition(int boxNum, bool lookRight) {
  int pos = mNoteParams.notes[mCurPitchClass]->generators[boxNum]->candidate->get();
  int newPos = lookRight ? pos + 1 : pos - 1;
  newPos = (newPos + Utils::MAX_CANDIDATES) % Utils::MAX_CANDIDATES;
  ParamHelper::setParam(mNoteParams.notes[mCurPitchClass]
      ->generators[boxNum]
      ->candidate, newPos);
  return newPos;
}

std::vector<CandidateParams*> GranularSynth::getActiveCandidates() {
  std::vector<CandidateParams*> candidates;
  for (int i = 0; i < NUM_GENERATORS; ++i) {
    if (mNoteParams.notes[mCurPitchClass]->shouldPlayGenerator(i)) {
      int candidate =
          mNoteParams.notes[mCurPitchClass]->generators[i]->candidate->get();
      candidates.push_back(
          mNoteParams.notes[mCurPitchClass]->candidates[candidate].get());
    } else {
      candidates.push_back(nullptr);
    }
  }
  return candidates;
}

void GranularSynth::setNoteOn(Utils::PitchClass pitchClass) {
  mCurPitchClass = pitchClass;
  bool isPlayingAlready = false;
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == pitchClass) {
      isPlayingAlready = true;
      // Start envelope over
      gNote.ampEnv.noteOn(mTotalSamps);
      // Start position envs over as well
      for (Utils::EnvelopeADSR& genEnv : gNote.genAmpEnvs) {
        genEnv.noteOn(mTotalSamps);
      }
    }
  }
  if (!isPlayingAlready) {
    mActiveNotes.add(GrainNote(
        pitchClass,
        Utils::EnvelopeADSR(mTotalSamps)));
  }
}

void GranularSynth::setNoteOff(Utils::PitchClass pitchClass) {
  for (GrainNote& gNote : mActiveNotes) {
    if (gNote.pitchClass == pitchClass) {
      // Set note off timestamp and set env state to release for each pos
      gNote.ampEnv.noteOff(mTotalSamps);
      for (Utils::EnvelopeADSR& genEnv : gNote.genAmpEnvs) {
        genEnv.noteOff(mTotalSamps);
      }
      break;
    }
  }
}

void GranularSynth::resetParameters() {
  mNoteParams.resetParams();
  mGlobalParams.resetParams();
}

void GranularSynth::createCandidates(
    juce::HashMap<Utils::PitchClass, std::vector<PitchDetector::Pitch>>&
        detectedPitches) {
  // Add candidates for each pitch class
  // TODO: check salience instead of blindly adding
  for (auto&& note : mNoteParams.notes) {
    // Look for detected pitches with correct pitch and good gain
    bool foundAll = false;
    int numFound = 0;
    int numSearches = 0;

    while (!foundAll) {
      int noteMin = note->noteIdx - numSearches;
      int noteMax = note->noteIdx + numSearches;
      // Check low note
      std::vector<PitchDetector::Pitch>& pitchVec =
          detectedPitches.getReference((Utils::PitchClass)(noteMin % 12));
      float pbRate = std::pow(Utils::TIMESTRETCH_RATIO, numSearches);
      for (int i = 0; i < pitchVec.size(); ++i) {
        if (pitchVec[i].gain < MIN_CANDIDATE_SALIENCE) continue;
        ParamHelper::setParam(note->candidates[numFound]->valid, true);
        ParamHelper::setParam(note->candidates[numFound]->posRatio,
                              pitchVec[i].posRatio);
        ParamHelper::setParam(note->candidates[numFound]->pbRate, pbRate);
        ParamHelper::setParam(note->candidates[numFound]->duration,
                              pitchVec[i].duration);
        ParamHelper::setParam(note->candidates[numFound]->salience,
                              pitchVec[i].gain);
        numFound++;
        if (numFound >= MAX_CANDIDATES) {
          foundAll = true;
          break;
        }
      }
      // Check high note if we haven't filled up the list yet
      if (!foundAll && numSearches > 0) {
        std::vector<PitchDetector::Pitch>& pitchVec =
            detectedPitches.getReference((Utils::PitchClass)(noteMax % 12));
        float pbRate = std::pow(Utils::TIMESTRETCH_RATIO, -numSearches);
        for (int i = 0; i < pitchVec.size(); ++i) {
          if (pitchVec[i].gain < MIN_CANDIDATE_SALIENCE) continue;
          ParamHelper::setParam(note->candidates[numFound]->valid, true);
          ParamHelper::setParam(note->candidates[numFound]->posRatio, pitchVec[i].posRatio);
          ParamHelper::setParam(note->candidates[numFound]->pbRate, pbRate);
          ParamHelper::setParam(note->candidates[numFound]->duration, pitchVec[i].duration);
          ParamHelper::setParam(note->candidates[numFound]->salience, pitchVec[i].gain);
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
  }
}