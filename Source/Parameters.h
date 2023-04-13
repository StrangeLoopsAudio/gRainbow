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

#include "Utils.h"

// Dynamically casts to AudioParameterFloat*
#define P_FLOAT(X) dynamic_cast<juce::AudioParameterFloat*>(X)
// Dynamically casts to AudioParameterChoice*
#define P_CHOICE(X) dynamic_cast<juce::AudioParameterChoice*>(X)
// Dynamically casts to AudioParameterBool*
#define P_BOOL(X) dynamic_cast<juce::AudioParameterBool*>(X)

namespace ParamIDs {
// Note params
static juce::String genSolo{"_solo_gen"};
static juce::String noteGain{"_note_gain"};
static juce::String noteAttack{"_note_attack"};
static juce::String noteDecay{"_note_decay"};
static juce::String noteSustain{"_note_sustain"};
static juce::String noteRelease{"_note_release"};
static juce::String noteFilterCutoff{"_note_filt_cutoff"};
static juce::String noteFilterResonance{"_note_filt_resonance"};
static juce::String noteFilterType{"_note_filt_type"};
static juce::String noteGrainShape{"_note_grain_shape"};
static juce::String noteGrainTilt{"_note_grain_tilt"};
static juce::String noteGrainRate{"_note_grain_rate"};
static juce::String noteGrainDuration{"_note_grain_duration"};
static juce::String noteGrainSync{"_note_grain_sync"};
static juce::String notePitchAdjust{"_note_pitch_adjust"};
static juce::String notePitchSpray{"_note_pitch_spray"};
static juce::String notePositionAdjust{"_note_position_adjust"};
static juce::String notePositionSpray{"_note_position_spray"};
// Generator params
static juce::String genEnable{"_enable_gen_"};
static juce::String genCandidate{"_candidate_gen_"};
static juce::String genGain{"_gain_gen_"};
static juce::String genAttack{"_attack_gen_"};
static juce::String genDecay{"_decay_gen_"};
static juce::String genSustain{"_sustain_gen_"};
static juce::String genRelease{"_release_gen_"};
static juce::String genFilterCutoff{"_filt_cutoff_gen_"};
static juce::String genFilterResonance{"_filt_resonance_gen_"};
static juce::String genFilterType{"_filt_type_gen_"};
static juce::String genGrainShape{"_grain_shape_gen_"};
static juce::String genGrainTilt{"_grain_tilt_gen_"};
static juce::String genGrainRate{"_grain_rate_gen_"};
static juce::String genGrainDuration{"_grain_duration_gen_"};
static juce::String genGrainSync{"_grain_sync_gen_"};
static juce::String genPitchAdjust{"_pitch_adjust_gen_"};
static juce::String genPitchSpray{"_pitch_spray_gen_"};
static juce::String genPositionAdjust{"_position_adjust_gen_"};
static juce::String genPositionSpray{"_position_spray_gen_"};
// Global params
static juce::String globalGain{"global_gain"};
static juce::String globalAttack{"global_attack"};
static juce::String globalDecay{"global_decay"};
static juce::String globalSustain{"global_sustain"};
static juce::String globalRelease{"global_release"};
static juce::String globalFilterCutoff{"global_filt_cutoff"};
static juce::String globalFilterResonance{"global_filt_resonance"};
static juce::String globalFilterType{"global_filt_type"};
static juce::String globalGrainShape{"global_grain_shape"};
static juce::String globalGrainTilt{"global_grain_tilt"};
static juce::String globalGrainRate{"global_grain_rate"};
static juce::String globalGrainDuration{"global_grain_duration"};
static juce::String globalGrainSync{"global_grain_sync"};
static juce::String globalPitchAdjust{"global_pitch_adjust"};
static juce::String globalPitchSpray{"global_pitch_spray"};
static juce::String globalPositionAdjust{"global_position_adjust"};
static juce::String globalPositionSpray{"global_position_spray"};
}  // namespace ParamIDs

namespace ParamRanges {
static juce::NormalisableRange<float> GAIN(0.0f, 1.0f);
static juce::NormalisableRange<float> ATTACK(0.01f, 2.0f);
static juce::NormalisableRange<float> DECAY(0.01f, 2.0f);
static juce::NormalisableRange<float> SUSTAIN(0.0f, 1.0f);
static juce::NormalisableRange<float> RELEASE(0.01f, 2.0f);
static juce::NormalisableRange<float> CUTOFF(100.0f, 10000.0f);
static juce::NormalisableRange<float> RESONANCE(0.01f, 1.0f);
static juce::NormalisableRange<float> GRAIN_SHAPE(0.0f, 1.0f);
static juce::NormalisableRange<float> GRAIN_TILT(0.0f, 1.0f);
static juce::NormalisableRange<float> GRAIN_RATE(0.25f, 1.0f);
static juce::NormalisableRange<float> GRAIN_DURATION(0.06f, 0.3f);
static juce::NormalisableRange<float> PITCH_ADJUST(-0.25f, 0.25f);
static juce::NormalisableRange<float> PITCH_SPRAY(0.0f, 0.1f);
static juce::NormalisableRange<float> POSITION_ADJUST(-0.5f, 0.5f);
static juce::NormalisableRange<float> POSITION_SPRAY(0.0f, 0.3f);

static int SYNC_DIV_MAX = 4;  // pow of 2 division, so 1/16
}  // namespace ParamRanges

namespace ParamDefaults {
static float GAIN_DEFAULT = 0.8f;
static float ATTACK_DEFAULT_SEC = 0.2f;
static float DECAY_DEFAULT_SEC = 0.2f;
static float SUSTAIN_DEFAULT = 0.8f;
static float RELEASE_DEFAULT_SEC = 0.2f;
static float FILTER_LP_CUTOFF_DEFAULT_HZ = 5000.0f;
static float FILTER_HP_CUTOFF_DEFAULT_HZ = 800.0f;
static float FILTER_BP_CUTOFF_DEFAULT_HZ = 1200.0f;
static float FILTER_RESONANCE_DEFAULT = 0.707f;
static juce::String FILTER_TYPE_DEFAULT = "none";
static float GRAIN_SHAPE_DEFAULT = 0.75f;
static float GRAIN_TILT_DEFAULT = 0.5f;
static float GRAIN_RATE_DEFAULT = 0.33f;
static float GRAIN_DURATION_DEFAULT = 0.2f;
static float PITCH_ADJUST_DEFAULT = 0.0f;
static float PITCH_SPRAY_DEFAULT = 0.01f;
static float POSITION_ADJUST_DEFAULT = 0.0f;
static float POSITION_SPRAY_DEFAULT = 0.0f;
}  // namespace ParamDefaults

enum ParamType { GLOBAL, NOTE, GENERATOR };
static juce::Array<juce::String> PITCH_CLASS_NAMES{"C", "Cs", "D", "Ds", "E", "F", "Fs", "G", "Gs", "A", "As", "B"};
static juce::Array<juce::String> FILTER_TYPE_NAMES{"none", "lowpass", "highpass", "bandpass"};

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
  static void setParam(juce::AudioParameterChoice* param, int newValue) { *param = newValue; }
};

static constexpr auto MAX_CANDIDATES = 6;
static constexpr auto NUM_GENERATORS = 4;
static constexpr auto SOLO_NONE = -1;
static constexpr auto NUM_FILTER_TYPES = 3;
static constexpr auto ENV_LUT_SIZE = 128;  // grain env lookup table size

// Common parameters types used by each generator, note and globally
class ParamCommon : public juce::AudioProcessorParameter::Listener {
 public:
  ParamCommon(ParamType type) : type(type) {
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
    updateGrainEnvelopeLUT(grainEnvLUT, ParamDefaults::GRAIN_SHAPE_DEFAULT, ParamDefaults::GRAIN_TILT_DEFAULT);
  }
  ~ParamCommon() {
    common[GRAIN_SHAPE]->removeListener(this);
    common[GRAIN_TILT]->removeListener(this);
    common[FILT_TYPE]->removeListener(this);
    common[FILT_CUTOFF]->removeListener(this);
    common[FILT_RESONANCE]->removeListener(this);
  }

  enum Type {
    GAIN,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    FILT_CUTOFF,
    FILT_RESONANCE,
    FILT_TYPE,
    GRAIN_SHAPE,
    GRAIN_TILT,
    GRAIN_RATE,
    GRAIN_DURATION,
    GRAIN_SYNC,
    PITCH_ADJUST,
    PITCH_SPRAY,
    POS_ADJUST,
    POS_SPRAY,
    NUM_COMMON
  };

  void addListener(juce::AudioProcessorParameter::Listener* listener) {
    common[GAIN]->addListener(listener);
    common[ATTACK]->addListener(listener);
    common[DECAY]->addListener(listener);
    common[SUSTAIN]->addListener(listener);
    common[RELEASE]->addListener(listener);
    common[FILT_CUTOFF]->addListener(listener);
    common[FILT_RESONANCE]->addListener(listener);
    common[FILT_TYPE]->addListener(listener);
    common[GRAIN_SHAPE]->addListener(listener);
    common[GRAIN_TILT]->addListener(listener);
    common[GRAIN_RATE]->addListener(listener);
    common[GRAIN_DURATION]->addListener(listener);
    common[GRAIN_SYNC]->addListener(listener);
    common[PITCH_ADJUST]->addListener(listener);
    common[PITCH_SPRAY]->addListener(listener);
    common[POS_ADJUST]->addListener(listener);
    common[POS_SPRAY]->addListener(listener);
  }
  void removeListener(juce::AudioProcessorParameter::Listener* listener) {
    common[GAIN]->removeListener(listener);
    common[ATTACK]->removeListener(listener);
    common[DECAY]->removeListener(listener);
    common[SUSTAIN]->removeListener(listener);
    common[RELEASE]->removeListener(listener);
    common[FILT_CUTOFF]->removeListener(listener);
    common[FILT_RESONANCE]->removeListener(listener);
    common[FILT_TYPE]->removeListener(listener);
    common[GRAIN_SHAPE]->removeListener(listener);
    common[GRAIN_TILT]->removeListener(listener);
    common[GRAIN_RATE]->removeListener(listener);
    common[GRAIN_DURATION]->removeListener(listener);
    common[GRAIN_SYNC]->removeListener(listener);
    common[PITCH_ADJUST]->removeListener(listener);
    common[PITCH_SPRAY]->removeListener(listener);
    common[POS_ADJUST]->removeListener(listener);
    common[POS_SPRAY]->removeListener(listener);
  }

  void resetParams(bool fullClear = true) {
    ParamHelper::setParam(P_FLOAT(common[GAIN]), ParamDefaults::GAIN_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[ATTACK]), ParamDefaults::ATTACK_DEFAULT_SEC);
    ParamHelper::setParam(P_FLOAT(common[DECAY]), ParamDefaults::DECAY_DEFAULT_SEC);
    ParamHelper::setParam(P_FLOAT(common[SUSTAIN]), ParamDefaults::SUSTAIN_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[RELEASE]), ParamDefaults::RELEASE_DEFAULT_SEC);
    ParamHelper::setParam(P_FLOAT(common[FILT_CUTOFF]), ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ);
    ParamHelper::setParam(P_FLOAT(common[FILT_RESONANCE]), ParamDefaults::FILTER_RESONANCE_DEFAULT);
    ParamHelper::setParam(P_CHOICE(common[FILT_TYPE]), 0);
    ParamHelper::setParam(P_FLOAT(common[PITCH_ADJUST]), 0.0f);
    ParamHelper::setParam(P_FLOAT(common[PITCH_SPRAY]), 0.0f);
    ParamHelper::setParam(P_FLOAT(common[POS_ADJUST]), 0.0f);
    ParamHelper::setParam(P_FLOAT(common[POS_SPRAY]), ParamDefaults::POSITION_SPRAY_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[GRAIN_SHAPE]), 0.5f);
    ParamHelper::setParam(P_FLOAT(common[GRAIN_TILT]), 0.5f);
    ParamHelper::setParam(P_FLOAT(common[GRAIN_RATE]), ParamDefaults::GRAIN_RATE_DEFAULT);
    ParamHelper::setParam(P_FLOAT(common[GRAIN_DURATION]), ParamDefaults::GRAIN_DURATION_DEFAULT);
    ParamHelper::setParam(P_BOOL(common[GRAIN_SYNC]), false);
  }

  void parameterValueChanged(int paramIdx, float newValue) override {
    if (paramIdx == common[GRAIN_SHAPE]->getParameterIndex() || paramIdx == common[GRAIN_TILT]->getParameterIndex()) {
      updateGrainEnvelopeLUT(grainEnvLUT, P_FLOAT(common[GRAIN_SHAPE])->get(), P_FLOAT(common[GRAIN_TILT])->get());
    } else if (paramIdx == common[FILT_TYPE]->getParameterIndex()) {
      switch (P_CHOICE(common[FILT_TYPE])->getIndex()) {
        case Utils::FilterType::LOWPASS: {
          filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
          break;
        }
        case Utils::FilterType::HIGHPASS: {
          filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
          break;
        }
        case Utils::FilterType::BANDPASS: {
          filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
          break;
        }
        default:
          break;
      }
    } else if (paramIdx == common[FILT_CUTOFF]->getParameterIndex()) {
      filter.setCutoffFrequency(P_FLOAT(common[FILT_CUTOFF])->get());
    } else if (paramIdx == common[FILT_RESONANCE]->getParameterIndex()) {
      filter.setResonance(P_FLOAT(common[FILT_RESONANCE])->get());
    }
  };
  void parameterGestureChanged(int, bool) override {}

  juce::RangedAudioParameter* common[Type::NUM_COMMON];

  // Type of derived class
  ParamType type;
  // LUT of the grain envelope
  std::vector<float> grainEnvLUT;
  // State variable filter for generator
  juce::dsp::StateVariableTPTFilter<float> filter;
  double sampleRate = 48000;

 private:
  void updateGrainEnvelopeLUT(std::vector<float>& lut, float shape, float tilt) {
    lut.clear();
    /* LUT divided into 3 parts

                 1.0
                -----
       rampUp  /     \  rampDown
              /       \
    */
    float scaledShape = (shape * ENV_LUT_SIZE) / 2.0f;
    float scaledTilt = tilt * ENV_LUT_SIZE;
    int rampUpEndSample = juce::jmax(0.0f, scaledTilt - scaledShape);
    int rampDownStartSample = juce::jmin((float)ENV_LUT_SIZE, scaledTilt + scaledShape);
    for (int i = 0; i < ENV_LUT_SIZE; i++) {
      if (i < rampUpEndSample) {
        lut.push_back((float)i / rampUpEndSample);
      } else if (i > rampDownStartSample) {
        lut.push_back(1.0f - (float)(i - rampDownStartSample) / (ENV_LUT_SIZE - rampDownStartSample));
      } else {
        lut.push_back(1.0f);
      }
    }
    juce::FloatVectorOperations::clip(lut.data(), lut.data(), 0.0f, 1.0f, lut.size());
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamCommon)
};

static float COMMON_DEFAULTS[ParamCommon::Type::NUM_COMMON] = {ParamDefaults::GAIN_DEFAULT,
                                                               ParamDefaults::ATTACK_DEFAULT_SEC,
                                                               ParamDefaults::DECAY_DEFAULT_SEC,
                                                               ParamDefaults::SUSTAIN_DEFAULT,
                                                               ParamDefaults::RELEASE_DEFAULT_SEC,
                                                               ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ,
                                                               ParamDefaults::FILTER_RESONANCE_DEFAULT,
                                                               0,
                                                               ParamDefaults::GRAIN_SHAPE_DEFAULT,
                                                               ParamDefaults::GRAIN_TILT_DEFAULT,
                                                               ParamDefaults::GRAIN_RATE_DEFAULT,
                                                               ParamDefaults::GRAIN_DURATION_DEFAULT,
                                                               0,
                                                               ParamDefaults::PITCH_ADJUST_DEFAULT,
                                                               ParamDefaults::PITCH_SPRAY_DEFAULT,
                                                               ParamDefaults::POSITION_ADJUST_DEFAULT,
                                                               ParamDefaults::POSITION_SPRAY_DEFAULT};

struct ParamCandidate {
  float posRatio;
  float pbRate;
  float duration;
  float salience;

  ParamCandidate(float posRatio, float pbRate, float duration, float salience)
      : posRatio(posRatio), pbRate(pbRate), duration(duration), salience(salience) {}

  // setXml equivalent since we always need a valid candidate param  value
  ParamCandidate(juce::XmlElement* xml) {
    jassert(xml->hasTagName("ParamCandidate"));
    posRatio = xml->getDoubleAttribute("posRatio");
    pbRate = xml->getDoubleAttribute("pbRate");
    duration = xml->getDoubleAttribute("duration");
    salience = xml->getDoubleAttribute("salience");
  }

  juce::XmlElement* getXml() {
    juce::XmlElement* xml = new juce::XmlElement("ParamCandidate");
    xml->setAttribute("posRatio", posRatio);
    xml->setAttribute("pbRate", pbRate);
    xml->setAttribute("duration", duration);
    xml->setAttribute("salience", salience);
    return xml;
  }
};

struct ParamGenerator : ParamCommon {
  ParamGenerator(int noteIdx, int genIdx) : ParamCommon(ParamType::GENERATOR), noteIdx(noteIdx), genIdx(genIdx) {}
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

  int noteIdx;
  int genIdx;

  juce::AudioParameterBool* enable = nullptr;
  juce::AudioParameterInt* candidate = nullptr;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamGenerator)
};

struct ParamNote : ParamCommon {
  ParamNote(int noteIdx) : ParamCommon(ParamType::NOTE), noteIdx(noteIdx), numActiveGens(1) {
    for (int i = 0; i < NUM_GENERATORS; ++i) {
      generators.emplace_back(new ParamGenerator(noteIdx, i));
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
    ParamCommon::resetParams(fullClear);
    for (auto& generator : generators) {
      generator->resetParams();
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
  int numActiveGens;

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
    for (int i = 0; i < Utils::PitchClass::COUNT; ++i) {
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
      notes[i].get()->setXml(xml->getChildElement(i));
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamsNote)
};

struct ParamGlobal : ParamCommon {
  ParamGlobal() : ParamCommon(ParamType::GLOBAL) {}
  ~ParamGlobal() {}

  void addParams(juce::AudioProcessor& p);

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
      generatorTab = xml->getIntAttribute("generatorTab");
      pitchClass = xml->getIntAttribute("pitchClass");
      specType = (SpecType)xml->getIntAttribute("specType");
    }
  }

  // Build the XML representation to save in plugin state.
  juce::XmlElement* getXml() {
    juce::XmlElement* xml = new juce::XmlElement("ParamUI");
    xml->setAttribute("fileName", loadedFileName);
    xml->setAttribute("generatorTab", generatorTab);
    xml->setAttribute("pitchClass", pitchClass);
    xml->setAttribute("specType", static_cast<int>(specType));
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

  juce::String fileName = "";        // currently being viewed
  juce::String loadedFileName = "";  // name of what was loaded last
  int generatorTab = 0;
  // default when new instance is loaded
  int pitchClass = Utils::PitchClass::C;

  // ArcSpectrogram related items
  SpecType specType = ParamUI::SpecType::INVALID;
  std::array<juce::Image, SpecType::COUNT> specImages;
  // Where ArcSpectrogram can let others know when it is "complete"
  // Makes no scenes to save to preset file
  bool specComplete = false;

  // Tracks what component is being displayed
  enum class CenterComponent { LOGO, ARC_SPEC, TRIM_SELECTION };
  CenterComponent centerComponent = CenterComponent::LOGO;

  // trim selection status to pass information to the synth so it can pipe the audio out the main output
  bool trimPlaybackOn = false;
  int trimPlaybackSample;  // sampling buffer index position
  int trimPlaybackMaxSample;
};

struct Parameters {
  // The 3 types of parameter sets
  ParamUI ui;
  ParamGlobal global;
  ParamsNote note;

  // Called when current selected note or generator changes
  // Should be used only by PluginEditor and passed on to subcomponents
  std::function<void()> onSelectedChange = nullptr;
  
  // Keeps track of the current selected global/note/generator parameters for editing, global by default
  ParamCommon* selectedParams = &global;
  juce::Colour getSelectedParamColour() {
    switch (selectedParams->type) {
      case ParamType::GLOBAL:
        return Utils::GLOBAL_COLOUR;
        break;
      case ParamType::NOTE:
        return Utils::getRainbow12Colour(dynamic_cast<ParamNote*>(selectedParams)->noteIdx);
        break;
      case ParamType::GENERATOR:
        ParamGenerator* gen = dynamic_cast<ParamGenerator*>(selectedParams);
        return Utils::getRainbow12Colour(gen->noteIdx).brighter(gen->genIdx * Utils::GENERATOR_BRIGHTNESS_ADD);
        break;
    }
    return juce::Colours::black;
  }

  // Finds the lowest level parameter that's different from its parent
  // Hierarchy (high to low): global, note, generator
  float getFloatParam(ParamGenerator* gen, ParamCommon::Type type) {
    float defaultVal = COMMON_DEFAULTS[type];
    // If gen value is different from default, return it
    juce::AudioParameterFloat* genParam = P_FLOAT(gen->common[type]);
    if (genParam->get() != defaultVal) {
      return genParam->get();
    }
    // Otherwise if note value is different from default, return it
    juce::AudioParameterFloat* noteParam = P_FLOAT(note.notes[gen->noteIdx]->common[type]);
    if (noteParam->get() != defaultVal) {
      return noteParam->get();
    }
    // Both note and generator are still defaults, so let's use the global value
    return P_FLOAT(global.common[type])->get();
  }
  int getChoiceParam(ParamGenerator* gen, ParamCommon::Type type) {
    int defaultVal = COMMON_DEFAULTS[type];
    // If gen value is different from default, return it
    juce::AudioParameterChoice* genParam = P_CHOICE(gen->common[type]);
    if (genParam->getIndex() != defaultVal) {
      return genParam->getIndex();
    }
    // Otherwise if note value is different from default, return it
    juce::AudioParameterChoice* noteParam = P_CHOICE(note.notes[gen->noteIdx]->common[type]);
    if (noteParam->getIndex() != defaultVal) {
      return noteParam->getIndex();
    }
    // Both note and generator are still defaults, so let's use the global value
    return P_CHOICE(global.common[type])->getIndex();
  }
  int getBoolParam(ParamGenerator* gen, ParamCommon::Type type) {
    int defaultVal = COMMON_DEFAULTS[type];
    // If gen value is different from default, return it
    juce::AudioParameterBool* genParam = P_BOOL(gen->common[type]);
    if (genParam->get() != defaultVal) {
      return genParam->get();
    }
    // Otherwise if note value is different from default, return it
    juce::AudioParameterBool* noteParam = P_BOOL(note.notes[gen->noteIdx]->common[type]);
    if (noteParam->get() != defaultVal) {
      return noteParam->get();
    }
    // Both note and generator are still defaults, so let's use the global value
    return P_BOOL(global.common[type])->get();
  }
};