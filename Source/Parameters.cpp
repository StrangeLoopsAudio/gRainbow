/*
  ==============================================================================

    Parameters.cpp
    Created: 10 Aug 2021 6:47:57pm
    Author:  brady

  ==============================================================================
*/

#include "Parameters.h"

void GlobalParams::addParams(juce::AudioProcessor& p) {
  p.addParameter(attack = new juce::AudioParameterFloat(
                     ParamIDs::globalAttack, "Master Attack",
                     juce::NormalisableRange<float>(0.01f, 1.0f), 0.2f));
  p.addParameter(decay = new juce::AudioParameterFloat(
                     ParamIDs::globalDecay, "Master Decay",
                     juce::NormalisableRange<float>(0.01f, 1.0f), 0.2f));
  p.addParameter(sustain = new juce::AudioParameterFloat(
                     ParamIDs::globalSustain, "Master Sustain",
                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));
  p.addParameter(release = new juce::AudioParameterFloat(
                     ParamIDs::globalRelease, "Master Release",
                     juce::NormalisableRange<float>(0.01f, 1.0f), 0.5f));
}

void GeneratorParams::addParams(juce::AudioProcessor& p) {
  p.addParameter(
      enable = new juce::AudioParameterBool(
          juce::String(genIdx) + ParamIDs::genEnable + juce::String(noteIdx),
          "Gen Enable", false));
  p.addParameter(
      solo = new juce::AudioParameterBool(
          juce::String(genIdx) + ParamIDs::genSolo + juce::String(noteIdx),
          "Gen Solo", false));
  p.addParameter(
      candidate = new juce::AudioParameterInt(
          juce::String(genIdx) + ParamIDs::genCandidate + juce::String(noteIdx),
          "Gen Candidate", 0, MAX_CANDIDATES - 1, 0));
  p.addParameter(
      pitchAdjust = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genPitchAdjust + juce::String(noteIdx),
          "Gen Pitch Adjust", juce::NormalisableRange<float>(-0.5f, 0.5f), 0.0f));
  p.addParameter(
      positionAdjust = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genPositionAdjust + juce::String(noteIdx),
          "Gen Position Adjust", juce::NormalisableRange<float>(-0.5f, 0.5f), 0.0f));

  p.addParameter(grainShape = new juce::AudioParameterFloat(
                     juce::String(genIdx) + ParamIDs::genGrainShape +
                         juce::String(noteIdx),
                     "Gen Grain Shape",
                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
  p.addParameter(
      grainTilt = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genGrainTilt + juce::String(noteIdx),
          "Gen Grain Tilt", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
  p.addParameter(
      grainRate = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genGrainRate + juce::String(noteIdx),
          "Gen Grain Rate", juce::NormalisableRange<float>(0.25f, 1.0f), 0.5f));
  p.addParameter(grainDuration = new juce::AudioParameterFloat(
                     juce::String(genIdx) + ParamIDs::genGrainDuration +
                         juce::String(noteIdx),
                     "Gen Grain Duration",
                     juce::NormalisableRange<float>(60.0f, 300.0f), 100.0f));
  p.addParameter(
      grainGain = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genGrainGain + juce::String(noteIdx),
          "Gen Grain Gain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));
  p.addParameter(
      attack = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genAttack + juce::String(noteIdx),
          "Gen Attack", juce::NormalisableRange<float>(0.01f, 1.0f), 0.2f));
  p.addParameter(
      decay = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genDecay + juce::String(noteIdx),
          "Gen Decay", juce::NormalisableRange<float>(0.01f, 1.0f), 0.2f));
  p.addParameter(
      sustain = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genSustain + juce::String(noteIdx),
          "Gen Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));
  p.addParameter(
      release = new juce::AudioParameterFloat(
          juce::String(genIdx) + ParamIDs::genRelease + juce::String(noteIdx),
          "Gen Release", juce::NormalisableRange<float>(0.01f, 1.0f), 0.5f));
}

void NoteParam::addParams(juce::AudioProcessor& p) {
  for (auto& generator : generators) {
    generator->addParams(p);
  }
}

void NoteParams::addParams(juce::AudioProcessor& p) {
  for (auto& note : notes) {
    note->addParams(p);
  }
}