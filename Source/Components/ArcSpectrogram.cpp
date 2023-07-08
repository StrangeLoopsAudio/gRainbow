/*
  ==============================================================================

    ArcSpectrogram.cpp
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#include "ArcSpectrogram.h"

#include <limits.h>

#include "Settings.h"
#include "Utils/Colour.h"

//==============================================================================
ArcSpectrogram::ArcSpectrogram(Parameters& parameters)
    : juce::Thread("spectrogram thread"), mParameters(parameters) {
  setFramesPerSecond(REFRESH_RATE_FPS);
  mBuffers.fill(nullptr);

  // check if params has images, which would mean the plugin was reopened
  if (!mParameters.ui.specComplete) {
    // if not complete, we assume all images will be remade, no "half way"
    // support currently
    for (int i = 0; i < (int)ParamUI::SpecType::COUNT; i++) {
      mImagesComplete[i] = false;
    }
  }

  // ComboBox for some reason is not zero indexed like the rest of JUCE and C++
  // for adding items we go by 'id' base but everything else is 'index' based
  mSpecType.addItem(Utils::SpecTypeNames[ParamUI::SpecType::SPECTROGRAM], (int)ParamUI::SpecType::SPECTROGRAM + 1);
  mSpecType.addItem(Utils::SpecTypeNames[ParamUI::SpecType::HPCP], (int)ParamUI::SpecType::HPCP + 1);
  mSpecType.addItem(Utils::SpecTypeNames[ParamUI::SpecType::DETECTED], (int)ParamUI::SpecType::DETECTED + 1);
  mSpecType.addItem(Utils::SpecTypeNames[ParamUI::SpecType::WAVEFORM], (int)ParamUI::SpecType::WAVEFORM + 1);
  mSpecType.setTooltip("Select different spectrum type");
  mSpecType.onChange = [this](void) {
    // Will get called from user using UI ComboBox and from inside this class
    // when loading buffers
    mParameters.ui.specType = (ParamUI::SpecType)mSpecType.getSelectedItemIndex();
    repaint();
  };
  mSpecType.setTooltip("Change Spectrogram type to view");
  mSpecType.setVisible(true);

  mActivePitchClass.reset(false);

  /*mParameters.note.onGrainCreated = [this](Utils::PitchClass pitchClass, int genIdx, float durationSec, float envGain) {
    // always get the callback, but ignore it if note was released or over grain max
    if (!mActivePitchClass[pitchClass] || mArcGrains.size() >= MAX_NUM_GRAINS) {
      return;
    }
    ParamGenerator* gen = mParameters.note.notes[pitchClass]->generators[genIdx].get();
    float envIncSamples = Utils::ENV_LUT_SIZE / (durationSec * REFRESH_RATE_FPS);
    mArcGrains.add(ArcGrain(gen, envGain, envIncSamples, pitchClass));
  }; */

  addChildComponent(mSpecType);
}

ArcSpectrogram::~ArcSpectrogram() {
  mParameters.note.onGrainCreated = nullptr;
  stopThread(4000);
}

void ArcSpectrogram::paint(juce::Graphics& g) {
  // Set gradient
  g.setFillType(Utils::getBgGradient(getBounds(), mParameters.ui.loadingProgress));
  g.fillAll();

  // If nothing has been loaded skip image, progress bar will fill in void space
  if (mParameters.ui.specType != ParamUI::SpecType::INVALID) {
    int imageIndex = mSpecType.getSelectedItemIndex();
    // When loading up a plugin a second time, need to set the ComboBox state,
    // but can't in the constructor so there is the first spot we can enforce
    // it. Without this, the logo will appear when reopening the plugin
    if (imageIndex == -1) {
      mSpecType.setSelectedItemIndex(mParameters.ui.specType, juce::dontSendNotification);
      imageIndex = (int)mParameters.ui.specType;
    }
    g.drawImage(mParameters.ui.specImages[imageIndex], getLocalBounds().toFloat(),
                juce::RectanglePlacement(juce::RectanglePlacement::fillDestination), false);
  }

  // Note and Candidate can be null while loading new values
  if (!mParameters.ui.specComplete) return;

  // Draw position lines from active note
  ParamNote* note = nullptr;
  int genIdx = -1; // Currently selected generator. If >= 0, darken generator's line
  switch (mParameters.selectedParams->type) {
    case ParamType::NOTE: note = dynamic_cast<ParamNote*>(mParameters.selectedParams); break;
    case ParamType::GENERATOR: {
      auto gen = dynamic_cast<ParamGenerator*>(mParameters.selectedParams);
      note = mParameters.note.notes[gen->noteIdx].get();
      genIdx = gen->genIdx;
    }
    case ParamType::GLOBAL: break; // Do nothing, leave note as nullptr
    default: break; // do nothing
  }
  if (note != nullptr) {
    float startRadians = (1.5f * juce::MathConstants<float>::pi);
    juce::Colour noteColour = mParameters.getSelectedParamColour();
    std::vector<ParamCandidate*> usedCandidates;
    // Draw generator position lines
    for (int i = 0; i < NUM_GENERATORS; ++i) {
      if (note->shouldPlayGenerator(i)) {
        // Draw position line where the gen's candidate is
        ParamCandidate* candidate = note->getCandidate(i);
        usedCandidates.push_back(candidate);
        float endRadians = startRadians + candidate->posRatio * juce::MathConstants<float>::pi;
        g.setColour(genIdx == i ? noteColour.darker() : noteColour);
        g.drawLine(juce::Line<float>(mCenterPoint.getPointOnCircumference(mStartRadius, mStartRadius, endRadians), mCenterPoint.getPointOnCircumference(mEndRadius, mEndRadius, endRadians)), genIdx == i ? 3.0f : 2.0f);
      }
    }
    // Draw numbered candidate bubbles
    for (int i = 0; i < note->candidates.size(); ++i) {
      ParamCandidate& candidate = note->candidates[i];
      float endRadians = startRadians + candidate.posRatio * juce::MathConstants<float>::pi;
      juce::Rectangle<int> bubbleRect = juce::Rectangle<int>(CANDIDATE_BUBBLE_SIZE, CANDIDATE_BUBBLE_SIZE).withCentre(mCenterPoint.getPointOnCircumference(mEndRadius + CANDIDATE_BUBBLE_SIZE / 2, mEndRadius + CANDIDATE_BUBBLE_SIZE / 2, endRadians).toInt());
      if (std::find(usedCandidates.begin(), usedCandidates.end(), &candidate) != usedCandidates.end()) {
        g.setColour(noteColour);
        g.fillEllipse(bubbleRect.toFloat());
        g.setColour(juce::Colours::white);
        g.drawFittedText(juce::String(i + 1), bubbleRect, juce::Justification::centred, 1);
      } else {
        g.setColour(noteColour);
        g.drawFittedText(juce::String(i + 1), bubbleRect, juce::Justification::centred, 1);
      }
      g.setColour(noteColour);
      g.drawEllipse(bubbleRect.toFloat(), 2.0f);
    }
  }

  // Draw active grains
  /* if (PowerUserSettings::get().getAnimated()) {
    for (ArcGrain& grain : mArcGrains) {
      const int noteIdx = grain.paramGenerator->noteIdx;
      const int genIdx = grain.paramGenerator->genIdx;
      const ParamCandidate* candidate = mParamsNote.notes[noteIdx]->getCandidate(genIdx);
      // candidates are updated when a new sampler is created, there is a chance we are still playing the old sample in which case
      // just end drawing
      if (candidate == nullptr) {
        return;
      }

      float xRatio = candidate->posRatio + (candidate->duration * P_FLOAT(grain.paramGenerator->common[ParamCommon::Type::POS_ADJUST])->get());
      float grainProg = (grain.numFramesActive * grain.envIncSamples) / Utils::ENV_LUT_SIZE;
      xRatio += (candidate->duration / candidate->pbRate) * grainProg;
      float pitchClass = noteIdx - (std::log(candidate->pbRate) / std::log(Utils::TIMESTRETCH_RATIO));
      float yRatio = (pitchClass + 0.25f + (P_FLOAT(grain.paramGenerator->common[ParamCommon::Type::PITCH_ADJUST])->get() * 6.0f)) /
                     (float)Utils::PitchClass::COUNT;
      int grainRad = mStartRadius + (yRatio * mBowWidth);
      juce::Point<float> grainPoint = mCenterPoint.getPointOnCircumference(
          grainRad, (1.5f * juce::MathConstants<float>::pi) + (xRatio * juce::MathConstants<float>::pi));
      float envIdx = juce::jmin(Utils::ENV_LUT_SIZE - 1.0f, grain.numFramesActive * grain.envIncSamples);
      float grainSize = grain.gain * Utils::getGrainEnvelopeLUT(grain.paramGenerator->grainEnvLUT[envIdx] * MAX_GRAIN_SIZE;

      juce::Rectangle<float> grainRect = juce::Rectangle<float>(grainSize, grainSize).withCentre(grainPoint);
      juce::Colour pitchColour = Utils::getRainbow12Colour(grain.pitchClass);
      g.setColour(pitchColour);
      g.drawEllipse(grainRect, 2.0f);

      grain.numFramesActive++;
    }
  } else {
    // still increment frames if not animating
    for (ArcGrain& grain : mArcGrains) {
      grain.numFramesActive++;
    }
  }

  // Remove arc grains that are completed
  mArcGrains.removeIf([](ArcGrain& grain) { return (grain.numFramesActive * grain.envIncSamples) > Utils::ENV_LUT_SIZE; });
   */
}

void ArcSpectrogram::resized() {
  auto r = getLocalBounds();
  // Spec type combobox
  mSpecType.setBounds(r.removeFromRight(SPEC_TYPE_WIDTH).removeFromTop(SPEC_TYPE_HEIGHT));

  mCenterPoint = juce::Point<float>(getWidth() / 2.0f, getHeight());
  mStartRadius = getHeight() / 2.6f;
  mEndRadius = getHeight() - 20;
  mBowWidth = mEndRadius - mStartRadius;
}

void ArcSpectrogram::run() {
  // Initialize rainbow parameters
  juce::Point<int> startPoint = juce::Point<int>(getWidth() / 2, getHeight());
  mParameters.ui.specImages[mParameters.ui.specType] = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
  juce::Graphics g(mParameters.ui.specImages[mParameters.ui.specType]);

  // Audio waveform (1D) is handled a bit differently than its 2D spectrograms
  if (mParameters.ui.specType == ParamUI::SpecType::WAVEFORM) {
    juce::AudioBuffer<float>* audioBuffer = (juce::AudioBuffer<float>*)mBuffers[mParameters.ui.specType];
    const float* bufferSamples = audioBuffer->getReadPointer(0);
    float maxMagnitude = audioBuffer->getMagnitude(0, audioBuffer->getNumSamples());

    // Draw NUM_COLS worth of audio samples
    juce::Point<float> prevPoint = startPoint.getPointOnCircumference(mStartRadius + mBowWidth / 2, mStartRadius + mBowWidth / 2,
                                                                      -(juce::MathConstants<float>::pi / 2.0f));
    juce::Colour prevColour = juce::Colours::black;
    for (auto i = 0; i < NUM_COLS; ++i) {
      if (threadShouldExit()) return;
      int sampleIdx = ((float)i / NUM_COLS) * audioBuffer->getNumSamples();
      float sampleRadius =
          juce::jmap(bufferSamples[sampleIdx], -maxMagnitude, maxMagnitude, (float)mStartRadius, (float)mEndRadius);

      // Choose rainbow color depending on radius
      auto rainbowColour =
          juce::Colour::fromHSV(juce::jmap(bufferSamples[sampleIdx], -maxMagnitude, maxMagnitude, 0.0f, 1.0f), 1.0, 1.0f, 1.0f);

      // Draw a line connecting to the previous point, blending colours between them
      float xPerc = ((float)i / NUM_COLS);
      float angleRad = (juce::MathConstants<float>::pi * xPerc) - (juce::MathConstants<float>::pi / 2.0f);
      juce::Point<float> p = startPoint.getPointOnCircumference(sampleRadius, sampleRadius, angleRad);
      juce::ColourGradient gradient = juce::ColourGradient(prevColour, prevPoint, rainbowColour, p, false);
      g.setGradientFill(gradient);
      g.drawLine(juce::Line<float>(prevPoint, p), 2.0f);
      prevPoint = p;
      prevColour = rainbowColour;
    }
  } else {
    // All other types of spectrograms
    Utils::SpecBuffer& spec = *(Utils::SpecBuffer*)mBuffers[mParameters.ui.specType];  // cast to SpecBuffer
    if (!spec.empty() || threadShouldExit()) return;

    const float maxRow =
        static_cast<float>((mParameters.ui.specType == ParamUI::SpecType::SPECTROGRAM) ? spec[0].size() / 8 : spec[0].size());

    // Draw each column of frequencies
    for (size_t i = 0; i < NUM_COLS; ++i) {
      if (threadShouldExit()) return;
      const float specCol = ((float)i / NUM_COLS) * spec.size();
      // Draw each row of frequencies
      for (auto curRadius = mStartRadius; curRadius < mEndRadius; curRadius += 1) {
        const float radPerc = (curRadius - mStartRadius) / (float)mBowWidth;
        const float specRow = radPerc * maxRow;

        // Choose rainbow color depending on radius
        const size_t colIndex = static_cast<size_t>(specCol);
        const size_t rowIndex = static_cast<size_t>(specRow);
        const float level = juce::jlimit(0.0f, 1.0f, spec[colIndex][rowIndex] * spec[colIndex][rowIndex] * COLOUR_MULTIPLIER);
        auto rainbowColour = juce::Colour::fromHSV(radPerc, 1.0, 1.0f, level);
        g.setColour(rainbowColour);

        float xPerc = specCol / static_cast<float>(spec.size());
        float angleRad = (juce::MathConstants<float>::pi * xPerc) - (juce::MathConstants<float>::pi / 2.0f);

        // Create and rotate a rectangle to represent the "pixel"
        juce::Point<float> p = startPoint.getPointOnCircumference(curRadius, curRadius, angleRad);
        juce::AffineTransform rotation = juce::AffineTransform();
        rotation = rotation.rotated(angleRad, p.x, p.y);
        juce::Rectangle<float> rect = juce::Rectangle<float>(2, 2);
        rect = rect.withCentre(p);
        rect = rect.transformedBy(rotation);
        juce::Path rectPath;
        rectPath.addRectangle(rect);

        // Finally, draw the rectangle
        g.fillPath(rectPath, rotation);
      }
    }
  }

  // pass type as another thread can change member variable right after run() is
  // done
  onImageComplete(mParameters.ui.specType);
  mIsProcessing = false;
}

void ArcSpectrogram::onImageComplete(ParamUI::SpecType specType) {
  mImagesComplete[specType] = true;
  for (int i = 0; i < (int)ParamUI::SpecType::COUNT; i++) {
    if (!mImagesComplete[i]) {
      return;
    }
  }
  mParameters.ui.specComplete = true;
  // Lets UI know it so it can enable other UI components
  onImagesComplete();
}

void ArcSpectrogram::reset() {
  // Reset all images
  for (size_t i = 0; i < mParameters.ui.specImages.size(); i++) {
    mParameters.ui.specImages[i].clear(mParameters.ui.specImages[i].getBounds());
  }
  for (int i = 0; i < (int)ParamUI::SpecType::COUNT; i++) {
    mImagesComplete[i] = false;
  }
  mParameters.ui.specComplete = false;
  // might be lingering grains
  mArcGrains.clear();
}

void ArcSpectrogram::loadSpecBuffer(Utils::SpecBuffer* buffer, ParamUI::SpecType type) {
  if (buffer == nullptr) return;
  waitForThreadToExit(BUFFER_PROCESS_TIMEOUT);
  if (mImagesComplete[type]) return;

  mParameters.ui.specType = type;
  mBuffers[mParameters.ui.specType] = buffer;

  // As each buffer is loaded, want to display it being generated
  // Will be loaded in what ever order loaded from async callbacks
  // The last item loaded will be the first item selected in ComboBox
  if ((int)mParameters.ui.specType < mSpecType.getNumItems()) {
    mSpecType.setSelectedItemIndex(mParameters.ui.specType, juce::sendNotification);
  }
  // Only make image if component size has been set
  if (getWidth() > 0 && getHeight() > 0) {
    mIsProcessing = true;
    startThread();
  }
}

void ArcSpectrogram::loadWaveformBuffer(juce::AudioBuffer<float>* audioBuffer) {
  if (audioBuffer == nullptr) return;
  waitForThreadToExit(BUFFER_PROCESS_TIMEOUT);
  if (mImagesComplete[ParamUI::SpecType::WAVEFORM]) return;

  mParameters.ui.specType = ParamUI::SpecType::WAVEFORM;
  mBuffers[mParameters.ui.specType] = audioBuffer;

  // Only make image if component size has been set
  if (getWidth() > 0 && getHeight() > 0) {
    mIsProcessing = true;
    startThread();
  }
}

// loadSpecBuffer is never called when a preset is loaded
void ArcSpectrogram::loadPreset() {
  // make visible if preset was loaded first
  mParameters.ui.specComplete = true;
  mSpecType.setSelectedItemIndex(mParameters.ui.specType, juce::dontSendNotification);
  repaint();
}

void ArcSpectrogram::setMidiNotes(const juce::Array<Utils::MidiNote>& midiNotes) {
  mActivePitchClass.reset();
  for (const Utils::MidiNote note : midiNotes) {
    mActivePitchClass.set(note.pitch, true);
  }
}
