/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "Preset.h"

// Used for getting memory usage
#ifdef __linux__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif
#ifdef _WINDOWS
#include <windows.h>
#include <Psapi.h>
#endif

GRainbowLogo::GRainbowLogo() { mLogoImage = juce::PNGImageFormat::loadFrom(BinaryData::logo_png, BinaryData::logo_pngSize); }

void GRainbowLogo::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
  g.drawImage(mLogoImage, getLocalBounds().toFloat(), juce::RectanglePlacement(juce::RectanglePlacement::fillDestination), false);
}

//==============================================================================
GRainbowAudioProcessorEditor::GRainbowAudioProcessorEditor(GranularSynth& synth)
    : AudioProcessorEditor(&synth),
      mSynth(synth),
      mGlobalParamBox(mSynth.getParamGlobal()),
      mNoteGrid(mSynth.getParamsNote()),
      mGeneratorsBox(mSynth.getParamsNote(), synth.getParamUI()),
      mArcSpec(synth.getParamsNote(), synth.getParamUI()),
      mKeyboard(synth.getKeyboardState()),
      mProgressBar(synth.getLoadingProgress()),
      mParamUI(synth.getParamUI()),
      mTrimSelection(mFormatManager, synth.getParamUI()),
      mFormatReader(nullptr) {
  setLookAndFeel(&mRainbowLookAndFeel);
  mErrorMessage.clear();

  // Open file button
  juce::Image openFileNormal = juce::PNGImageFormat::loadFrom(BinaryData::openFileNormal_png, BinaryData::openFileNormal_pngSize);
  juce::Image openFileOver = juce::PNGImageFormat::loadFrom(BinaryData::openFileOver_png, BinaryData::openFileOver_pngSize);
  mBtnOpenFile.setImages(false, true, true, openFileNormal, 1.0f, juce::Colours::transparentBlack, openFileOver, 1.0f,
                         juce::Colours::transparentBlack, openFileOver, 1.0f, juce::Colours::transparentBlack);
  mBtnOpenFile.onClick = [this] { openNewFile(); };
  mBtnOpenFile.setTooltip("Load new sample from file or preset");
  addAndMakeVisible(mBtnOpenFile);

  // Recording button
  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
  juce::Image recordOver = juce::PNGImageFormat::loadFrom(BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f, juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f, juce::Colours::transparentBlack);
  mBtnRecord.onClick = [this] {
    if (mRecorder.isRecording()) {
      stopRecording();
    } else {
      startRecording();
    }
  };
  mBtnRecord.setTooltip("Record to add new sample");
  addAndMakeVisible(mBtnRecord);

  // Preset button
  juce::Image presetNormal = juce::PNGImageFormat::loadFrom(BinaryData::presetNormal_png, BinaryData::presetNormal_pngSize);
  juce::Image presetOver = juce::PNGImageFormat::loadFrom(BinaryData::presetOver_png, BinaryData::presetOver_pngSize);
  mBtnPreset.setImages(false, true, true, presetNormal, 1.0f, juce::Colours::transparentBlack, presetOver, 1.0f,
                       juce::Colours::transparentBlack, presetOver, 1.0f, juce::Colours::transparentBlack);
  mBtnPreset.onClick = [this] { savePreset(); };
  mBtnPreset.setTooltip("Save everything as a preset");
  addAndMakeVisible(mBtnPreset);
  // if reloading and images are done, then enable right away
  mBtnPreset.setEnabled(mParamUI.specComplete);

  // File info label
  mLabelFileName.setJustificationType(juce::Justification::centred);
  if (!mParamUI.fileName.isEmpty()) {
    // Set if saved from reopening plugin
    mLabelFileName.setText(mParamUI.fileName, juce::dontSendNotification);
  }
  addAndMakeVisible(mLabelFileName);

  mGeneratorsBox.onPositionChanged = [this](int gen, bool isRight) { mSynth.incrementPosition(gen, isRight); };
  addAndMakeVisible(mGeneratorsBox);
  addAndMakeVisible(mGlobalParamBox);
  addAndMakeVisible(mNoteGrid);
  addAndMakeVisible(mResourceUsage);

  // Arc spectrogram
  mArcSpec.onImagesComplete = [this]() {
    const juce::MessageManagerLock lock;
    jassert(mParamUI.specComplete);
    mArcSpec.setSpecType(ParamUI::SpecType::WAVEFORM);
    mBtnPreset.setEnabled(true);
  };

  mTrimSelection.onCancel = [this]() {
    // if nothing was ever loaded, got back to the logo
    updateCenterComponent((mParamUI.specComplete) ? ParamUI::CenterComponent::ARC_SPEC : ParamUI::CenterComponent::LOGO);
  };

  mTrimSelection.onProcessSelection = [this](juce::Range<double> range, bool setSelection) {
    const double sampleLength = static_cast<double>(mFormatReader->lengthInSamples);
    const double secondLength = sampleLength / mFormatReader->sampleRate;
    juce::int64 start = static_cast<juce::int64>(sampleLength * (range.getStart() / secondLength));
    juce::int64 end = static_cast<juce::int64>(sampleLength * (range.getEnd() / secondLength));
    // TODO - if small enough, it will get stuck trying to load
    if (start == end) {
      displayError("Attempted to select an empty range");
    } else {
      mSynth.processInput(juce::Range<juce::int64>(start, end), setSelection, false);
      if (setSelection) {
        // Reset any UI elements that will need to wait until processing
        mArcSpec.reset();
        mBtnPreset.setEnabled(false);
        updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
        mArcSpec.loadBuffer(&mSynth.getAudioBuffer());
      }
    }
  };

  // These share the same space, but only 1 is seen at a time
  addAndMakeVisible(mLogo);
  addChildComponent(mArcSpec);
  addChildComponent(mProgressBar);
  addChildComponent(mTrimSelection);

  addAndMakeVisible(mKeyboard);

  mAudioDeviceManager.initialise(1, 2, nullptr, true, {}, nullptr);

  mAudioDeviceManager.addAudioCallback(&mRecorder);

  mFormatManager.registerBasicFormats();

  // Only want keyboard input focus for standalone as DAW will have own input
  // mappings
  if (mSynth.wrapperType == GranularSynth::WrapperType::wrapperType_Standalone) {
    setWantsKeyboardFocus(true);
    // Standalone will persist between usages
    mSynth.resetParameters();
  }

  mTooltipWindow->setMillisecondsBeforeTipAppears(500);  // default is 700ms

  startTimer(50);

  int editorWidth = 1200;
  int editorHeight = 650;

#ifndef GRAINBOW_PRODUCTION
  addAndMakeVisible(mSettings);
  editorHeight += mSettings.getHeight();
#endif

  setSize(editorWidth, editorHeight);

  // Will update to be what it was when editor was last closed
  updateCenterComponent(mParamUI.centerComponent);
}

GRainbowAudioProcessorEditor::~GRainbowAudioProcessorEditor() {
  // can't wait for the message manager to eventually delete this
  if (mDialogWindow != nullptr) {
    mDialogWindow->exitModalState(0);
    delete mDialogWindow;
  }

  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  auto recordFile = parentDir.getChildFile(FILE_RECORDING);
  recordFile.deleteFile();
  mAudioDeviceManager.removeAudioCallback(&mRecorder);
  setLookAndFeel(nullptr);
}

void GRainbowAudioProcessorEditor::updateCenterComponent(ParamUI::CenterComponent component) {
  mParamUI.centerComponent = component;
  mLogo.setVisible(component == ParamUI::CenterComponent::LOGO);
  mArcSpec.setVisible(component == ParamUI::CenterComponent::ARC_SPEC);
  mTrimSelection.setVisible(component == ParamUI::CenterComponent::TRIM_SELECTION);
}

void GRainbowAudioProcessorEditor::timerCallback() {
  // Update progress bar when loading audio clip
  // Will overlay on the other center components
  double loadingProgress = mSynth.getLoadingProgress();
  if (loadingProgress < 1.0 && loadingProgress > 0.0) {
    mProgressBar.setVisible(true);
  } else {
    mProgressBar.setVisible(false);
  }

  // Check for buffers needing to be updated
  if (!mParamUI.specComplete) {
    std::vector<Utils::SpecBuffer*> specs = mSynth.getProcessedSpecs();
    for (int i = 0; i < specs.size(); ++i) {
      if (specs[i] != nullptr && mArcSpec.shouldLoadImage((ParamUI::SpecType)i))
        mArcSpec.loadBuffer(specs[i], (ParamUI::SpecType)i);
    }
  }

  // Get notes being played, send off to each children and then redraw.
  // Grab the notes from the Synth instead of MidiKeyboardState::Listener to not block the thread to draw.
  // There is a chance notes are pressed and released inbetween timer callback if they are super short, but can always increase the
  // callback timer
  const juce::Array<Utils::MidiNote>& midiNotes = mSynth.getMidiNotes();
  // Each component has has a different use for the midi notes, so just give them the notes and have them do what logic they want
  // with it
  mKeyboard.setMidiNotes(midiNotes);
  mArcSpec.setMidiNotes(midiNotes);
  mGeneratorsBox.setMidiNotes(midiNotes);

  if (PowerUserSettings::get().getResourceUsage()) {
    const double cpuPerc = mAudioDeviceManager.getCpuUsage() * 100;
    size_t virtual_memory = 0;
    size_t resident_memory = 0;
#if defined(__linux__)
    FILE* file = fopen("/proc/self/statm", "r");
    if (file) {
      unsigned long VmSize = 0;
      unsigned long VmRSS = 0;
      fscanf(file, "%lu %lu", &VmSize, &VmRSS);
      fclose(file);
      virtual_memory = static_cast<size_t>(VmSize) * getpagesize();
      resident_memory = static_cast<size_t>(VmRSS) * getpagesize();
    }
#elif defined(_WINDOWS)
    // According to MSDN
    PROCESS_MEMORY_COUNTERS counters;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
      virtual_memory = counters.PagefileUsage;
      resident_memory = counters.WorkingSetSize;
    }
#endif

    mResourceUsage.setText(juce::String(cpuPerc, 1) + "% CPU | Virtual " + juce::String(virtual_memory >> 20) + " MB" +
                               " | Resident " +
                               juce::String(resident_memory >> 20) + " MB",
                           juce::dontSendNotification);
  }

  repaint();
}

//==============================================================================
void GRainbowAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw background for open file button
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnOpenFile.getBounds().toFloat(), 14);

  // Draw background for record button
  g.setColour(mRecorder.isRecording() ? juce::Colours::red : juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnRecord.getBounds().toFloat(), 14);

  // Draw background for preset button
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnPreset.getBounds().toFloat(), 14);
}

/**
  @brief Draw note display (the small section between the keyboard and arc spectrogram)
*/
void GRainbowAudioProcessorEditor::paintOverChildren(juce::Graphics& g) {
  if (!PowerUserSettings::get().getAnimated()) {
    return;
  }

  // Right now just give last note played, not truely polyphony yet
  const juce::Array<Utils::MidiNote>& midiNotes = mSynth.getMidiNotes();
  if (!midiNotes.isEmpty()) {
    // If there are not candidates, will just not draw any arrows/lines
    std::vector<ParamCandidate*> candidates = mSynth.getActiveCandidates();
    if (!candidates.empty()) {
      // Draw position arrows
      juce::Colour pitchColour = Utils::getRainbow12Colour(mSynth.getLastPitchClass());
      for (int i = 0; i < candidates.size(); ++i) {
        if (candidates[i] == nullptr) continue;
        g.setColour((i == mGeneratorsBox.getSelectedGenerator()) ? pitchColour.brighter() : pitchColour.darker().darker());
        auto middlePos = candidates[i]->posRatio + (candidates[i]->duration / 2.0f);
        float angleRad = (juce::MathConstants<float>::pi * middlePos) - (juce::MathConstants<float>::pi / 2.0f);
        juce::Point<float> startPoint = juce::Point<float>(mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
        juce::Point<float> endPoint = startPoint.getPointOnCircumference(mArcSpec.getHeight() / 4.5f, angleRad);
        g.drawArrow(juce::Line<float>(startPoint, endPoint), 4.0f, 10.0f, 6.0f);
      }

      // Draw path from rainbow key to the arrow's base
      for (const Utils::MidiNote& midiNote : midiNotes) {
        const Utils::PitchClass pitchClass = midiNote.pitch;
        // if more than 1 note played, the last note will be empathized
        const bool empathized = (midiNote == midiNotes.getLast());

        float noteX = mKeyboard.getBounds().getX() + (mKeyboard.getWidth() * mKeyboard.getPitchXRatio(pitchClass));
        juce::Path displayPath;
        displayPath.startNewSubPath(noteX, mNoteDisplayRect.getBottom());
        displayPath.lineTo(noteX, mNoteDisplayRect.getBottom() - (NOTE_DISPLAY_HEIGHT / 2.0f));
        displayPath.lineTo(mNoteDisplayRect.getCentre());
        displayPath.lineTo(mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
        const float alpha = (empathized) ? 1.0f : 0.3f;
        g.setColour(Utils::getRainbow12Colour(pitchClass).withAlpha(alpha));
        g.strokePath(displayPath, juce::PathStrokeType(4.0f));
        if (empathized) {
          g.fillEllipse(mNoteDisplayRect.getCentreX() - (NOTE_BULB_SIZE / 2.0f), mNoteDisplayRect.getY() - (NOTE_BULB_SIZE / 2.0f),
                        NOTE_BULB_SIZE, NOTE_BULB_SIZE);
        }
      }
    }
  }

  // When dragging a file over, give feedback it will be accepted when released
  if (mIsFileHovering) {
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.fillRect(getLocalBounds());
  }
}

void GRainbowAudioProcessorEditor::resized() {
  auto r = getLocalBounds();

#ifndef GRAINBOW_PRODUCTION
  mSettings.setBounds(r.removeFromBottom(mSettings.getHeight()));
#endif

  // Generators box
  auto leftPanel = r.removeFromLeft(PANEL_WIDTH);
  mGeneratorsBox.setBounds(leftPanel);

  auto rightPanel = r.removeFromRight(PANEL_WIDTH);
  mGlobalParamBox.setBounds(rightPanel.removeFromTop(rightPanel.getHeight() / 2));
  const int resourceUsageHight = 15;
  mNoteGrid.setBounds(rightPanel.removeFromTop(rightPanel.getHeight() - resourceUsageHight));
  mResourceUsage.setBounds(rightPanel);

  // Open and record buttons
  auto filePanel = r.removeFromTop(BTN_PANEL_HEIGHT + BTN_PADDING);
  filePanel.removeFromTop(BTN_PADDING);
  filePanel.removeFromLeft(BTN_PADDING);
  mBtnOpenFile.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));
  filePanel.removeFromLeft(BTN_PADDING);
  mBtnRecord.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));
  // preset button
  filePanel.removeFromRight(BTN_PADDING);
  mBtnPreset.setBounds(filePanel.removeFromRight(OPEN_FILE_WIDTH));
  // no button currntly, but add padding as if one was present
  filePanel.removeFromRight(BTN_PADDING);
  filePanel.removeFromRight(OPEN_FILE_WIDTH);
  // remaining space on sides remaing is for file information
  mLabelFileName.setBounds(filePanel.removeFromTop(BTN_PANEL_HEIGHT));

  r.removeFromTop(NOTE_DISPLAY_HEIGHT);  // Just padding

  // Center middle space
  const juce::Rectangle<int> centerRect = r.removeFromTop(r.getWidth() / 2.0f);
  mArcSpec.setBounds(centerRect);
  mLogo.setBounds(centerRect);
  mTrimSelection.setBounds(centerRect);
  mProgressBar.setBounds(centerRect.withSizeKeepingCentre(PROGRESS_SIZE, PROGRESS_SIZE));

  // Space for note display
  mNoteDisplayRect = r.removeFromTop(NOTE_DISPLAY_HEIGHT).toFloat();

  // Keyboard
  juce::Rectangle<int> keyboardRect = r.removeFromTop(r.getHeight() - NOTE_DISPLAY_HEIGHT).reduced(NOTE_DISPLAY_HEIGHT, 0.0f);
  mKeyboard.setBounds(keyboardRect);
}

bool GRainbowAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files) {
  // Only accept 1 file of wav/mp3/gbow at a time
  if (files.size() == 1) {
    juce::String extension = files[0].fromLastOccurrenceOf(".", false, false);
    if (extension == "wav" || extension == "mp3" || extension == "gbow") {
      return true;
    }
  }
  return false;
}
void GRainbowAudioProcessorEditor::fileDragEnter(const juce::StringArray& files, int x, int y) {
  mIsFileHovering = true;
  repaint();
}
void GRainbowAudioProcessorEditor::fileDragExit(const juce::StringArray& files) {
  mIsFileHovering = false;
  repaint();
}
void GRainbowAudioProcessorEditor::filesDropped(const juce::StringArray& files, int x, int y) {
  jassert(files.size() == 1);
  mIsFileHovering = false;
  repaint();
  processFile(juce::File(files[0]));
}

/** Pauses audio to open file
    @param path optional path to load, otherwise will prompt user for file
   location
*/
void GRainbowAudioProcessorEditor::openNewFile(const char* path) {
  if (mIsFileChooserUsed) {
    return;
  }
  if (path == nullptr) {
    mIsFileChooserUsed = true;
    juce::FileChooser chooser("Select a file to granulize...", juce::File::getCurrentWorkingDirectory(), "*.wav;*.mp3;*.gbow",
                              true);

    if (chooser.browseForFileToOpen()) {
      mIsFileChooserUsed = false;
      auto file = chooser.getResult();
      processFile(file);
    } else {
      mIsFileChooserUsed = false;
    }
  } else {
    auto file = juce::File(juce::String(path));
    processFile(file);
  }
}

void GRainbowAudioProcessorEditor::startRecording() {
  if (!juce::RuntimePermissions::isGranted(juce::RuntimePermissions::writeExternalStorage)) {
    SafePointer<GRainbowAudioProcessorEditor> safeThis(this);

    juce::RuntimePermissions::request(juce::RuntimePermissions::writeExternalStorage, [safeThis](bool granted) mutable {
      if (granted) safeThis->startRecording();
    });
    return;
  }

  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  parentDir.getChildFile(FILE_RECORDING).deleteFile();
  mRecordedFile = parentDir.getChildFile(FILE_RECORDING);

  mRecorder.startRecording(mRecordedFile);

  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f, juce::Colours::transparentBlack, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordIcon, 1.0f, juce::Colours::transparentBlack);
  repaint();
}

void GRainbowAudioProcessorEditor::stopRecording() {
  mRecorder.stop();

  processFile(mRecordedFile);

  mRecordedFile = juce::File();

  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
  juce::Image recordOver = juce::PNGImageFormat::loadFrom(BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f, juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f, juce::Colours::transparentBlack);
  repaint();
}

void GRainbowAudioProcessorEditor::processFile(juce::File file) {
  // Show users which file is being loaded/processed
  mParamUI.fileName = file.getFileName();
  mLabelFileName.setText(mParamUI.fileName, juce::dontSendNotification);

  if (file.getFileExtension() == ".gbow") {
    processPreset(file);
    updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
  } else {
    mFormatReader = mFormatManager.createReaderFor(file);
    if (mFormatReader == nullptr) {
      displayError("Unable to read the file.");
      return;
    }

    juce::AudioBuffer<float> fileAudioBuffer;
    const int length = static_cast<int>(mFormatReader->lengthInSamples);
    fileAudioBuffer.setSize(mFormatReader->numChannels, length);
    mFormatReader->read(&fileAudioBuffer, 0, length, 0, true, true);
    mSynth.setInputBuffer(&fileAudioBuffer, mFormatReader->sampleRate);

    // TODO - It is not obvious that this function will delete the AudioFormatReader for use when called multiple times because
    // AudioThumbnail deletes it when calling setReader again
    mTrimSelection.parse(mFormatReader, file.hashCode64(), mErrorMessage);
    if (mErrorMessage.isEmpty()) {
      // display screen to trim sample
      updateCenterComponent(ParamUI::CenterComponent::TRIM_SELECTION);
    } else {
      delete (mFormatReader);
      mFormatReader = nullptr;
      displayError(mErrorMessage);
      mErrorMessage.clear();
      return;
    }
  }

}

void GRainbowAudioProcessorEditor::processPreset(juce::File file) {
  Preset::Header header;
  juce::FileInputStream input(file);
  if (input.openedOk()) {
    input.read(&header, sizeof(header));

    if (header.magic != Preset::MAGIC) {
      displayError("The file is not recognized as a valid .gbow preset file.");
      return;
    }

    juce::AudioBuffer<float> fileAudioBuffer;
    double sampleRate;

    // Currently there is only a VERSION_MAJOR of 0
    if (header.versionMajor == 0) {
      // Get Audio Buffer blob
      fileAudioBuffer.setSize(header.audioBufferChannel, header.audioBufferNumberOfSamples);
      input.read(fileAudioBuffer.getWritePointer(0), header.audioBufferSize);
      sampleRate = header.audioBufferSamplerRate;

      // Get offsets and load all png for spec images
      uint32_t maxSpecImageSize =
          juce::jmax(header.specImageSpectrogramSize, header.specImageHpcpSize, header.specImageDetectedSize);
      void* specImageData = malloc(maxSpecImageSize);
      jassert(specImageData != nullptr);
      input.read(specImageData, header.specImageSpectrogramSize);
      mParamUI.specImages[ParamUI::SpecType::SPECTROGRAM] =
          juce::PNGImageFormat::loadFrom(specImageData, header.specImageSpectrogramSize);
      input.read(specImageData, header.specImageHpcpSize);
      mParamUI.specImages[ParamUI::SpecType::HPCP] = juce::PNGImageFormat::loadFrom(specImageData, header.specImageHpcpSize);
      input.read(specImageData, header.specImageDetectedSize);
      mParamUI.specImages[ParamUI::SpecType::DETECTED] =
          juce::PNGImageFormat::loadFrom(specImageData, header.specImageDetectedSize);
      free(specImageData);

      // juce::FileInputStream uses 'int' to read
      int xmlSize = static_cast<int>(input.getTotalLength() - input.getPosition());
      void* xmlData = malloc(xmlSize);
      jassert(xmlData != nullptr);
      input.read(xmlData, xmlSize);
      mSynth.setPresetParamsXml(xmlData, xmlSize);
      free(xmlData);
    } else {
      displayError(juce::String::formatted(
          "The file is gbow version %u.%u and is not supported. This copy of gRainbow can open files up to version %u.%u",
          header.versionMajor, header.versionMinor, Preset::VERSION_MAJOR, Preset::VERSION_MINOR));
      return;
    }

    mBtnPreset.setEnabled(true);
    mArcSpec.loadPreset();
    mSynth.setInputBuffer(&fileAudioBuffer, sampleRate);
    mSynth.processInput(juce::Range<juce::int64>(), false, true);
    mArcSpec.loadBuffer(&mSynth.getAudioBuffer());
  } else {
    displayError(juce::String::formatted("The file failed to open because %s", input.getStatus().getErrorMessage().toRawUTF8()));
    return;
  }
}

void GRainbowAudioProcessorEditor::savePreset() {
  if (mIsFileChooserUsed) {
    return;
  }
  Preset::Header header;
  header.magic = Preset::MAGIC;
  header.versionMajor = Preset::VERSION_MAJOR;
  header.versionMinor = Preset::VERSION_MINOR;

  mIsFileChooserUsed = true;
  juce::FileChooser chooser("Save gRainbow presets to a file", juce::File::getCurrentWorkingDirectory(), "*.gbow", true);

  if (chooser.browseForFileToSave(true)) {
    mIsFileChooserUsed = false;
    juce::File file = chooser.getResult().withFileExtension("gbow");
    file.deleteFile();  // clear file if replacing
    juce::FileOutputStream outputStream(file);

    if (file.hasWriteAccess() && outputStream.openedOk()) {
      // Audio buffer data is grabbed from current synth
      const juce::AudioBuffer<float>& audioBuffer = mSynth.getAudioBuffer();
      header.audioBufferSamplerRate = mSynth.getSampleRate();
      header.audioBufferNumberOfSamples = audioBuffer.getNumSamples();
      header.audioBufferChannel = audioBuffer.getNumChannels();
      header.audioBufferSize = header.audioBufferNumberOfSamples * header.audioBufferChannel * sizeof(float);

      // There is no way in JUCE to be able to know the size of the
      // png/imageFormat blob until after it is written into the outstream which
      // is too late. To keep things working, just do a double copy to a
      // internal memory object so the size is know prior to writtin the image
      // data to the stream.
      juce::MemoryOutputStream spectrogramStaging;
      if (!mParamUI.saveSpecImage(spectrogramStaging, ParamUI::SpecType::SPECTROGRAM)) {
        displayError("Unable to write spectrogram image out the file");
        return;
      }
      header.specImageSpectrogramSize = spectrogramStaging.getDataSize();

      juce::MemoryOutputStream hpcpStaging;
      if (!mParamUI.saveSpecImage(hpcpStaging, ParamUI::SpecType::HPCP)) {
        displayError("Unable to write HPCP image out the file");
        return;
      }
      header.specImageHpcpSize = hpcpStaging.getDataSize();

      juce::MemoryOutputStream detectedStaging;
      header.specImageDetectedSize = detectedStaging.getDataSize();
      if (!mParamUI.saveSpecImage(detectedStaging, ParamUI::SpecType::DETECTED)) {
        displayError("Unable to write Detected image out the file");
        return;
      }

      // XML structure of preset contains all audio related information
      // These include not just AudioParams but also other params not exposes to
      // the DAW or UI directly
      juce::MemoryBlock xmlMemoryBlock;
      mSynth.getPresetParamsXml(xmlMemoryBlock);

      // Write data out section by section
      outputStream.write(&header, sizeof(header));
      outputStream.write(reinterpret_cast<const void*>(audioBuffer.getReadPointer(0)), header.audioBufferSize);
      outputStream.write(spectrogramStaging.getData(), header.specImageSpectrogramSize);
      outputStream.write(hpcpStaging.getData(), header.specImageHpcpSize);
      outputStream.write(detectedStaging.getData(), header.specImageDetectedSize);
      outputStream.write(xmlMemoryBlock.getData(), xmlMemoryBlock.getSize());
    } else {
      displayError(juce::String::formatted("Unable to open %s to write", file.getFullPathName().toRawUTF8()));
      return;
    }
  } else {
    mIsFileChooserUsed = false;
  }
}

void GRainbowAudioProcessorEditor::displayError(juce::String message) {
  juce::DialogWindow::LaunchOptions options;
  juce::Label* label = new juce::Label();
  label->setText(message, juce::dontSendNotification);
  label->setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
  options.content.setOwned(label);

  juce::Rectangle<int> area(0, 0, 400, 300);
  options.content->setSize(area.getWidth(), area.getHeight());
  options.dialogTitle = "gRainbow Error Message";
  options.dialogBackgroundColour = juce::Colour(0xff0e345a);
  options.escapeKeyTriggersCloseButton = true;
  options.useNativeTitleBar = false;
  options.resizable = true;

  mDialogWindow = options.launchAsync();
  if (mDialogWindow != nullptr) {
    mDialogWindow->centreWithSize(300, 200);
  }
}

/** Fast Debug Mode is used to speed up iterations of testing
    This method should be called only once and no-op if not being used
*/
void GRainbowAudioProcessorEditor::fastDebugMode() {
#ifdef FDB_LOAD_FILE
  // Loads a file right away - make sure macro is in quotes in Projucer
  DBG("Fast Debug Mode - Loading file " << FDB_LOAD_FILE);
  openNewFile(FDB_LOAD_FILE);
#endif  // FDB_LOAD_FILE
}