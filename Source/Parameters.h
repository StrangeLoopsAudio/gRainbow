/*
  ==============================================================================

    Parameters.h
    Created: 10 Aug 2021 6:27:45pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include "Utils/Utils.h"
#include "Utils/Colour.h"
#include "Utils/PitchClass.h"
#include "Modulators.h"

// Dynamically casts to AudioParameterFloat*
#define P_FLOAT(X) dynamic_cast<juce::AudioParameterFloat*>(X)
// Dynamically casts to AudioParameterFloat*
#define P_INT(X) dynamic_cast<juce::AudioParameterInt*>(X)
// Dynamically casts to AudioParameterChoice*
#define P_CHOICE(X) dynamic_cast<juce::AudioParameterChoice*>(X)
// Dynamically casts to AudioParameterBool*
#define P_BOOL(X) dynamic_cast<juce::AudioParameterBool*>(X)

namespace ParamIDs {
// Global params
static juce::String ampEnvAttack{"env_attack"};
static juce::String ampEnvDecay{"amp_env_decay"};
static juce::String ampEnvSustain{"amp_env_sustain"};
static juce::String ampEnvRelease{"amp_env_release"};
// Modulators
static juce::String lfoShape{"lfo_shape_"};
static juce::String lfoRate{"lfo_rate_"};
static juce::String lfoPhase{"lfo_phase_"};
static juce::String lfoSync{"lfo_sync_"};
static juce::String lfoBipolar{"lfo_bipolar_"};
static juce::String lfoRetrigger{"lfo_retrigger_"};
static juce::String modEnvAttack{"mod_env_attack_"};
static juce::String modEnvDecay{"mod_env_decay_"};
static juce::String modEnvSustain{"mod_env_sustain_"};
static juce::String modEnvRelease{"mod_env_release_"};
static juce::String macro{"macro_"};
// Global common
static juce::String globalGain{"global_gain"};
static juce::String globalAttack{"global_attack"};
static juce::String globalDecay{"global_decay"};
static juce::String globalSustain{"global_sustain"};
static juce::String globalRelease{"global_release"};
static juce::String globalGrainShape{"global_grain_shape"};
static juce::String globalGrainTilt{"global_grain_tilt"};
static juce::String globalGrainRate{"global_grain_rate"};
static juce::String globalGrainDuration{"global_grain_duration"};
static juce::String globalGrainSync{"global_grain_sync"};
static juce::String globalPitchAdjust{"global_pitch_adjust"};
static juce::String globalPitchSpray{"global_pitch_spray"};
static juce::String globalPositionAdjust{"global_position_adjust"};
static juce::String globalPositionSpray{"global_position_spray"};
static juce::String globalPanAdjust{"global_pan_adjust"};
static juce::String globalPanSpray{"global_pan_spray"};
static juce::String globalReverse{"global_reverse"};
// Note params
static juce::String genSolo{"_solo_gen"};
static juce::String noteGain{"_note_gain"};
static juce::String noteAttack{"_note_attack"};
static juce::String noteDecay{"_note_decay"};
static juce::String noteSustain{"_note_sustain"};
static juce::String noteRelease{"_note_release"};
static juce::String noteGrainShape{"_note_grain_shape"};
static juce::String noteGrainTilt{"_note_grain_tilt"};
static juce::String noteGrainRate{"_note_grain_rate"};
static juce::String noteGrainDuration{"_note_grain_duration"};
static juce::String noteGrainSync{"_note_grain_sync"};
static juce::String notePitchAdjust{"_note_pitch_adjust"};
static juce::String notePitchSpray{"_note_pitch_spray"};
static juce::String notePositionAdjust{"_note_position_adjust"};
static juce::String notePositionSpray{"_note_position_spray"};
static juce::String notePanAdjust{"_note_pan_adjust"};
static juce::String notePanSpray{"_note_pan_spray"};
static juce::String noteReverse{"_note_reverse"};
// Generator params
static juce::String genEnable{"_enable_gen_"};
static juce::String genCandidate{"_candidate_gen_"};
static juce::String genGain{"_gain_gen_"};
static juce::String genAttack{"_attack_gen_"};
static juce::String genDecay{"_decay_gen_"};
static juce::String genSustain{"_sustain_gen_"};
static juce::String genRelease{"_release_gen_"};
static juce::String genGrainShape{"_grain_shape_gen_"};
static juce::String genGrainTilt{"_grain_tilt_gen_"};
static juce::String genGrainRate{"_grain_rate_gen_"};
static juce::String genGrainDuration{"_grain_duration_gen_"};
static juce::String genGrainSync{"_grain_sync_gen_"};
static juce::String genPitchAdjust{"_pitch_adjust_gen_"};
static juce::String genPitchSpray{"_pitch_spray_gen_"};
static juce::String genPositionAdjust{"_position_adjust_gen_"};
static juce::String genPositionSpray{"_position_spray_gen_"};
static juce::String genPanAdjust{"_pan_adjust_gen_"};
static juce::String genPanSpray{"_pan_spray_gen_"};
static juce::String genReverse{"_reverse_gen_"};
} // namespace ParamIDs

namespace ParamRanges {
static juce::NormalisableRange<float> LFO_RATE(0.01f, 10.0f);
static juce::NormalisableRange<float> LFO_PHASE(0.0f, 2.0f * M_PI);
static juce::NormalisableRange<float> MACRO(0.0f, 1.0f);
static juce::NormalisableRange<float> GAIN(0.0f, 1.0f);
static juce::NormalisableRange<float> ATTACK(0.01f, 2.0f);
static juce::NormalisableRange<float> DECAY(0.01f, 2.0f);
static juce::NormalisableRange<float> SUSTAIN(0.0f, 1.0f);
static juce::NormalisableRange<float> RELEASE(0.01f, 2.0f);
static juce::NormalisableRange<float> FILT_CUTOFF(100.0f, 2000.0f, 0.0f, 0.25f);
static juce::NormalisableRange<float> FILT_RESONANCE(0.5f, 1.0f);
static juce::NormalisableRange<float> GRAIN_SHAPE(0.0f, 1.0f);
static juce::NormalisableRange<float> GRAIN_TILT(-1.0f, 1.0f);
static juce::NormalisableRange<float> GRAIN_RATE(0.5f, 50.0f);
static juce::NormalisableRange<float> GRAIN_DURATION(0.06f, 0.3f);
static juce::NormalisableRange<float> PITCH_ADJUST(-0.25f, 0.25f);
static juce::NormalisableRange<float> PITCH_SPRAY(0.0f, 0.1f);
static juce::NormalisableRange<float> POSITION_ADJUST(-0.5f, 0.5f);
static juce::NormalisableRange<float> POSITION_SPRAY(0.0f, 0.3f);
static juce::NormalisableRange<float> PAN_ADJUST(-1.0f, 1.0f);
static juce::NormalisableRange<float> PAN_SPRAY(0.0f, 1.0f);

static int SYNC_DIV_MAX = 7;  // pow of 2 division, so 1/16
}  // namespace ParamRanges

namespace ParamDefaults {
static int   LFO_SHAPE_DEFAULT = 0;
static float LFO_RATE_DEFAULT = 1.0f;
static float LFO_PHASE_DEFAULT = 0.0f;
static int   LFO_SYNC_DEFAULT = 0;
static int   LFO_BIPOLAR_DEFAULT = 1;
static int   LFO_RETRIGGER_DEFAULT = 1;
static float MACRO_DEFAULT = 0.5f;
static float GAIN_DEFAULT = 0.8f;
static float ATTACK_DEFAULT_SEC = 0.2f;
static float DECAY_DEFAULT_SEC = 0.2f;
static float SUSTAIN_DEFAULT = 0.8f;
static float RELEASE_DEFAULT_SEC = 0.2f;
static float GRAIN_SHAPE_DEFAULT = 0.5f;
static float GRAIN_TILT_DEFAULT = 0.0f;
static float GRAIN_RATE_DEFAULT = 10.0f;
static int   GRAIN_SYNC_DEFAULT = 0;
static float GRAIN_DURATION_DEFAULT = 0.2f;
static float PITCH_ADJUST_DEFAULT = 0.0f;
static float PITCH_SPRAY_DEFAULT = 0.01f;
static float POSITION_ADJUST_DEFAULT = 0.0f;
static float POSITION_SPRAY_DEFAULT = 0.05f;
static float PAN_ADJUST_DEFAULT = 0.0f;
static float PAN_SPRAY_DEFAULT = 0.05f;
static int   REVERSE_DEFAULT = 0;
}  // namespace ParamDefaults

enum ParamType { GLOBAL, NOTE, GENERATOR };
static juce::Array<juce::String> PARAM_TYPE_NAMES{"global", "note", "generator"};
static juce::Array<juce::String> PITCH_CLASS_NAMES{"C", "Cs", "D", "Ds", "E", "F", "Fs", "G", "Gs", "A", "As", "B"};
static juce::Array<juce::String> LFO_SHAPE_NAMES{"sine", "tri", "square", "saw"};

static constexpr int MAX_CANDIDATES = 6;
static constexpr int NUM_GENERATORS = 4;
static constexpr int SOLO_NONE = -1;

namespace ParamHelper {
[[maybe_unused]] static juce::String getParamID(juce::AudioProcessorParameter* param) {
  if (auto paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param)) return paramWithID->paramID;

  return param->getName(50);
}
  // Utility function to avoid ugly dereferencing code before sending norm value
  // to host
  static void setParam(juce::AudioParameterFloat* param, float newValue) { *param = newValue; }
  static void setParam(juce::AudioParameterInt* param, int newValue) { *param = newValue; }
  static void setParam(juce::AudioParameterBool* param, bool newValue) { *param = newValue; }
  static void setParam(juce::AudioParameterChoice* param, int newValue) { *param = newValue; }
}

// Common parameters types used by each generator, note and globally
class ParamCommon {
 public:
  ParamCommon(ParamType _type) : type(_type) { }
  virtual ~ParamCommon() { }

  enum Type {
    GAIN = 0,
    GRAIN_SHAPE,
    GRAIN_TILT,
    GRAIN_RATE,
    GRAIN_DURATION,
    GRAIN_SYNC,
    PITCH_ADJUST,
    PITCH_SPRAY,
    POS_ADJUST,
    POS_SPRAY,
    PAN_ADJUST,
    PAN_SPRAY,
    REVERSE,
    NUM_COMMON
  };

  void addListener(juce::AudioProcessorParameter::Listener* listener) {
    common[GAIN]->addListener(listener);
    common[GRAIN_SHAPE]->addListener(listener);
    common[GRAIN_TILT]->addListener(listener);
    common[GRAIN_RATE]->addListener(listener);
    common[GRAIN_DURATION]->addListener(listener);
    common[GRAIN_SYNC]->addListener(listener);
    common[PITCH_ADJUST]->addListener(listener);
    common[PITCH_SPRAY]->addListener(listener);
    common[POS_ADJUST]->addListener(listener);
    common[POS_SPRAY]->addListener(listener);
    common[PAN_ADJUST]->addListener(listener);
    common[PAN_SPRAY]->addListener(listener);
    common[REVERSE]->addListener(listener);
  }
  void removeListener(juce::AudioProcessorParameter::Listener* listener) {
    common[GAIN]->removeListener(listener);
    common[GRAIN_SHAPE]->removeListener(listener);
    common[GRAIN_TILT]->removeListener(listener);
    common[GRAIN_RATE]->removeListener(listener);
    common[GRAIN_DURATION]->removeListener(listener);
    common[GRAIN_SYNC]->removeListener(listener);
    common[PITCH_ADJUST]->removeListener(listener);
    common[PITCH_SPRAY]->removeListener(listener);
    common[POS_ADJUST]->removeListener(listener);
    common[POS_SPRAY]->removeListener(listener);
    common[PAN_ADJUST]->removeListener(listener);
    common[PAN_SPRAY]->removeListener(listener);
    common[REVERSE]->removeListener(listener);
  }

  void resetParams() {
    ParamHelper::setParam(P_FLOAT(common[GAIN]), ParamDefaults::GAIN_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[PITCH_ADJUST]), ParamDefaults::PITCH_ADJUST_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[PITCH_SPRAY]), ParamDefaults::PITCH_SPRAY_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[POS_ADJUST]), ParamDefaults::POSITION_ADJUST_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[POS_SPRAY]), ParamDefaults::POSITION_SPRAY_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[PAN_ADJUST]), ParamDefaults::PAN_ADJUST_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[PAN_SPRAY]), ParamDefaults::PAN_SPRAY_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[GRAIN_SHAPE]), ParamDefaults::GRAIN_SHAPE_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[GRAIN_TILT]), ParamDefaults::GRAIN_TILT_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[GRAIN_RATE]), ParamDefaults::GRAIN_RATE_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[GRAIN_DURATION]), ParamDefaults::GRAIN_DURATION_DEFAULT);
    ParamHelper::setParam(P_BOOL(common[GRAIN_SYNC]), ParamDefaults::GRAIN_SYNC_DEFAULT);
    ParamHelper::setParam(P_BOOL(common[REVERSE]), ParamDefaults::REVERSE_DEFAULT);
    for (auto& used : isUsed) { used = false; }
  }

  juce::RangedAudioParameter* common[Type::NUM_COMMON];
  bool isUsed[Type::NUM_COMMON]; // Flag for each parameter set to true when changed from its default

  // Type of derived class
  ParamType type;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamCommon)
};

static float COMMON_DEFAULTS[ParamCommon::Type::NUM_COMMON] = {ParamDefaults::GAIN_DEFAULT,
  ParamDefaults::GRAIN_SHAPE_DEFAULT,
  ParamDefaults::GRAIN_TILT_DEFAULT,
  ParamDefaults::GRAIN_RATE_DEFAULT,
  ParamDefaults::GRAIN_DURATION_DEFAULT,
  (float)ParamDefaults::GRAIN_SYNC_DEFAULT,
  ParamDefaults::PITCH_ADJUST_DEFAULT,
  ParamDefaults::PITCH_SPRAY_DEFAULT,
  ParamDefaults::POSITION_ADJUST_DEFAULT,
  ParamDefaults::POSITION_SPRAY_DEFAULT,
  ParamDefaults::PAN_ADJUST_DEFAULT,
  ParamDefaults::PAN_SPRAY_DEFAULT,
  (float)ParamDefaults::REVERSE_DEFAULT
};

static juce::NormalisableRange<float> COMMON_RANGES[ParamCommon::Type::NUM_COMMON] = {ParamRanges::GAIN,
  ParamRanges::GRAIN_SHAPE,
  ParamRanges::GRAIN_TILT,
  ParamRanges::GRAIN_RATE,
  ParamRanges::GRAIN_DURATION,
  juce::NormalisableRange<float>(0.0f, 1.0f), // grain sync
  ParamRanges::PITCH_ADJUST,
  ParamRanges::PITCH_SPRAY,
  ParamRanges::POSITION_ADJUST,
  ParamRanges::POSITION_SPRAY,
  ParamRanges::PAN_ADJUST,
  ParamRanges::PAN_SPRAY,
  juce::NormalisableRange<float>(0.0f, 1.0f) // reverse
};

namespace ParamHelper {
[[maybe_unused]] static void setCommonParam(ParamCommon* common, ParamCommon::Type type, float newValue) {
  ParamHelper::setParam(P_FLOAT(common->common[type]), newValue);
  common->isUsed[type] = true;
}
[[maybe_unused]] static void setCommonParam(ParamCommon* common, ParamCommon::Type type, int newValue) {
  ParamHelper::setParam(P_CHOICE(common->common[type]), newValue);
  common->isUsed[type] = true;
}
[[maybe_unused]] static void setCommonParam(ParamCommon* common, ParamCommon::Type type, bool newValue) {
  ParamHelper::setParam(P_BOOL(common->common[type]), newValue);
  common->isUsed[type] = true;
}
}

struct ParamCandidate {
  double posRatio;
  int    octave;
  double pbRate;
  double duration;
  double salience;

  ParamCandidate(float _posRatio, int _octave, float _pbRate, float _duration, float _salience)
      : posRatio(_posRatio), octave(_octave), pbRate(_pbRate), duration(_duration), salience(_salience) {}

  // setXml equivalent since we always need a valid candidate param value
  ParamCandidate(juce::XmlElement* xml) {
    jassert(xml->hasTagName("ParamCandidate"));
    posRatio = xml->getDoubleAttribute("posRatio");
    octave = xml->getIntAttribute("octave");
    pbRate = xml->getDoubleAttribute("pbRate");
    duration = xml->getDoubleAttribute("duration");
    salience = xml->getDoubleAttribute("salience");
  }

  juce::XmlElement* getXml() {
    juce::XmlElement* xml = new juce::XmlElement("ParamCandidate");
    xml->setAttribute("posRatio", posRatio);
    xml->setAttribute("octave", octave);
    xml->setAttribute("pbRate", pbRate);
    xml->setAttribute("duration", duration);
    xml->setAttribute("salience", salience);
    return xml;
  }
};

struct ParamGenerator : ParamCommon {
  ParamGenerator(int _noteIdx, int _genIdx) : ParamCommon(ParamType::GENERATOR), noteIdx(_noteIdx), genIdx(_genIdx) {}
  ~ParamGenerator() {}

  void addParams(juce::AudioProcessor& p);

  void addListener(juce::AudioProcessorParameter::Listener* listener) {
    ParamCommon::addListener(listener);
    candidate->addListener(listener);
  }
  void removeListener(juce::AudioProcessorParameter::Listener* listener) {
    ParamCommon::removeListener(listener);
    candidate->removeListener(listener);
  }

  void resetParams(bool) {
    ParamCommon::resetParams();
    ParamHelper::setParam(enable, true);
    ParamHelper::setParam(candidate, genIdx);
  }

  int noteIdx;
  int genIdx;

  juce::AudioParameterBool* enable = nullptr;
  juce::AudioParameterInt* candidate = nullptr;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamGenerator)
};

struct ParamNote : ParamCommon {
  ParamNote(int noteIdx_) : ParamCommon(ParamType::NOTE), noteIdx(noteIdx_) {
    for (int i = 0; i < NUM_GENERATORS; ++i) {
      generators.emplace_back(new ParamGenerator(noteIdx, i));
    }
  }

  // Returns the number of enabled generators
  [[nodiscard]] int getNumEnabledGens() const {
    int numEnabled = 0;
    for (auto& gen : generators) {
      if (gen->enable->get()) numEnabled++;
    }
    return numEnabled;
  }

  // Gets list of enabled generators, then returns the one at idx, or nullptr if idx > num enabled gens
  [[nodiscard]] ParamGenerator* getEnabledGenByIdx(int idx) const {
    int numEnabled = 0;
    for (size_t i = 0; i < NUM_GENERATORS; ++i) {
      if (generators[i]->enable->get()) {
        if (numEnabled == idx) return generators[i].get();
        numEnabled++;
      }
    }
    return nullptr;
  }

  void enableNextAvailableGen() {
    for (size_t i = 0; i < NUM_GENERATORS; ++i) {
      if (!generators[i]->enable->get()) {
        ParamHelper::setParam(generators[i]->enable, true);
        break;
      }
    }
  }

  void addParams(juce::AudioProcessor& p);

  void addListener(juce::AudioProcessorParameter::Listener* listener) {
    ParamCommon::addListener(listener);
    soloIdx->addListener(listener);
  }
  void removeListener(juce::AudioProcessorParameter::Listener* listener) {
    ParamCommon::removeListener(listener);
    soloIdx->removeListener(listener);
  }

  void resetParams(bool fullClear) {
    ParamCommon::resetParams();
    for (auto& generator : generators) {
      generator->resetParams(fullClear);
    }
    if (fullClear) {
      candidates.clear();
    }
    ParamHelper::setParam(soloIdx, SOLO_NONE);
  }

  bool shouldPlayGenerator(int genIdx);
  ParamCandidate* getCandidate(int genIdx);
  void setStartingCandidatePosition();

  int noteIdx;

  std::vector<std::unique_ptr<ParamGenerator>> generators;
  std::vector<ParamCandidate> candidates;

  juce::AudioParameterInt* soloIdx = nullptr;

  juce::XmlElement* getXml() {
    juce::XmlElement* xml = new juce::XmlElement("ParamNote");
    for (ParamCandidate& candidate : candidates) {
      xml->addChildElement(candidate.getXml());
    }
    return xml;
  }

  void setXml(juce::XmlElement* xml) {
    jassert(xml->hasTagName("ParamNote"));
    candidates.clear();
    for (auto* children : xml->getChildIterator()) {
      if (children->hasTagName("ParamCandidate")) {
        candidates.push_back(ParamCandidate(children));
      }
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamNote)
};

struct ParamsNote {
  ParamsNote() {
    for (int i = 0; i < (int)Utils::PitchClass::COUNT; ++i) {
      notes[i] = std::unique_ptr<ParamNote>(new ParamNote(i));
    }
  }

  void addParams(juce::AudioProcessor& p);

  void resetParams(bool fullClear = true) {
    for (auto& note : notes) {
      note->resetParams(fullClear);
    }
  }

  // always send creation and have callback scope decide if valid or not
  void grainCreated(Utils::PitchClass pitchClass, int genIdx, float durationSec, float envGain) {
    if (onGrainCreated != nullptr) {
      onGrainCreated(pitchClass, genIdx, durationSec, envGain);
    }
  }
  std::function<void(Utils::PitchClass pitchClass, int genIdx, float durationSec, float envGain)> onGrainCreated = nullptr;

  std::array<std::unique_ptr<ParamNote>, Utils::PitchClass::COUNT> notes;

  juce::XmlElement* getXml() {
    juce::XmlElement* xml = new juce::XmlElement("NotesParams");
    for (auto&& note : notes) {
      xml->addChildElement(note->getXml());
    }
    return xml;
  }

  void setXml(juce::XmlElement* xml) {
    jassert(xml->hasTagName("NotesParams"));
    // Currently all child elements are NotesParam elements
    for (size_t i = 0; i < notes.size(); i++) {
      notes[i]->setXml(xml->getChildElement(i));
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamsNote)
};

struct ParamGlobal : ParamCommon {
  ParamGlobal() : ParamCommon(ParamType::GLOBAL) {
    // Default to using global parameters
    for (auto& used : isUsed) {
      used = true;
    }
    modLFOs[0].colour = juce::Colour(0xffffe8d6);
    modLFOs[1].colour = juce::Colour(0xffddbea9);
    modLFOs[2].colour = juce::Colour(0xffcb997e);
    modEnvs[0].colour = juce::Colour(0xff88a9af);
    modEnvs[1].colour = juce::Colour(0xff7a9e9f);
    macros[0].colour = juce::Colour(0xffB9DAC9);
    macros[1].colour = juce::Colour(0xffA2C3B1);
    macros[2].colour = juce::Colour(0xff85BCA2);
    macros[3].colour = juce::Colour(0xff7b9d8f);
  }
  ~ParamGlobal() {}

  void addParams(juce::AudioProcessor& p);
  
  void resetParams() {
    ParamCommon::resetParams();
    ParamHelper::setParam(ampEnvAttack, ParamDefaults::ATTACK_DEFAULT_SEC);
    ParamHelper::setParam(ampEnvDecay, ParamDefaults::DECAY_DEFAULT_SEC);
    ParamHelper::setParam(ampEnvSustain, ParamDefaults::SUSTAIN_DEFAULT);
    ParamHelper::setParam(ampEnvRelease, ParamDefaults::RELEASE_DEFAULT_SEC);
    for (auto& lfo : modLFOs) {
      ParamHelper::setParam(lfo.shape, ParamDefaults::LFO_SHAPE_DEFAULT);
      ParamHelper::setParam(lfo.rate, ParamDefaults::LFO_RATE_DEFAULT);
      ParamHelper::setParam(lfo.phase, ParamDefaults::LFO_PHASE_DEFAULT);
      ParamHelper::setParam(lfo.sync, ParamDefaults::LFO_SYNC_DEFAULT);
      ParamHelper::setParam(lfo.bipolar, ParamDefaults::LFO_BIPOLAR_DEFAULT);
      ParamHelper::setParam(lfo.retrigger, ParamDefaults::LFO_RETRIGGER_DEFAULT);
    }
    for (auto& env : modEnvs) {
      ParamHelper::setParam(env.attack, ParamDefaults::ATTACK_DEFAULT_SEC);
      ParamHelper::setParam(env.decay, ParamDefaults::DECAY_DEFAULT_SEC);
      ParamHelper::setParam(env.sustain, ParamDefaults::SUSTAIN_DEFAULT);
      ParamHelper::setParam(env.release, ParamDefaults::RELEASE_DEFAULT_SEC);
    }
    for (auto& macro : macros) {
      ParamHelper::setParam(macro.macro, ParamDefaults::MACRO_DEFAULT);
    }
  }
  
  // Global parameters
  juce::AudioParameterFloat* ampEnvAttack;
  juce::AudioParameterFloat* ampEnvDecay;
  juce::AudioParameterFloat* ampEnvSustain;
  juce::AudioParameterFloat* ampEnvRelease;

  // Global modulation sources
  std::array<LFOModSource, 3> modLFOs;
  std::array<EnvModSource, 2> modEnvs;
  std::array<MacroModSource, 4> macros;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamGlobal)
};

/**
 * A representation of the last UI settings to restore it when loading the
 * editor. The Synth owns this and used to allow state to be saved properly as
 * the UI is open and closed.
 */
struct ParamUI {
  ParamUI() = default;

  enum SpecType { INVALID = -1, SPECTROGRAM = 0, HPCP, DETECTED, WAVEFORM, COUNT };

  // Get it from the plugin state
  // will only set xml-able items (floats/int/strings)
  void setXml(juce::XmlElement* xml) {
    if (xml != nullptr) {
      loadedFileName = xml->getStringAttribute("fileName");
      fileName = loadedFileName;
      pitchClass = xml->getIntAttribute("pitchClass");
      specType = (SpecType)xml->getIntAttribute("specType");
      centerComponent = (CenterComponent)xml->getIntAttribute("centerComponent");
      trimRange.setStart(xml->getDoubleAttribute("trimRangeStart"));
      trimRange.setEnd(xml->getDoubleAttribute("trimRangeEnd"));
      specComplete = xml->getBoolAttribute("specComplete");
      if (auto images = xml->getChildByName("Images")) {
        for (int i = 0; i < ParamUI::SpecType::COUNT; ++i) {
          juce::String attrName = "image" + juce::String(i);
          if (images->hasAttribute(attrName)) {
            juce::MemoryBlock imageData;
            if (imageData.fromBase64Encoding(images->getStringAttribute(attrName, ""))) {
              juce::MemoryInputStream in(imageData.getData(), imageData.getSize(), true);
              specImages[i] = juce::PNGImageFormat::loadFrom(in);
            }
          }
        }
      }
    }
  }

  // Build the XML representation to save in plugin state.
  juce::XmlElement* getXml() {
    juce::XmlElement* xml = new juce::XmlElement("ParamUI");
    xml->setAttribute("fileName", loadedFileName);
    xml->setAttribute("pitchClass", pitchClass);
    xml->setAttribute("specType", static_cast<int>(specType));
    xml->setAttribute("centerComponent", static_cast<int>(centerComponent));
    xml->setAttribute("trimRangeStart", trimRange.getStart());
    xml->setAttribute("trimRangeEnd", trimRange.getEnd());
    xml->setAttribute("specComplete", specComplete);
    juce::XmlElement* images = new juce::XmlElement("Images");
    for (size_t i = 0; i < ParamUI::SpecType::COUNT; ++i) {
      juce::MemoryOutputStream out;
      saveSpecImage(out, i);
      juce::MemoryBlock imageData(out.getData(), out.getDataSize());
      images->setAttribute("image" + juce::String(i), imageData.toBase64Encoding());
    }
    xml->addChildElement(images);
    return xml;
  }

  // Save image files
  bool saveSpecImage(juce::OutputStream& outputStream, size_t index) {
    juce::PNGImageFormat pngWriter;
    if (index >= specImages.size() || !specImages[index].isValid()) {
      DBG("saveSpecImage failed\nindex = " << index << "\nspecImage.size = " << specImages.size());
      return false;
    }
    return pngWriter.writeImageToStream(specImages[index], outputStream);
  }

  juce::String fileName = "-- init --";        // currently being viewed
  juce::String loadedFileName = "";  // name of what was loaded last
  juce::Range<double> trimRange;
  // default when new instance is loaded
  int pitchClass = Utils::PitchClass::C;

  // ArcSpectrogram related items
  SpecType specType = ParamUI::SpecType::INVALID;
  std::array<juce::Image, SpecType::COUNT> specImages;
  // Where ArcSpectrogram can let others know when it is "complete"
  // Makes no sense to save to preset file
  bool specComplete = false;
  bool isLoading = false;

  // Tracks what component is being displayed
  enum class CenterComponent { ARC_SPEC, TRIM_SELECTION };
  CenterComponent centerComponent = CenterComponent::ARC_SPEC;

  // trim selection status to pass information to the synth so it can pipe the audio out the main output
  bool trimPlaybackOn = false;
  int trimPlaybackSample;  // sampling buffer index position
  int trimPlaybackMaxSample;
};

class Parameters {
public:
  class Listener
  {
  public:
    virtual ~Listener() = default;
    // Called when selected parameters changes
    virtual void selectedCommonParamsChanged(ParamCommon* newParams) {}
    // Called when the current modulator mapping source changes
    virtual void mappingSourceChanged(ModSource* mod) {}
  };
  
  Parameters() {
    mSelectedParams = &global; // Init to using global params
  }
  
  // The 3 types of parameter sets
  ParamUI ui;
  ParamGlobal global;
  ParamsNote note;
  
  juce::HashMap<int, Modulation> modulations;
  
  ModSource* getMappingModSource() { return mMappingModSource; }
  void setMappingModSource(ModSource* mod) {
    mMappingModSource = mod;
    mListeners.call(&Parameters::Listener::mappingSourceChanged, mMappingModSource);
  }
  
  // Listeners for callbacks relating to parameters
  void addListener(Parameters::Listener* listener);
  void removeListener(Parameters::Listener* listener);
  
  // Returns the currently selected pitch class, or NONE if global selected
  Utils::PitchClass getSelectedPitchClass();
  juce::Colour getSelectedParamColour();
  ParamCommon* getSelectedParams() {
    return mSelectedParams;
  }
  void setSelectedParams(ParamCommon* params) {
    jassert(params);
    mSelectedParams = params;
    mListeners.call(&Parameters::Listener::selectedCommonParamsChanged, mSelectedParams);
  }
  
  // Modulation processing
  void prepareModSources(int blockSize, double sampleRate);
  void processModSources();
  void applyModulations(juce::RangedAudioParameter* param, float& value0To1);
  
  // Returns the candidate used in a given generator
  ParamCandidate* getGeneratorCandidate(ParamGenerator* gen);
  
  juce::RangedAudioParameter* getUsedParam(ParamCommon* common, ParamCommon::Type type);

  // Finds the lowest level parameter that's different from its parent
  // Hierarchy (high to low): global, note, generator
  // Optionally applies modulations before returning value
  float getFloatParam(ParamCommon* common, ParamCommon::Type type, bool withModulations = false);
  float getFloatParam(juce::RangedAudioParameter* param, bool withModulations = false);
  int getChoiceParam(ParamCommon* common, ParamCommon::Type type);
  int getBoolParam(ParamCommon* common, ParamCommon::Type type);
  
private:
  // Keeps track of the current selected global/note/generator parameters for editing, global by default
  ParamCommon* mSelectedParams = &global;
  
  ModSource* mMappingModSource = nullptr; // If not null, then a modulator is waiting to be mapped
  
  // Listener list for our callbacks
  juce::ListenerList<Listener> mListeners;
};
