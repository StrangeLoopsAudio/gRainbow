/*
  ==============================================================================

    Parameters.cpp
    Created: 10 Aug 2021 6:47:57pm
    Author:  brady

  ==============================================================================
*/

#include "Parameters.h"

void ParamGlobal::addParams(juce::AudioProcessor& p) {
  p.addParameter(gain = new juce::AudioParameterFloat(ParamIDs::globalGain, "Master Gain",
                                                      juce::NormalisableRange<float>(0.0f, 1.0f), ParamDefaults::GAIN_DEFAULT));
  p.addParameter(attack = new juce::AudioParameterFloat(ParamIDs::globalAttack, "Master Attack", ParamRanges::ATTACK,
                                                        ParamDefaults::ATTACK_DEFAULT_SEC));
  p.addParameter(decay = new juce::AudioParameterFloat(ParamIDs::globalDecay, "Master Decay", ParamRanges::DECAY,
                                                       ParamDefaults::DECAY_DEFAULT_SEC));
  p.addParameter(sustain =
                     new juce::AudioParameterFloat(ParamIDs::globalSustain, "Master Sustain",
                                                   juce::NormalisableRange<float>(0.0f, 1.0f), ParamDefaults::SUSTAIN_DEFAULT));
  p.addParameter(release = new juce::AudioParameterFloat(ParamIDs::globalRelease, "Master Release", ParamRanges::RELEASE,
                                                         ParamDefaults::RELEASE_DEFAULT_SEC));
}

void ParamGlobal::resetParams() {
  ParamHelper::setParam(gain, ParamDefaults::GAIN_DEFAULT);
  ParamHelper::setParam(attack, ParamDefaults::ATTACK_DEFAULT_SEC);
  ParamHelper::setParam(decay, ParamDefaults::DECAY_DEFAULT_SEC);
  ParamHelper::setParam(sustain, ParamDefaults::SUSTAIN_DEFAULT);
  ParamHelper::setParam(release, ParamDefaults::RELEASE_DEFAULT_SEC);
}

void ParamGenerator::addParams(juce::AudioProcessor& p) {
  juce::String enableId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genEnable + juce::String(genIdx);
  p.addParameter(enable = new juce::AudioParameterBool(enableId, enableId, genIdx == 0));
  juce::String gainId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGain + juce::String(genIdx);
  p.addParameter(gain = new juce::AudioParameterFloat(gainId, gainId, juce::NormalisableRange<float>(0.0f, 1.0f),
                                                      ParamDefaults::GAIN_DEFAULT));
  juce::String candidateId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genCandidate + juce::String(genIdx);
  p.addParameter(candidate = new juce::AudioParameterInt(candidateId, candidateId, 0, MAX_CANDIDATES - 1, 0));
  juce::String pitchAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPitchAdjust + juce::String(genIdx);
  p.addParameter(pitchAdjust = new juce::AudioParameterFloat(pitchAdjustId, pitchAdjustId, ParamRanges::PITCH_ADJUST, 0.0f));
  juce::String pitchSprayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPitchSpray + juce::String(genIdx);
  p.addParameter(pitchSpray = new juce::AudioParameterFloat(pitchSprayId, pitchSprayId, ParamRanges::PITCH_SPRAY, 0.0f));
  juce::String posAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPositionAdjust + juce::String(genIdx);
  p.addParameter(positionAdjust = new juce::AudioParameterFloat(posAdjustId, posAdjustId, ParamRanges::POSITION_ADJUST, 0.0f));
  juce::String posSprayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPositionSpray + juce::String(genIdx);
  p.addParameter(positionSpray = new juce::AudioParameterFloat(posSprayId, posSprayId, ParamRanges::POSITION_SPRAY,
                                                               ParamDefaults::POSITION_SPRAY_DEFAULT));

  // Shape and Tilt have listeners as changing then will change the envolope LUT
  juce::String shapeId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainShape + juce::String(genIdx);
  p.addParameter(grainShape = new juce::AudioParameterFloat(shapeId, shapeId, juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
  grainShape->addListener(this);
  juce::String tiltId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainTilt + juce::String(genIdx);
  p.addParameter(grainTilt = new juce::AudioParameterFloat(tiltId, tiltId, juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
  grainTilt->addListener(this);

  juce::String rateId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainRate + juce::String(genIdx);
  p.addParameter(grainRate =
                     new juce::AudioParameterFloat(rateId, rateId, ParamRanges::GRAIN_RATE, ParamDefaults::GRAIN_RATE_DEFAULT));
  juce::String durationId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainDuration + juce::String(genIdx);
  p.addParameter(grainDuration = new juce::AudioParameterFloat(durationId, durationId, ParamRanges::GRAIN_DURATION,
                                                               ParamDefaults::GRAIN_DURATION_DEFAULT));
  juce::String syncId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainSync + juce::String(genIdx);
  p.addParameter(grainSync = new juce::AudioParameterBool(syncId, syncId, false));
  juce::String attackId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genAttack + juce::String(genIdx);
  p.addParameter(attack =
                     new juce::AudioParameterFloat(attackId, attackId, ParamRanges::ATTACK, ParamDefaults::ATTACK_DEFAULT_SEC));
  juce::String decayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genDecay + juce::String(genIdx);
  p.addParameter(decay = new juce::AudioParameterFloat(decayId, decayId, ParamRanges::DECAY, ParamDefaults::DECAY_DEFAULT_SEC));
  juce::String sustainId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genSustain + juce::String(genIdx);
  p.addParameter(sustain = new juce::AudioParameterFloat(sustainId, sustainId, juce::NormalisableRange<float>(0.0f, 1.0f),
                                                         ParamDefaults::SUSTAIN_DEFAULT));
  juce::String releaseId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genRelease + juce::String(genIdx);
  p.addParameter(release =
                     new juce::AudioParameterFloat(releaseId, releaseId, ParamRanges::RELEASE, ParamDefaults::RELEASE_DEFAULT_SEC));
  juce::String cutoffId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genFilterCutoff + juce::String(genIdx);
  p.addParameter(filterCutoff = 
                     new juce::AudioParameterFloat(cutoffId, cutoffId, ParamRanges::CUTOFF, ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ));
  juce::String resonanceId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genFilterResonance + juce::String(genIdx);
  p.addParameter(filterResonance = new juce::AudioParameterFloat(resonanceId, resonanceId, ParamRanges::RESONANCE,
                                                           ParamDefaults::FILTER_RESONANCE_DEFAULT));
  juce::String strengthId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genFilterStrength + juce::String(genIdx);
  p.addParameter(filterStrength = new juce::AudioParameterFloat(strengthId, strengthId, ParamRanges::STRENGTH,
                                                           ParamDefaults::FILTER_STRENGTH_DEFAULT));
  juce::String filterTypeId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genFilterType + juce::String(genIdx);
  p.addParameter(filterType = 
                     new juce::AudioParameterChoice(filterTypeId, filterTypeId, FILTER_TYPE_NAMES, 0));
  updateGrainEnvelopeLUT(); 
}

void ParamGenerator::addListener(juce::AudioProcessorParameter::Listener* listener) {
  gain->addListener(listener);
  candidate->addListener(listener);
  pitchAdjust->addListener(listener);
  pitchSpray->addListener(listener);
  positionAdjust->addListener(listener);
  positionSpray->addListener(listener);
  grainShape->addListener(listener);
  grainTilt->addListener(listener);
  grainRate->addListener(listener);
  grainDuration->addListener(listener);
  grainSync->addListener(listener);
  attack->addListener(listener);
  decay->addListener(listener);
  sustain->addListener(listener);
  release->addListener(listener);
  filterCutoff->addListener(listener);
  filterResonance->addListener(listener);
  filterStrength->addListener(listener);
  filterType->addListener(listener);
}

void ParamGenerator::removeListener(juce::AudioProcessorParameter::Listener* listener) {
  gain->removeListener(listener);
  candidate->removeListener(listener);
  pitchAdjust->removeListener(listener);
  pitchSpray->removeListener(listener);
  positionAdjust->removeListener(listener);
  positionSpray->removeListener(listener);
  grainShape->removeListener(listener);
  grainTilt->removeListener(listener);
  grainRate->removeListener(listener);
  grainDuration->removeListener(listener);
  grainSync->removeListener(listener);
  attack->removeListener(listener);
  decay->removeListener(listener);
  sustain->removeListener(listener);
  release->removeListener(listener);
  filterCutoff->removeListener(listener);
  filterResonance->removeListener(listener);
  filterStrength->removeListener(listener);
  filterType->removeListener(listener);
}

void ParamGenerator::updateGrainEnvelopeLUT() {
  grainEnvLUT.clear();
  /* LUT divided into 3 parts

               1.0
              -----
     rampUp  /     \  rampDown
            /       \
  */
  float scaledShape = (grainShape->get() * ENV_LUT_SIZE) / 2.0f;
  float scaledTilt = grainTilt->get() * ENV_LUT_SIZE;
  int rampUpEndSample = juce::jmax(0.0f, scaledTilt - scaledShape);
  int rampDownStartSample = juce::jmin((float)ENV_LUT_SIZE, scaledTilt + scaledShape);
  for (int i = 0; i < ENV_LUT_SIZE; i++) {
    if (i < rampUpEndSample) {
      grainEnvLUT.push_back((float)i / rampUpEndSample);
    } else if (i > rampDownStartSample) {
      grainEnvLUT.push_back(1.0f - (float)(i - rampDownStartSample) / (ENV_LUT_SIZE - rampDownStartSample));
    } else {
      grainEnvLUT.push_back(1.0f);
    }
  }
  juce::FloatVectorOperations::clip(grainEnvLUT.data(), grainEnvLUT.data(), 0.0f, 1.0f, grainEnvLUT.size());
}

void ParamNote::addParams(juce::AudioProcessor& p) {
  for (auto& generator : generators) {
    generator->addParams(p);
  }
  juce::String soloId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genSolo;
  p.addParameter(soloIdx = new juce::AudioParameterInt(soloId, soloId, SOLO_NONE, NUM_GENERATORS - 1, SOLO_NONE));
}

void ParamNote::addListener(int genIdx, juce::AudioProcessorParameter::Listener* listener) {
  soloIdx->addListener(listener);
  for (auto&& gen : generators) {
    gen->enable->addListener(listener);
  }
  generators[genIdx]->addListener(listener);
}

void ParamNote::removeListener(int genIdx, juce::AudioProcessorParameter::Listener* listener) {
  soloIdx->removeListener(listener);
  for (auto&& gen : generators) {
    gen->enable->removeListener(listener);
  }
  generators[genIdx]->removeListener(listener);
}

ParamCandidate* ParamNote::getCandidate(int genIdx) {
  if (candidates.empty()) return nullptr;
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

void ParamsNote::resetParams() {
  for (auto& note : notes) {
    for (auto& generator : note->generators) {
      ParamHelper::setParam(generator->enable, generator->genIdx == 0);
      ParamHelper::setParam(generator->gain, ParamDefaults::GAIN_DEFAULT);
      ParamHelper::setParam(generator->candidate, 0);
      ParamHelper::setParam(generator->pitchAdjust, 0.0f);
      ParamHelper::setParam(generator->pitchSpray, 0.0f);
      ParamHelper::setParam(generator->positionAdjust, 0.0f);
      ParamHelper::setParam(generator->positionSpray, ParamDefaults::POSITION_SPRAY_DEFAULT);
      ParamHelper::setParam(generator->grainShape, 0.5f);
      ParamHelper::setParam(generator->grainTilt, 0.5f);
      ParamHelper::setParam(generator->grainRate, ParamDefaults::GRAIN_RATE_DEFAULT);
      ParamHelper::setParam(generator->grainDuration, ParamDefaults::GRAIN_DURATION_DEFAULT);
      ParamHelper::setParam(generator->grainSync, false);
      ParamHelper::setParam(generator->attack, ParamDefaults::ATTACK_DEFAULT_SEC);
      ParamHelper::setParam(generator->decay, ParamDefaults::DECAY_DEFAULT_SEC);
      ParamHelper::setParam(generator->sustain, ParamDefaults::SUSTAIN_DEFAULT);
      ParamHelper::setParam(generator->release, ParamDefaults::RELEASE_DEFAULT_SEC);
      ParamHelper::setParam(generator->filterCutoff, ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
      ParamHelper::setParam(generator->filterResonance, ParamDefaults::FILTER_RESONANCE_DEFAULT);
      ParamHelper::setParam(generator->filterStrength, ParamDefaults::FILTER_STRENGTH_DEFAULT);
      ParamHelper::setParam(generator->filterType, 0);
    }
    note->candidates.clear();
    ParamHelper::setParam(note->soloIdx, SOLO_NONE);
  }
}