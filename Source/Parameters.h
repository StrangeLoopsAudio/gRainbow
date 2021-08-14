/*
  ==============================================================================

    Parameters.h
    Created: 10 Aug 2021 6:27:45pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace ParamIDs {
// Generator params
static juce::String genEnable{"_enable_gen_"};
static juce::String genSolo{"_solo_gen_"};
static juce::String genCandidate{"_candidate_gen_"};
static juce::String genPitchAdjust{"_pitch_adjust_gen_"};
static juce::String genPositionAdjust{"_position_adjust_gen_"};
static juce::String genGrainShape{"_grain_shape_gen_"};
static juce::String genGrainTilt{"_grain_tilt_gen_"};
static juce::String genGrainRate{"_grain_rate_gen_"};
static juce::String genGrainDuration{"_grain_duration_gen_"};
static juce::String genGrainGain{"_grain_gain_gen_"};
static juce::String genAttack{"_attack_gen_"};
static juce::String genDecay{"_decay_gen_"};
static juce::String genSustain{"_sustain_gen_"};
static juce::String genRelease{"_release_gen_"};
// Global params
static juce::String globalAttack{"global_attack"};
static juce::String globalDecay{"global_decay"};
static juce::String globalSustain{"global_sustain"};
static juce::String globalRelease{"global_release"};
}  // namespace ParamIDs

namespace ParamRanges {
static juce::NormalisableRange<float> PITCH_ADJUST(-0.25f, 0.25f);
static juce::NormalisableRange<float> POSITION_ADJUST(-0.5f, 0.5f);
static juce::NormalisableRange<float> GRAIN_RATE(0.25f, 1.0f);
static juce::NormalisableRange<float> GRAIN_DURATION(0.06f, 0.3f);
static juce::NormalisableRange<float> ATTACK(0.01f, 2.0f);
static juce::NormalisableRange<float> DECAY(0.01f, 2.0f);
static juce::NormalisableRange<float> RELEASE(0.01f, 2.0f);
}  // namespace ParamRanges

namespace ParamDefaults {
static float GRAIN_RATE_DEFAULT = 0.5f;
static float GRAIN_DURATION_DEFAULT = 0.1f;
static float GRAIN_GAIN_DEFAULT = 0.8f;
static float ATTACK_DEFAULT_SEC = 0.2f;
static float DECAY_DEFAULT_SEC = 0.2f;
static float SUSTAIN_DEFAULT = 0.8f;
static float RELEASE_DEFAULT_SEC = 0.2f;
}  // namespace ParamDefaults

static juce::Array<juce::String> PITCH_CLASS_NAMES{
    "C", "Cs", "D", "Ds", "E", "F", "Fs", "G", "Gs", "A", "As", "B"};

struct ParamHelper {
  static juce::String getParamID(juce::AudioProcessorParameter* param) {
    if (auto paramWithID =
            dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
      return paramWithID->paramID;

    return param->getName(50);
  }
  // Utility function to avoid ugly dereferencing code before sending norm value
  // to host
  static void setParam(juce::AudioParameterFloat* param, float newValue) {
    *param = newValue;
  }
  static void setParam(juce::AudioParameterInt* param, int newValue) {
    *param = newValue;
  }
  static void setParam(juce::AudioParameterBool* param, bool newValue) {
    *param = newValue;
  }
};

static constexpr auto MAX_CANDIDATES = 6;
static constexpr auto NUM_NOTES = 12;
static constexpr auto NUM_GENERATORS = 4;
static constexpr auto ENV_LUT_SIZE = 128;
static constexpr auto SOLO_NONE = -1;

struct CandidateParams {
  float posRatio;
  float pbRate;
  float duration;
  float salience;

  CandidateParams(float posRatio, float pbRate, float duration, float salience)
      : posRatio(posRatio),
        pbRate(pbRate),
        duration(duration),
        salience(salience) {}
};

struct GeneratorParams : juce::AudioProcessorParameter::Listener {
  GeneratorParams(int noteIdx, int genIdx) : noteIdx(noteIdx), genIdx(genIdx) {}
  ~GeneratorParams() {
    grainShape->removeListener(this);
    grainTilt->removeListener(this);
  }

  void parameterValueChanged(int, float) override { updateGrainEnvelope(); };
  void parameterGestureChanged(int, bool) override {}

  void addParams(juce::AudioProcessor& p);
  void addListener(juce::AudioProcessorParameter::Listener* listener);
  void removeListener(juce::AudioProcessorParameter::Listener* listener);
  void updateGrainEnvelope();

  int noteIdx;
  int genIdx;

  juce::AudioParameterBool* enable = nullptr;
  juce::AudioParameterInt* candidate = nullptr;
  juce::AudioParameterFloat* pitchAdjust = nullptr;
  juce::AudioParameterFloat* positionAdjust = nullptr;
  juce::AudioParameterFloat* grainShape = nullptr;
  juce::AudioParameterFloat* grainTilt = nullptr;
  juce::AudioParameterFloat* grainRate = nullptr;
  juce::AudioParameterFloat* grainDuration = nullptr;
  juce::AudioParameterFloat* grainGain = nullptr;
  juce::AudioParameterFloat* attack = nullptr;
  juce::AudioParameterFloat* decay = nullptr;
  juce::AudioParameterFloat* sustain = nullptr;
  juce::AudioParameterFloat* release = nullptr;
  std::vector<float> grainEnv;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratorParams)
};

struct NoteParam {
  NoteParam(int noteIdx) : noteIdx(noteIdx) {
    for (int i = 0; i < NUM_GENERATORS; ++i) {
      generators.emplace_back(new GeneratorParams(noteIdx, i));
    }
  }

  void addParams(juce::AudioProcessor& p);
  bool shouldPlayGenerator(int genIdx);
  CandidateParams* getCandidate(int genIdx) {
    if (genIdx >= candidates.size()) return nullptr;
    return &candidates[generators[genIdx]->candidate->get()];
  }

  void grainCreated(int genIdx, float envGain) {
    if (onGrainCreated != nullptr) onGrainCreated(genIdx, envGain);
  }
  std::function<void(int genIdx, float envGain)> onGrainCreated = nullptr;

  int noteIdx;
  std::vector<std::unique_ptr<GeneratorParams>> generators;
  std::vector<CandidateParams> candidates;
  juce::AudioParameterInt* soloIdx = nullptr;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteParam)
};

struct NoteParams {
  NoteParams() {
    for (int i = 0; i < NUM_NOTES; ++i) {
      notes.emplace_back(new NoteParam(i));
    }
  }

  void addParams(juce::AudioProcessor& p);
  void resetParams();

  std::vector<std::unique_ptr<NoteParam>> notes;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteParams)
};

struct GlobalParams {
  GlobalParams() {}

  void addParams(juce::AudioProcessor& p);
  void resetParams();

  juce::AudioParameterFloat* attack = nullptr;
  juce::AudioParameterFloat* decay = nullptr;
  juce::AudioParameterFloat* sustain = nullptr;
  juce::AudioParameterFloat* release = nullptr;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalParams)
};

/**
 * A representation of the last UI settings to restore it when loading the
 * editor
 */
struct UIParams {
  UIParams() = default;
  // Get it from the plugin state
  UIParams(juce::XmlElement* xml) {
    if (xml != nullptr) {
      generatorTab = xml->getIntAttribute("generatorTab");
      pitchClass = xml->getIntAttribute("pitchClass");
      specType = xml->getIntAttribute("specType");
    }
  }

  // Build the XML representation to save in plugin state.
  juce::XmlElement* getXml() {
    juce::XmlElement* xml = new juce::XmlElement("UIParams");
    xml->setAttribute("generatorTab", generatorTab);
    xml->setAttribute("pitchClass", pitchClass);
    xml->setAttribute("specType", specType);
    return xml;
  }

  int generatorTab = 0;
  int pitchClass = 0;
  int specType = 0;
};