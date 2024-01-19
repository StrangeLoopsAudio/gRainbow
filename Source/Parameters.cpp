/*
  ==============================================================================

    Parameters.cpp
    Created: 10 Aug 2021 6:47:57pm
    Author:  brady

  ==============================================================================
*/

#include "Parameters.h"

void Parameters::prepareModSources(int blockSize, double sampleRate) {
  for (auto& lfo : global.modLFOs) {
    lfo.prepare(blockSize, sampleRate);
  }
  for (auto& env : global.modEnvs) {
    env.prepare(blockSize, sampleRate);
  }
  for (auto& macro : global.macros) {
    macro.prepare(blockSize, sampleRate);
  }
}
void Parameters::processModSources() {
  for (auto& lfo : global.modLFOs) {
    lfo.processBlock();
  }
  for (auto& env : global.modEnvs) {
    env.processBlock();
  }
  for (auto& macro : global.macros) {
    macro.processBlock();
  }
}
void Parameters::applyModulations(juce::RangedAudioParameter* param, float& value0To1) {
  const int idx = param->getParameterIndex();
  if (param && modulations.contains(idx)) {
    Modulation& mod = modulations.getReference(idx);
    if (mod.source) {
      value0To1 = juce::jlimit(0.0f, 1.0f, value0To1 + (mod.depth * mod.source->getOutput()));
    }
  }
}

// Returns the candidate used in a given generator
ParamCandidate* Parameters::getGeneratorCandidate(ParamGenerator* gen) {
  if (gen == nullptr) return nullptr;
  return &note.notes[gen->noteIdx]->candidates[gen->candidate->get()];
}

// Returns the currently selected pitch class, or NONE if global selected
Utils::PitchClass Parameters::getSelectedPitchClass() {
  switch (getSelectedParams()->type) {
    case ParamType::GLOBAL:
      return Utils::PitchClass::NONE;
    case ParamType::NOTE:
      return (Utils::PitchClass) dynamic_cast<ParamNote*>(getSelectedParams())->noteIdx;
    case ParamType::GENERATOR:
      ParamGenerator* gen = dynamic_cast<ParamGenerator*>(getSelectedParams());
      return (Utils::PitchClass)gen->noteIdx;
  }
  return Utils::PitchClass::NONE;
}
// Keeps track of the current selected global/note/generator parameters for editing, global by default
juce::Colour Parameters::getSelectedParamColour() {
  switch (getSelectedParams()->type) {
    case ParamType::GLOBAL:
      return Utils::GLOBAL_COLOUR;
    case ParamType::NOTE:
      return Utils::getRainbow12Colour(dynamic_cast<ParamNote*>(getSelectedParams())->noteIdx);
    case ParamType::GENERATOR:
      ParamGenerator* gen = dynamic_cast<ParamGenerator*>(getSelectedParams());
      return Utils::getRainbow12Colour(gen->noteIdx);
  }
  return juce::Colours::black;
}

juce::RangedAudioParameter* Parameters::getUsedParam(ParamCommon* common, ParamCommon::Type type) {
  const ParamGenerator* pGen = dynamic_cast<ParamGenerator*>(common);
  ParamNote* pNote = dynamic_cast<ParamNote*>(common);
  juce::RangedAudioParameter* param = nullptr;
  bool keepLooking = true;
  if (pGen) {
    // If gen param is used, use it
    if (pGen->isUsed[type]) {
      param = pGen->common[type];
      keepLooking = false;
    }
    pNote = note.notes[pGen->noteIdx].get();
  }
  if (pNote && keepLooking) {
    // Otherwise if note param is used, use it
    if (pNote->isUsed[type]) {
      param = pNote->common[type];
      keepLooking = false;
    }
  }
  if (keepLooking) {
    // If neither used, just use global param
    param = global.common[type];
  }
  jassert(param);
  return param;
}

void Parameters::addListener(Parameters::Listener* listener)
{
  mListeners.add(listener);
}

void Parameters::removeListener(Parameters::Listener* listener)
{
  mListeners.remove(listener);
}

// Finds the lowest level parameter that's different from its parent
// Hierarchy (high to low): global, note, generator
// Optionally applies modulations before returning value
float Parameters::getFloatParam(ParamCommon* common, ParamCommon::Type type, bool withModulations) {
  juce::RangedAudioParameter* param = getUsedParam(common, type);
  return getFloatParam(P_FLOAT(param), withModulations);
}
float Parameters::getFloatParam(juce::AudioParameterFloat* param, bool withModulations) {
  float value0To1 = param->convertTo0to1(param->get());
  if (withModulations) applyModulations(param, value0To1);
  return param->convertFrom0to1(value0To1);
}
int Parameters::getIntParam(ParamCommon* common, ParamCommon::Type type, bool withModulations) {
  juce::RangedAudioParameter* param = getUsedParam(common, type);
  return getIntParam(P_INT(param), withModulations);
}
int Parameters::getIntParam(juce::AudioParameterInt* param, bool withModulations) {
  float value0To1 = param->convertTo0to1(param->get());
  if (withModulations) applyModulations(param, value0To1);
  return param->convertFrom0to1(value0To1);
}
int Parameters::getChoiceParam(ParamCommon* common, ParamCommon::Type type) {
  juce::RangedAudioParameter* param = getUsedParam(common, type);
  return P_CHOICE(param)->getIndex();
}
bool Parameters::getBoolParam(ParamCommon* common, ParamCommon::Type type) {
  juce::RangedAudioParameter* param = getUsedParam(common, type);
  return P_BOOL(param)->get();
}

// Parameter classes init
void ParamGlobal::addParams(juce::AudioProcessor& p) {
  // Global amp env
  p.addParameter(ampEnvAttack = new juce::AudioParameterFloat({ParamIDs::ampEnvAttack, 1}, "Amp Env Attack",
                                                                   ParamRanges::ATTACK, ParamDefaults::ATTACK_DEFAULT_SEC));
  p.addParameter(ampEnvDecay = new juce::AudioParameterFloat({ParamIDs::ampEnvDecay, 1}, "Amp Env Decay",
                                                                  ParamRanges::DECAY, ParamDefaults::DECAY_DEFAULT_SEC));
  p.addParameter(ampEnvSustain = new juce::AudioParameterFloat({ParamIDs::ampEnvSustain, 1}, "Amp Env Sustain",
                                                                    ParamRanges::SUSTAIN, ParamDefaults::SUSTAIN_DEFAULT));
  p.addParameter(ampEnvRelease = new juce::AudioParameterFloat({ParamIDs::ampEnvRelease, 1}, "Amp Env Release",
                                                                    ParamRanges::RELEASE, ParamDefaults::RELEASE_DEFAULT_SEC));
  // Modulators
  // LFOs
  for (int i = 0; i < modLFOs.size(); ++i) {
    auto strI = juce::String(i);
    p.addParameter(modLFOs[i].shape = new juce::AudioParameterChoice({ParamIDs::lfoShape + strI, 1}, "LFO " + strI + " Shape",
                                                                     LFO_SHAPE_NAMES, ParamDefaults::LFO_SHAPE_DEFAULT));
    p.addParameter(modLFOs[i].rate = new juce::AudioParameterFloat({ParamIDs::lfoRate + strI, 1}, "LFO " + strI + " Rate",
                                                                   ParamRanges::LFO_RATE, ParamDefaults::LFO_RATE_DEFAULT));
    p.addParameter(modLFOs[i].phase = new juce::AudioParameterFloat({ParamIDs::lfoPhase + strI, 1}, "LFO " + strI + " Phase",
                                                                    ParamRanges::LFO_PHASE, ParamDefaults::LFO_PHASE_DEFAULT));
    p.addParameter(modLFOs[i].sync = new juce::AudioParameterBool({ParamIDs::lfoSync + strI, 1}, "LFO " + strI + " Sync",
                                                                  ParamDefaults::LFO_SYNC_DEFAULT));
    p.addParameter(modLFOs[i].bipolar = new juce::AudioParameterBool({ParamIDs::lfoBipolar + strI, 1}, "LFO " + strI + " Bipolar",
                                                                     ParamDefaults::LFO_BIPOLAR_DEFAULT));
    p.addParameter(modLFOs[i].retrigger = new juce::AudioParameterBool(
                                                                       {ParamIDs::lfoRetrigger + strI, 1}, "LFO " + strI + " Retrigger", ParamDefaults::LFO_RETRIGGER_DEFAULT));
  }
  // Mod envelopes
  for (int i = 0; i < modEnvs.size(); ++i) {
    auto strI = juce::String(i);
    p.addParameter(modEnvs[i].attack = new juce::AudioParameterFloat({ParamIDs::modEnvAttack + strI, 1}, "Env " + strI + " Attack",
                                                                     ParamRanges::ATTACK, ParamDefaults::ATTACK_DEFAULT_SEC));
    p.addParameter(modEnvs[i].decay = new juce::AudioParameterFloat({ParamIDs::modEnvDecay + strI, 1}, "Env " + strI + " Decay",
                                                                    ParamRanges::DECAY, ParamDefaults::DECAY_DEFAULT_SEC));
    p.addParameter(modEnvs[i].sustain = new juce::AudioParameterFloat({ParamIDs::modEnvSustain + strI, 1}, "Env " + strI + " Sustain",
                                                                      ParamRanges::SUSTAIN, ParamDefaults::SUSTAIN_DEFAULT));
    p.addParameter(modEnvs[i].release = new juce::AudioParameterFloat({ParamIDs::modEnvRelease + strI, 1}, "Env " + strI + " Release",
                                                                      ParamRanges::RELEASE, ParamDefaults::RELEASE_DEFAULT_SEC));
  }
  // Macros
  for (int i = 0; i < macros.size(); ++i) {
    auto strI = juce::String(i);
    p.addParameter(macros[i].macro = new juce::AudioParameterFloat({ParamIDs::macro + strI, 1}, "Macro " + strI, ParamRanges::MACRO,
                                                                   ParamDefaults::MACRO_DEFAULT));
  }
  // Common
  p.addParameter(common[GAIN] = new juce::AudioParameterFloat({ParamIDs::globalGain, 1}, "Master Gain", ParamRanges::GAIN,
                                                              ParamDefaults::GAIN_DEFAULT));
  p.addParameter(common[GRAIN_SHAPE] = new juce::AudioParameterFloat({ParamIDs::globalGrainShape, 1}, "Master Grain Shape",
                                                                     ParamRanges::GRAIN_SHAPE, ParamDefaults::GRAIN_SHAPE_DEFAULT));
  p.addParameter(common[GRAIN_TILT] = new juce::AudioParameterFloat({ParamIDs::globalGrainTilt, 1}, "Master Grain Tilt",
                                                                    ParamRanges::GRAIN_TILT, ParamDefaults::GRAIN_TILT_DEFAULT));

  p.addParameter(common[GRAIN_RATE] = new juce::AudioParameterFloat({ParamIDs::globalGrainRate, 1}, "Master Grain Rate",
                                                                    ParamRanges::GRAIN_RATE, ParamDefaults::GRAIN_RATE_DEFAULT));
  p.addParameter(common[GRAIN_DURATION] =
                     new juce::AudioParameterFloat({ParamIDs::globalGrainDuration, 1}, "Master Grain Duration",
                                                   ParamRanges::GRAIN_DURATION, ParamDefaults::GRAIN_DURATION_DEFAULT));
  p.addParameter(common[GRAIN_SYNC] = new juce::AudioParameterBool({ParamIDs::globalGrainSync, 1}, "Master Grain Sync", ParamDefaults::GRAIN_SYNC_DEFAULT));

  p.addParameter(common[PITCH_ADJUST] =
                     new juce::AudioParameterFloat({ParamIDs::globalPitchAdjust, 1}, "Master Pitch Adjust", ParamRanges::PITCH_ADJUST,
                                                   ParamDefaults::PITCH_ADJUST_DEFAULT));
  p.addParameter(common[PITCH_SPRAY] = new juce::AudioParameterFloat({ParamIDs::globalPitchSpray, 1}, "Master Pitch Spray",
                                                                     ParamRanges::PITCH_SPRAY, ParamDefaults::PITCH_SPRAY_DEFAULT));
  p.addParameter(common[POS_ADJUST] =
                     new juce::AudioParameterFloat({ParamIDs::globalPositionAdjust, 1}, "Master Position Adjust",
                                                   ParamRanges::POSITION_ADJUST, ParamDefaults::POSITION_ADJUST_DEFAULT));
  p.addParameter(common[POS_SPRAY] =
                     new juce::AudioParameterFloat({ParamIDs::globalPositionSpray, 1}, "Master Position Spray",
                                                   ParamRanges::POSITION_SPRAY, ParamDefaults::POSITION_SPRAY_DEFAULT));

  p.addParameter(common[PAN_ADJUST] = new juce::AudioParameterFloat({ParamIDs::globalPanAdjust, 1}, "Master Pan Adjust",
                                                                    ParamRanges::PAN_ADJUST, ParamDefaults::PAN_ADJUST_DEFAULT));
  p.addParameter(common[PAN_SPRAY] = new juce::AudioParameterFloat({ParamIDs::globalPanSpray, 1}, "Master Pan Spray",
                                                                   ParamRanges::PAN_SPRAY, ParamDefaults::PAN_SPRAY_DEFAULT));
  p.addParameter(common[REVERSE] = new juce::AudioParameterBool({ParamIDs::globalReverse, 1}, ParamIDs::globalReverse, ParamDefaults::REVERSE_DEFAULT));
  p.addParameter(common[OCTAVE_ADJUST] = new juce::AudioParameterInt({ParamIDs::globalOctaveAdjust, 1}, "Master Octave Adjust", ParamRanges::OCTAVE_ADJUST.start, ParamRanges::OCTAVE_ADJUST.end, ParamDefaults::OCTAVE_ADJUST_DEFAULT));
}

void ParamGenerator::addParams(juce::AudioProcessor& p) {
  juce::String enableId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genEnable + juce::String(genIdx);
  p.addParameter(enable = new juce::AudioParameterBool({enableId, 1}, enableId, true));
  juce::String candidateId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genCandidate + juce::String(genIdx);
  p.addParameter(candidate = new juce::AudioParameterInt({candidateId, 1}, candidateId, 0, MAX_CANDIDATES - 1, 0));

  juce::String gainId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGain + juce::String(genIdx);
  p.addParameter(common[GAIN] = new juce::AudioParameterFloat({gainId, 1}, gainId, ParamRanges::GAIN, ParamDefaults::GAIN_DEFAULT));
  juce::String pitchAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPitchAdjust + juce::String(genIdx);
  p.addParameter(common[PITCH_ADJUST] = new juce::AudioParameterFloat({pitchAdjustId, 1}, pitchAdjustId, ParamRanges::PITCH_ADJUST,
                                                                      ParamDefaults::PITCH_ADJUST_DEFAULT));
  juce::String pitchSprayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPitchSpray + juce::String(genIdx);
  p.addParameter(common[PITCH_SPRAY] = new juce::AudioParameterFloat({pitchSprayId, 1}, pitchSprayId, ParamRanges::PITCH_SPRAY,
                                                                     ParamDefaults::PITCH_SPRAY_DEFAULT));
  juce::String posAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPositionAdjust + juce::String(genIdx);
  p.addParameter(common[POS_ADJUST] = new juce::AudioParameterFloat({posAdjustId, 1}, posAdjustId, ParamRanges::POSITION_ADJUST,
                                                                    ParamDefaults::POSITION_ADJUST_DEFAULT));
  juce::String posSprayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPositionSpray + juce::String(genIdx);
  p.addParameter(common[POS_SPRAY] = new juce::AudioParameterFloat({posSprayId, 1}, posSprayId, ParamRanges::POSITION_SPRAY,
                                                                   ParamDefaults::POSITION_SPRAY_DEFAULT));
  juce::String panAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPanAdjust + juce::String(genIdx);
  p.addParameter(common[PAN_ADJUST] = new juce::AudioParameterFloat({panAdjustId, 1}, panAdjustId, ParamRanges::PAN_ADJUST,
                                                                    ParamDefaults::PAN_ADJUST_DEFAULT));
  juce::String panSprayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPanSpray + juce::String(genIdx);
  p.addParameter(common[PAN_SPRAY] = new juce::AudioParameterFloat({panSprayId, 1}, panSprayId, ParamRanges::PAN_SPRAY,
                                                                   ParamDefaults::PAN_SPRAY_DEFAULT));

  juce::String shapeId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainShape + juce::String(genIdx);
  p.addParameter(common[GRAIN_SHAPE] =
                     new juce::AudioParameterFloat({shapeId, 1}, shapeId, ParamRanges::GRAIN_SHAPE, ParamDefaults::GRAIN_SHAPE_DEFAULT));
  juce::String tiltId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainTilt + juce::String(genIdx);
  p.addParameter(common[GRAIN_TILT] =
                     new juce::AudioParameterFloat({tiltId, 1}, tiltId, ParamRanges::GRAIN_TILT, ParamDefaults::GRAIN_TILT_DEFAULT));

  juce::String rateId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainRate + juce::String(genIdx);
  p.addParameter(common[GRAIN_RATE] =
                     new juce::AudioParameterFloat({rateId, 1}, rateId, ParamRanges::GRAIN_RATE, ParamDefaults::GRAIN_RATE_DEFAULT));
  juce::String durationId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainDuration + juce::String(genIdx);
  p.addParameter(common[GRAIN_DURATION] = new juce::AudioParameterFloat({durationId, 1}, durationId, ParamRanges::GRAIN_DURATION,
                                                                        ParamDefaults::GRAIN_DURATION_DEFAULT));
  juce::String syncId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainSync + juce::String(genIdx);
  p.addParameter(common[GRAIN_SYNC] = new juce::AudioParameterBool({syncId, 1}, syncId, ParamDefaults::GRAIN_SYNC_DEFAULT));
  juce::String reverseId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genReverse + juce::String(genIdx);
  p.addParameter(common[REVERSE] = new juce::AudioParameterBool({reverseId, 1}, reverseId, ParamDefaults::REVERSE_DEFAULT));
  juce::String octaveAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genOctaveAdjust + juce::String(genIdx);
  p.addParameter(common[OCTAVE_ADJUST] = new juce::AudioParameterInt({octaveAdjustId, 1}, octaveAdjustId, ParamRanges::OCTAVE_ADJUST.start, ParamRanges::OCTAVE_ADJUST.end, ParamDefaults::OCTAVE_ADJUST_DEFAULT));
}

void ParamNote::addParams(juce::AudioProcessor& p) {
  juce::String notePrefix = PITCH_CLASS_NAMES[noteIdx];

  // First the note's common parameters
  p.addParameter(common[GAIN] = new juce::AudioParameterFloat({notePrefix + ParamIDs::noteGain, 1}, notePrefix + ParamIDs::noteGain,
                                                              ParamRanges::GAIN, ParamDefaults::GAIN_DEFAULT));
  
  p.addParameter(common[GRAIN_SHAPE] = new juce::AudioParameterFloat({notePrefix + ParamIDs::noteGrainShape, 1}, notePrefix + ParamIDs::noteGrainShape,
                                                   ParamRanges::GRAIN_SHAPE, ParamDefaults::GRAIN_SHAPE_DEFAULT));
  p.addParameter(common[GRAIN_TILT] = new juce::AudioParameterFloat({notePrefix + ParamIDs::noteGrainTilt, 1}, notePrefix + ParamIDs::noteGrainTilt,
                                                   ParamRanges::GRAIN_TILT, ParamDefaults::GRAIN_TILT_DEFAULT));

  p.addParameter(common[GRAIN_RATE] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::noteGrainRate, 1}, notePrefix + ParamIDs::noteGrainRate,
                                                   ParamRanges::GRAIN_RATE, ParamDefaults::GRAIN_RATE_DEFAULT));
  p.addParameter(common[GRAIN_DURATION] = new juce::AudioParameterFloat({
                     notePrefix + ParamIDs::noteGrainDuration, 1}, notePrefix + ParamIDs::noteGrainDuration,
                     ParamRanges::GRAIN_DURATION, ParamDefaults::GRAIN_DURATION_DEFAULT));
  p.addParameter(common[GRAIN_SYNC] = new juce::AudioParameterBool({notePrefix + ParamIDs::noteGrainSync, 1},
                                                                   notePrefix + ParamIDs::noteGrainSync, ParamDefaults::GRAIN_SYNC_DEFAULT));

  p.addParameter(common[PITCH_ADJUST] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::notePitchAdjust, 1}, notePrefix + ParamIDs::notePitchAdjust,
                                                   ParamRanges::PITCH_ADJUST, ParamDefaults::PITCH_ADJUST_DEFAULT));
  p.addParameter(common[PITCH_SPRAY] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::notePitchSpray, 1}, notePrefix + ParamIDs::notePitchSpray,
                                                   ParamRanges::PITCH_SPRAY, ParamDefaults::PITCH_SPRAY_DEFAULT));
  p.addParameter(common[POS_ADJUST] = new juce::AudioParameterFloat({
                     notePrefix + ParamIDs::notePositionAdjust, 1}, notePrefix + ParamIDs::notePositionAdjust,
                     ParamRanges::POSITION_ADJUST, ParamDefaults::POSITION_ADJUST_DEFAULT));
  p.addParameter(common[POS_SPRAY] = new juce::AudioParameterFloat({
                     notePrefix + ParamIDs::notePositionSpray, 1}, notePrefix + ParamIDs::notePositionSpray,
                     ParamRanges::POSITION_SPRAY, ParamDefaults::POSITION_SPRAY_DEFAULT));
  p.addParameter(common[PAN_ADJUST] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::notePanAdjust, 1}, notePrefix + ParamIDs::notePanAdjust,
                                                   ParamRanges::PAN_ADJUST, ParamDefaults::PAN_ADJUST_DEFAULT));
  p.addParameter(common[PAN_SPRAY] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::notePanSpray, 1}, notePrefix + ParamIDs::notePanSpray,
                                                   ParamRanges::PAN_SPRAY, ParamDefaults::PAN_SPRAY_DEFAULT));
  p.addParameter(common[REVERSE] = new juce::AudioParameterBool({notePrefix + ParamIDs::noteReverse, 1}, notePrefix + ParamIDs::noteReverse, ParamDefaults::REVERSE_DEFAULT));
  p.addParameter(common[OCTAVE_ADJUST] = new juce::AudioParameterInt({notePrefix + ParamIDs::noteOctaveAdjust, 1}, notePrefix + ParamIDs::noteOctaveAdjust, ParamRanges::OCTAVE_ADJUST.start, ParamRanges::OCTAVE_ADJUST.end, ParamDefaults::OCTAVE_ADJUST_DEFAULT));

  // Then make each of its generators
  for (auto& generator : generators) {
    generator->addParams(p);
  }
  juce::String soloId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genSolo;
  p.addParameter(soloIdx = new juce::AudioParameterInt({soloId, 1}, soloId, SOLO_NONE, NUM_GENERATORS - 1, SOLO_NONE));
}

ParamCandidate* ParamNote::getCandidate(int genIdx) {
  if (candidates.size() <= genIdx) return nullptr;
  return &candidates[generators[genIdx]->candidate->get()];
}

void ParamNote::setStartingCandidatePosition() {
  // We start each candidate position at zero and here update it to start each generator at a unique position.
  // If there are only 2 candidate and 4 generators, we want genIdx 3 and 4 to also get candidate[1]
  const int maxPosition = candidates.size() - 1;
  for (int i = 0; i < NUM_GENERATORS; ++i) {
    ParamHelper::setParam(generators[i].get()->candidate, (i > maxPosition) ? maxPosition : i);
  }
}

bool ParamNote::shouldPlayGenerator(int genIdx) {
  return (soloIdx->get() == genIdx) || (generators[genIdx]->enable->get() && soloIdx->get() == SOLO_NONE);
}

void ParamsNote::addParams(juce::AudioProcessor& p) {
  for (auto& note : notes) {
    note->addParams(p);
  }
}
