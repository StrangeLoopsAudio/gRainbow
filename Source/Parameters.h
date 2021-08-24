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
// Note params
static juce::String genSolo{"_solo_gen"};
// Generator params
static juce::String genEnable{"_enable_gen_"};
static juce::String genGain{"_gain_gen_"};
static juce::String genCandidate{"_candidate_gen_"};
static juce::String genPitchAdjust{"_pitch_adjust_gen_"};
static juce::String genPitchSpray{"_pitch_spray_gen_"};
static juce::String genPositionAdjust{"_position_adjust_gen_"};
static juce::String genPositionSpray{"_position_spray_gen_"};
static juce::String genGrainShape{"_grain_shape_gen_"};
static juce::String genGrainTilt{"_grain_tilt_gen_"};
static juce::String genGrainRate{"_grain_rate_gen_"};
static juce::String genGrainDuration{"_grain_duration_gen_"};
static juce::String genGrainSync{"_grain_sync_gen_"};
static juce::String genAttack{"_attack_gen_"};
static juce::String genDecay{"_decay_gen_"};
static juce::String genSustain{"_sustain_gen_"};
static juce::String genRelease{"_release_gen_"};
static juce::String genCutoff{"_cutoff_gen_"};
static juce::String genStrength{"_strength_gen_"};
static juce::String genFilterType{"_filter_type_gen_"};
// Global params
static juce::String globalGain{"global_gain"};
static juce::String globalAttack{"global_attack"};
static juce::String globalDecay{"global_decay"};
static juce::String globalSustain{"global_sustain"};
static juce::String globalRelease{"global_release"};
}  // namespace ParamIDs

namespace ParamRanges {
static juce::NormalisableRange<float> PITCH_ADJUST(-0.25f, 0.25f);
static juce::NormalisableRange<float> PITCH_SPRAY(0.0f, 0.1f);
static juce::NormalisableRange<float> POSITION_ADJUST(-0.5f, 0.5f);
static juce::NormalisableRange<float> POSITION_SPRAY(0.0f, 0.3f);
static juce::NormalisableRange<float> GRAIN_RATE(0.25f, 1.0f);
static juce::NormalisableRange<float> GRAIN_DURATION(0.06f, 0.3f);
static juce::NormalisableRange<float> ATTACK(0.01f, 2.0f);
static juce::NormalisableRange<float> DECAY(0.01f, 2.0f);
static juce::NormalisableRange<float> RELEASE(0.01f, 2.0f);
static juce::NormalisableRange<float> CUTOFF(100.0f, 10000.0f);
static juce::NormalisableRange<float> STRENGTH(0.01f, 1.0f);
static int SYNC_DIV_MAX = 4;  // pow of 2 division, so 1/16
}  // namespace ParamRanges

namespace ParamDefaults {
static float POSITION_SPRAY_DEFAULT = 0.01f;
static float GRAIN_RATE_DEFAULT = 0.5f;
static float GRAIN_DURATION_DEFAULT = 0.1f;
static float GAIN_DEFAULT = 0.8f;
static float ATTACK_DEFAULT_SEC = 0.2f;
static float DECAY_DEFAULT_SEC = 0.2f;
static float SUSTAIN_DEFAULT = 0.8f;
static float RELEASE_DEFAULT_SEC = 0.2f;
static float STRENGTH_DEFAULT = 0.5f;
static float CUTOFF_DEFAULT = 50000.0f;
static juce::String FILTER_TYPE_DEFAULT = "low_pass";
}  // namespace ParamDefaults

static juce::Array<juce::String> PITCH_CLASS_NAMES{"C", "Cs", "D", "Ds", "E", "F", "Fs", "G", "Gs", "A", "As", "B"};
static juce::Array<juce::String> FILTER_TYPE_NAMES{"low_pass", "high_pass", "band_pass"};

struct ParamHelper {
  static juce::String getParamID(juce::AudioProcessorParameter* param) {
    if (auto paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param)) return paramWithID->paramID;

    return param->getName(50);
  }
  // Utility function to avoid ugly dereferencing code before sending norm value
  // to host
  static void setParam(juce::AudioParameterFloat* param, float newValue) { *param = newValue; }
  static void setParam(juce::AudioParameterInt* param, int newValue) { *param = newValue; }
  static void setParam(juce::AudioParameterBool* param, bool newValue) { *param = newValue; }
};

static constexpr auto MAX_CANDIDATES = 6;
static constexpr auto NUM_NOTES = 12;
static constexpr auto NUM_GENERATORS = 4;
static constexpr auto SOLO_NONE = -1;

struct ParamCandidate {
  float posRatio;
  float pbRate;
  float duration;
  float salience;

  ParamCandidate(float posRatio, float pbRate, float duration, float salience)
      : posRatio(posRatio), pbRate(pbRate), duration(duration), salience(salience) {}

  // setUserStateXml equivalent since we always need a valid candidate param
  // value
  ParamCandidate(juce::XmlElement* xml) {
    jassert(xml->hasTagName("ParamCandidate"));
    posRatio = xml->getDoubleAttribute("posRatio");
    pbRate = xml->getDoubleAttribute("pbRate");
    duration = xml->getDoubleAttribute("duration");
    salience = xml->getDoubleAttribute("salience");
  }

  juce::XmlElement* getUserStateXml() {
    juce::XmlElement* xml = new juce::XmlElement("ParamCandidate");
    xml->setAttribute("posRatio", posRatio);
    xml->setAttribute("pbRate", pbRate);
    xml->setAttribute("duration", duration);
    xml->setAttribute("salience", salience);
    return xml;
  }
};

struct ParamGenerator : juce::AudioProcessorParameter::Listener {
  ParamGenerator(int noteIdx, int genIdx) : noteIdx(noteIdx), genIdx(genIdx) {}
  ~ParamGenerator() {
    grainShape->removeListener(this);
    grainTilt->removeListener(this);
  }

  void parameterValueChanged(int, float) override { updateGrainEnvelopeLUT(); };
  void parameterGestureChanged(int, bool) override {}

  void addParams(juce::AudioProcessor& p);
  void addListener(juce::AudioProcessorParameter::Listener* listener);
  void removeListener(juce::AudioProcessorParameter::Listener* listener);
  void updateGrainEnvelopeLUT();

  int noteIdx;
  int genIdx;

  juce::AudioParameterBool* enable = nullptr;
  juce::AudioParameterFloat* gain = nullptr;
  juce::AudioParameterInt* candidate = nullptr;
  juce::AudioParameterFloat* pitchAdjust = nullptr;
  juce::AudioParameterFloat* pitchSpray = nullptr;
  juce::AudioParameterFloat* positionAdjust = nullptr;
  juce::AudioParameterFloat* positionSpray = nullptr;
  juce::AudioParameterFloat* grainShape = nullptr;
  juce::AudioParameterFloat* grainTilt = nullptr;
  juce::AudioParameterFloat* grainRate = nullptr;
  juce::AudioParameterFloat* grainDuration = nullptr;
  juce::AudioParameterBool* grainSync = nullptr;
  juce::AudioParameterFloat* attack = nullptr;
  juce::AudioParameterFloat* decay = nullptr;
  juce::AudioParameterFloat* sustain = nullptr;
  juce::AudioParameterFloat* release = nullptr;
  juce::AudioParameterFloat* strength = nullptr;
  juce::AudioParameterFloat* cutoff = nullptr;
  juce::AudioParameterChoice* filterType = nullptr;

  // LUT of the grain envelope
  static constexpr auto ENV_LUT_SIZE = 128;
  std::vector<float> grainEnvLUT;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamGenerator)
};

struct ParamNote {
  ParamNote(int noteIdx) : noteIdx(noteIdx) {
    for (int i = 0; i < NUM_GENERATORS; ++i) {
      generators.emplace_back(new ParamGenerator(noteIdx, i));
    }
  }

  void addParams(juce::AudioProcessor& p);
  void addListener(int genIdx, juce::AudioProcessorParameter::Listener* listener);
  void removeListener(int genIdx, juce::AudioProcessorParameter::Listener* listener);
  bool shouldPlayGenerator(int genIdx);
  ParamCandidate* getCandidate(int genIdx);

  void grainCreated(int genIdx, float durationSec, float envGain) {
    if (onGrainCreated != nullptr) onGrainCreated(genIdx, durationSec, envGain);
  }
  std::function<void(int genIdx, float durationSec, float envGain)> onGrainCreated = nullptr;

  int noteIdx;
  std::vector<std::unique_ptr<ParamGenerator>> generators;
  std::vector<ParamCandidate> candidates;
  juce::AudioParameterInt* soloIdx = nullptr;

  juce::XmlElement* getUserStateXml() {
    juce::XmlElement* xml = new juce::XmlElement("ParamNote");
    for (ParamCandidate& candidate : candidates) {
      xml->addChildElement(candidate.getUserStateXml());
    }
    return xml;
  }

  void setUserStateXml(juce::XmlElement* xml) {
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
    for (int i = 0; i < NUM_NOTES; ++i) {
      notes.emplace_back(new ParamNote(i));
    }
  }

  void addParams(juce::AudioProcessor& p);
  void resetParams();

  std::vector<std::unique_ptr<ParamNote>> notes;

  juce::XmlElement* getUserStateXml() {
    juce::XmlElement* xml = new juce::XmlElement("NotesParams");
    for (auto&& note : notes) {
      xml->addChildElement(note->getUserStateXml());
    }
    return xml;
  }

  void setUserStateXml(juce::XmlElement* xml) {
    jassert(xml->hasTagName("NotesParams"));
    // Currently all child elements are NotesParam elements
    for (size_t i = 0; i < notes.size(); i++) {
      notes[i].get()->setUserStateXml(xml->getChildElement(i));
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamsNote)
};

struct ParamGlobal {
  ParamGlobal() {}

  void addParams(juce::AudioProcessor& p);
  void resetParams();

  juce::AudioParameterFloat* gain = nullptr;
  juce::AudioParameterFloat* attack = nullptr;
  juce::AudioParameterFloat* decay = nullptr;
  juce::AudioParameterFloat* sustain = nullptr;
  juce::AudioParameterFloat* release = nullptr;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamGlobal)
};

/**
 * A representation of the last UI settings to restore it when loading the
 * editor. The Synth owns this and used to allow state to be saved properly as
 * the UI is open and closed.
 */
struct ParamUI {
  ParamUI() = default;

  // Get it from the plugin state
  // will only set xml-able items (floats/int/strings)
  void setXml(juce::XmlElement* xml) {
    if (xml != nullptr) {
      fileName = xml->getStringAttribute("fileName");
      generatorTab = xml->getIntAttribute("generatorTab");
      pitchClass = xml->getIntAttribute("pitchClass");
      specType = xml->getIntAttribute("specType");
    }
  }

  // Build the XML representation to save in plugin state.
  juce::XmlElement* getXml() {
    juce::XmlElement* xml = new juce::XmlElement("ParamUI");
    xml->setAttribute("fileName", fileName);
    xml->setAttribute("generatorTab", generatorTab);
    xml->setAttribute("pitchClass", pitchClass);
    xml->setAttribute("specType", specType);
    return xml;
  }

  // Save image files
  bool saveSpecImage(juce::OutputStream& outputStream, size_t index) {
    juce::PNGImageFormat pngWriter;
    jassert(index < specImages.size());
    jassert(specImages[index].isValid());
    return pngWriter.writeImageToStream(specImages[index], outputStream);
  }

  enum SpecType { INVALID = -1, SPECTROGRAM = 0, HPCP, DETECTED, COUNT };

  juce::String fileName;
  int generatorTab = 0;
  int pitchClass = 0;

  // ArcSpectrogram related items
  int specType = 0;
  std::array<juce::Image, SpecType::COUNT> specImages;
  // Where ArcSpectrogram can let others know when it is "complete"
  // Makes no scenes to save to preset file
  bool specComplete = false;
};