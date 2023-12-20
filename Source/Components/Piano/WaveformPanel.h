/*
  ==============================================================================

    WaveformPanel.h
    Created: 15 Dec 2023 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "../Sliders.h"

//==============================================================================
/*
 */
class WaveformPanel : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  WaveformPanel(Parameters& parameters);
  ~WaveformPanel();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();
  
  bool isLoaded() { return mBuffer.getNumSamples() > 0; }
  void load(juce::AudioBuffer<float> &buffer);

 private:
  static constexpr int NUM_WAVE_BARS = 30;
  
  typedef struct WaveBar {
    WaveBar() {}
    WaveBar(float _magnitude, Utils::PitchClass _pitchClass = Utils::PitchClass::NONE) :
    magnitude(_magnitude), pitchClass(_pitchClass), isEnabled(true) { }
    
    float magnitude = 0.0f;
    Utils::PitchClass pitchClass = Utils::PitchClass::NONE;
    bool isEnabled = true;
  } WaveBar;
  
  void updateWaveBars();
  void addBarsForNote(ParamNote* note, bool showCandidates);
  
  // Bookkeeping
  Parameters& mParameters;
  ParamCommon* mCurSelectedParams;
  std::atomic<bool> mParamHasChanged;
  juce::Colour mParamColour;
  
  juce::AudioBuffer<float> mBuffer;
  std::array<WaveBar, NUM_WAVE_BARS> mWaveBars;
  juce::Range<int> mZoomRange; // Zoom range in samples
  int mSamplesPerBar;

  // Components

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformPanel)
};
