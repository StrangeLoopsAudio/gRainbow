/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#define GRAINBOW_PRODUCTION 1

#include "PluginEditor.h"
#include "Preset.h"
#include "BinaryData.h"
#include "Utils/Colour.h"
#include "Utils/MidiNote.h"

// Used for getting memory usage
#ifdef __linux__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif
#ifdef _WINDOWS
#include <windows.h>
#include <Psapi.h>
#endif

//==============================================================================
GRainbowAudioProcessorEditor::GRainbowAudioProcessorEditor(GranularSynth& synth)
    : AudioProcessorEditor(&synth),
      mSynth(synth),
      mParameters(synth.getParams()),
      mTabsEnvelopes(juce::TabbedButtonBar::Orientation::TabsAtTop),
      mTabsFx(juce::TabbedButtonBar::Orientation::TabsAtTop),
      mTabsModulators(juce::TabbedButtonBar::Orientation::TabsAtTop),
      mEnvAdsr(synth.getParams()),
      mEnvGrain(synth.getParams()),
      mAdjustPanel(synth.getParams()),
      mFx1(synth.getParams()),
      mFx2(synth.getParams()),
      mFx3(synth.getParams()),
      mModEnvelopes(synth.getParams()),
      mModKeyboard(synth.getParams()),
      mModLFOs(synth.getParams()),
      mMasterPanel(synth.getParams(), synth.getMeterSource()),
      mFilterControl(synth.getParams()),
      mArcSpec(synth.getParams()),
      mTrimSelection(synth.getFormatManager(), synth.getParamUI()),
      mProgressBar(mParameters.ui.loadingProgress),
      mPianoPanel(synth.getKeyboardState(), synth.getParams()) {
  setLookAndFeel(&mRainbowLookAndFeel);
  mRainbowLookAndFeel.setColour(juce::PopupMenu::ColourIds::backgroundColourId, Utils::GLOBAL_COLOUR);
  juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(RainbowLookAndFeel::getCustomTypeface());

  mErrorMessage.clear();
        
  // Title and preset panel
  mTitlePresetPanel.btnOpenFile.onClick = [this] { openNewFile(); };
  mTitlePresetPanel.btnSavePreset.onClick = [this] { savePreset(); };
  addAndMakeVisible(mTitlePresetPanel);
        
  if (!mParameters.ui.fileName.isEmpty()) {
    mTitlePresetPanel.labelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);
  }
        
  // Envelope/adjust tabs
  juce::Image tabImage = juce::PNGImageFormat::loadFrom(BinaryData::ampEnv_png, BinaryData::ampEnv_pngSize);
  auto* tabImageComp = new juce::ImageComponent();
  tabImageComp->setInterceptsMouseClicks(false, false);
  tabImageComp->setImage(tabImage, juce::RectanglePlacement::onlyReduceInSize);
  mTabsEnvelopes.getTabbedButtonBar().setColour(juce::TabbedButtonBar::ColourIds::tabTextColourId, Utils::GLOBAL_COLOUR);
  mTabsEnvelopes.getTabbedButtonBar().setColour(juce::TabbedButtonBar::ColourIds::frontTextColourId, Utils::GLOBAL_COLOUR);
  mTabsEnvelopes.setTabBarDepth(Utils::TAB_HEIGHT);
  mTabsEnvelopes.addTab("amp env", Utils::BG_COLOUR, &mEnvAdsr, false);
  mTabsEnvelopes.getTabbedButtonBar().getTabButton(0)->setExtraComponent(tabImageComp, juce::TabBarButton::ExtraComponentPlacement::beforeText);

  tabImage = juce::PNGImageFormat::loadFrom(BinaryData::grainEnv_png, BinaryData::grainEnv_pngSize);
  tabImageComp = new juce::ImageComponent();
  tabImageComp->setInterceptsMouseClicks(false, false);
  tabImageComp->setImage(tabImage, juce::RectanglePlacement::onlyReduceInSize);
  mTabsEnvelopes.addTab("grain env", Utils::BG_COLOUR, &mEnvGrain, false);
  mTabsEnvelopes.getTabbedButtonBar().getTabButton(1)->setExtraComponent(tabImageComp, juce::TabBarButton::ExtraComponentPlacement::beforeText);
        
  tabImage = juce::PNGImageFormat::loadFrom(BinaryData::adjust_png, BinaryData::adjust_pngSize);
  tabImageComp = new juce::ImageComponent();
  tabImageComp->setInterceptsMouseClicks(false, false);
  tabImageComp->setImage(tabImage, juce::RectanglePlacement::onlyReduceInSize);
  mTabsEnvelopes.addTab("adjust", Utils::BG_COLOUR, &mAdjustPanel, false);
  mTabsEnvelopes.getTabbedButtonBar().getTabButton(2)->setExtraComponent(tabImageComp, juce::TabBarButton::ExtraComponentPlacement::beforeText);
          
  mTabsEnvelopes.setOutline(0);
  addAndMakeVisible(mTabsEnvelopes);
        
  // FX tabs
  mTabsFx.setTabBarDepth(Utils::TAB_HEIGHT);
  mTabsFx.addTab("FX 1", Utils::BG_COLOUR, &mFx1, false);
  mTabsFx.addTab("FX 2", Utils::BG_COLOUR, &mFx2, false);
  mTabsFx.addTab("FX 3", Utils::BG_COLOUR, &mFx3, false);
  mTabsFx.setOutline(0);
  addAndMakeVisible(mTabsFx);
        
  // Modulator tabs
  mTabsModulators.setTabBarDepth(Utils::TAB_HEIGHT);
  mTabsModulators.addTab("midi", Utils::BG_COLOUR, &mModKeyboard, false);
  mTabsModulators.addTab("envelope", Utils::BG_COLOUR, &mModEnvelopes, false);
  mTabsModulators.addTab("lfo", Utils::BG_COLOUR, &mModLFOs, false);
  mTabsModulators.setOutline(0);
  addAndMakeVisible(mTabsModulators);

  // Arc spectrogram
  mArcSpec.onImagesComplete = [this]() {
    const juce::MessageManagerLock lock;
    jassert(mParameters.ui.specComplete);
    mArcSpec.setSpecType(ParamUI::SpecType::WAVEFORM);
    mTitlePresetPanel.btnSavePreset.setEnabled(true);
  };

  mTrimSelection.onCancel = [this]() {
    // if nothing was ever loaded, got back to the logo
    updateCenterComponent((mParameters.ui.specComplete) ? ParamUI::CenterComponent::ARC_SPEC : ParamUI::CenterComponent::LOGO);
    mParameters.ui.fileName = mParameters.ui.loadedFileName;
    mTitlePresetPanel.labelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);
  };

  mTrimSelection.onProcessSelection = [this](juce::Range<double> range) {
    // Convert time to sample range
    const double sampleLength = static_cast<double>(mSynth.getInputBuffer().getNumSamples());
    const double secondLength = sampleLength / mSynth.getSampleRate();
    juce::int64 start = static_cast<juce::int64>(sampleLength * (range.getStart() / secondLength));
    juce::int64 end = static_cast<juce::int64>(sampleLength * (range.getEnd() / secondLength));
    // TODO - if small enough, it will get stuck trying to load
    if (start == end) {
      displayError("Attempted to select an empty range");
    } else {
      mParameters.ui.trimPlaybackOn = false;
      mSynth.resetParameters();
      mSynth.trimAudioBuffer(mSynth.getInputBuffer(), mSynth.getAudioBuffer(), juce::Range<juce::int64>(start, end));
      mSynth.extractSpectrograms();
      mSynth.extractPitches();
      // Reset any UI elements that will need to wait until processing
      mArcSpec.reset();
      mTitlePresetPanel.btnSavePreset.setEnabled(false);
      updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
      mArcSpec.loadWaveformBuffer(&mSynth.getAudioBuffer());
      mParameters.ui.loadedFileName = mParameters.ui.fileName;
      mParameters.ui.trimRange = range;
    }
  };

  // Let other components know when the selected note or generator has been updated
  mParameters.onSelectedChange = [this]() {
    mEnvAdsr.updateSelectedParams();
    mEnvGrain.updateSelectedParams();
    mAdjustPanel.updateSelectedParams();
    mFx1.updateSelectedParams();
    mFx2.updateSelectedParams();
    mFx3.updateSelectedParams();
    mPianoPanel.updateSelectedParams();
    mModKeyboard.updateSelectedParams();
    mModEnvelopes.updateSelectedParams();
    mModLFOs.updateSelectedParams();
    mFilterControl.updateSelectedParams();
    mMasterPanel.updateSelectedParams();
    mRainbowLookAndFeel.setColour(juce::PopupMenu::ColourIds::backgroundColourId, mParameters.getSelectedParamColour());
  };
  addAndMakeVisible(mPianoPanel);

  // These share the same space, but only 1 is seen at a time
  addChildComponent(mArcSpec);
  addChildComponent(mProgressBar);
  addChildComponent(mTrimSelection);

  addAndMakeVisible(mFilterControl);
  mAdjustPanel.onRefToneOn =[this](){
    mSynth.startReferenceTone(mParameters.getSelectedPitchClass());
  };
  mAdjustPanel.onRefToneOff = [this](){
    mSynth.stopReferenceTone();
  };
  addAndMakeVisible(mMasterPanel);

  mCloudLeftImage = juce::PNGImageFormat::loadFrom(BinaryData::cloudLeft_png, BinaryData::cloudLeft_pngSize);
  mCloudRightImage = juce::PNGImageFormat::loadFrom(BinaryData::cloudRight_png, BinaryData::cloudRight_pngSize);
  mRainImage = juce::PNGImageFormat::loadFrom(BinaryData::rain_png, BinaryData::rain_pngSize);
  // Use different offsets to start to make look like different images
  mLeftRainDeltY = mRainImage.getHeight() / 2;
  mRightRainDeltY = mRainImage.getHeight() / 3;

  // Only want keyboard input focus for standalone as DAW will have own input
  // mappings
  if (mSynth.wrapperType == GranularSynth::WrapperType::wrapperType_Standalone) {
    setWantsKeyboardFocus(true);
  }

  mTooltipWindow->setMillisecondsBeforeTipAppears(500);  // default is 700ms

  startTimer(50);

#ifndef GRAINBOW_PRODUCTION
  addAndMakeVisible(mSettings);
  Utils::EDITOR_HEIGHT += mSettings.getHeight();
#endif

  setSize(Utils::EDITOR_WIDTH, Utils::EDITOR_HEIGHT);

  // Will update to be what it was when editor was last closed
  updateCenterComponent(mParameters.ui.centerComponent);
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

  mSynth.stopReferenceTone();
}

void GRainbowAudioProcessorEditor::updateCenterComponent(ParamUI::CenterComponent component) {
  mParameters.ui.centerComponent = component;
  mArcSpec.setVisible(component == ParamUI::CenterComponent::ARC_SPEC);
  mTrimSelection.setVisible(component == ParamUI::CenterComponent::TRIM_SELECTION);
}

void GRainbowAudioProcessorEditor::timerCallback() {
  // Update progress bar when loading audio clip
  // Will overlay on the other center components
  if (mParameters.ui.loadingProgress < 1.0 && mParameters.ui.loadingProgress > 0.0) {
    mProgressBar.setVisible(true);
  } else if (mProgressBar.isVisible()) {
    if (!mPianoPanel.waveform.isLoaded()) mPianoPanel.waveform.load(mSynth.getAudioBuffer());
    mProgressBar.setVisible(false);
  }

  // Check for buffers needing to be updated
  if (!mParameters.ui.specComplete) {
    std::vector<Utils::SpecBuffer*> specs = mSynth.getProcessedSpecs();
    for (size_t i = 0; i < specs.size(); ++i) {
      if (specs[i] != nullptr && mArcSpec.shouldLoadImage((ParamUI::SpecType)i)) {
        mArcSpec.loadSpecBuffer(specs[i], (ParamUI::SpecType)i);
      }
    }
  }

  // Get notes being played, send off to each children and then redraw.
  // Grab the notes from the Synth instead of MidiKeyboardState::Listener to not block the thread to draw.
  // There is a chance notes are pressed and released inbetween timer callback if they are super short, but can always increase the
  // callback timer
  const juce::Array<Utils::MidiNote>& midiNotes = mSynth.getMidiNotes();
  // Each component has has a different use for the midi notes, so just give them the notes and have them do what logic they want
  // with it
  mPianoPanel.keyboard.setMidiNotes(midiNotes);
  mArcSpec.setMidiNotes(midiNotes);

  /* if (PowerUserSettings::get().getResourceUsage()) {
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
  }*/

  repaint();
}

//==============================================================================
void GRainbowAudioProcessorEditor::paint(juce::Graphics& g) {
  // Set gradient
  g.setColour(Utils::BG_COLOUR);
  g.fillRect(getLocalBounds());
}

/**
  @brief Draw note display (the small section between the keyboard and arc spectrogram)
*/
void GRainbowAudioProcessorEditor::paintOverChildren(juce::Graphics& g) {
  // Right now just give last note played, not truely polyphony yet
  // TODO: new note displaying

  // When dragging a file over, give feedback it will be accepted when released
  if (mIsFileHovering) {
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.fillRect(getLocalBounds());
  }

  // Clouds
  if (mParameters.ui.centerComponent == ParamUI::CenterComponent::ARC_SPEC) {
    // TODO: fix the clouds
    //g.drawImage(mCloudLeftImage, mCloudLeftTargetArea, juce::RectanglePlacement::fillDestination);
    //g.drawImage(mCloudRightImage, mCloudRightTargetArea, juce::RectanglePlacement::fillDestination);

    // Make it rain girl (while loading)
    if (mProgressBar.isVisible()) {
      g.setColour(juce::Colours::blue);
      // Exploit fact left and right are same dimension
      const int rainHeight = mLeftRain.getHeight();
      const int rainWidth = mLeftRain.getWidth();
      g.drawImage(mRainImage, mLeftRain.getX(), mLeftRain.getY(), rainWidth, rainHeight, 0, mLeftRainDeltY, rainWidth, rainHeight);
      g.drawImage(mRainImage, mRightRain.getX(), mRightRain.getY(), rainWidth, rainHeight, 0, mRightRainDeltY, rainWidth,
                  rainHeight);

      // make rain slow up as closer to full progress (which is the value 1.0)
      const int speed = 20 - static_cast<int>(18.0 * mParameters.ui.loadingProgress);
      mLeftRainDeltY -= speed;
      mRightRainDeltY -= speed;
      if (mLeftRainDeltY < rainHeight) {
        mLeftRainDeltY = mRainImage.getHeight() - rainHeight;
      }
      if (mRightRainDeltY < rainHeight) {
        mRightRainDeltY = mRainImage.getHeight() - rainHeight;
      }
    }
  }
}

void GRainbowAudioProcessorEditor::resized() {
  auto r = getLocalBounds();

#ifndef GRAINBOW_PRODUCTION
  mSettings.setBounds(r.removeFromBottom(mSettings.getHeight()));
#endif
  
  mTitlePresetPanel.setBounds(r.removeFromTop(Utils::PRESET_PANEL_HEIGHT));
      
  // Left and right panels
  auto leftPanel = r.removeFromLeft(Utils::PANEL_WIDTH).reduced(Utils::PADDING, Utils::PADDING);
  mTabsEnvelopes.setBounds(leftPanel.removeFromTop(Utils::PANEL_HEIGHT));
  mTabsFx.setBounds(leftPanel.removeFromBottom(Utils::PANEL_HEIGHT));

  auto rightPanel = r.removeFromRight(Utils::PANEL_WIDTH).reduced(Utils::PADDING, Utils::PADDING);
  // TODO: add back in resource usage
  mMasterPanel.setBounds(rightPanel.removeFromTop(Utils::PANEL_HEIGHT));
  mTabsModulators.setBounds(rightPanel.removeFromBottom(Utils::PANEL_HEIGHT));
  //mFilterControl.setBounds(rightPanel);

  // Center middle space
  auto centerPanel = r.reduced(0, Utils::PADDING);
  mPianoPanel.setBounds(centerPanel.removeFromBottom(Utils::PANEL_HEIGHT));
  mArcSpec.setBounds(centerPanel.removeFromTop(Utils::PANEL_HEIGHT));
  mTrimSelection.setBounds(mArcSpec.getBounds());
  mProgressBar.setBounds(mArcSpec.getBounds().withSizeKeepingCentre(PROGRESS_SIZE, PROGRESS_SIZE));

  // Cloud centers
  {
    const int expansion = mCloudLeftImage.getWidth() / 4.0f;
    const int translation = expansion * 2;
    const auto leftCenter = mArcSpec.getBounds().getBottomLeft().translated(translation, 0);
    const auto rightCenter = mArcSpec.getBounds().getBottomRight().translated(-translation, 0);
    mCloudLeftTargetArea = mCloudLeftImage.getBounds().expanded(expansion).withCentre(leftCenter).toFloat();
    mCloudRightTargetArea = mCloudRightImage.getBounds().expanded(expansion).withCentre(rightCenter).toFloat();

    // This was sfigured out by using drawRect() until saw the area it should be
    const float leftCloudWidth = mCloudLeftTargetArea.getWidth();
    const float leftCloudHeight = mCloudLeftTargetArea.getHeight();
    mLeftRain = mCloudLeftTargetArea.translated(leftCloudWidth / 3.8f, leftCloudHeight / 1.7f)
                    .withWidth(leftCloudWidth / 2.0f)
                    .withHeight(leftCloudHeight / 1.6f);
    const float rightCloudWidth = mCloudRightTargetArea.getWidth();
    const float rightCloudHeight = mCloudRightTargetArea.getHeight();
    mRightRain = mCloudRightTargetArea.translated(rightCloudWidth / 4.2f, rightCloudHeight / 1.7f)
                     .withWidth(rightCloudWidth / 2.0f)
                     .withHeight(rightCloudHeight / 1.6f);
  }
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
void GRainbowAudioProcessorEditor::fileDragEnter(const juce::StringArray&, int, int) {
  mIsFileHovering = true;
  repaint();
}
void GRainbowAudioProcessorEditor::fileDragExit(const juce::StringArray&) {
  mIsFileHovering = false;
  repaint();
}
void GRainbowAudioProcessorEditor::filesDropped(const juce::StringArray& files, int, int) {
  jassert(files.size() == 1);
  mIsFileHovering = false;
  repaint();
  loadFile(juce::File(files[0]));
}

/** Pauses audio to open file
    @param path optional path to load, otherwise will prompt user for file
   location
*/
void GRainbowAudioProcessorEditor::openNewFile(const char* path) {
  if (path == nullptr) {
    mFileChooser = std::make_unique<juce::FileChooser>("Select a file to granulize...", juce::File::getCurrentWorkingDirectory(),
                                                       "*.wav;*.mp3;*.gbow", true);

    int openFlags =
        juce::FileBrowserComponent::FileChooserFlags::openMode | juce::FileBrowserComponent::canSelectFiles;

  mFileChooser->launchAsync(openFlags, [this](const juce::FileChooser& fc) {
      auto file = fc.getResult();
      if (file.existsAsFile()) loadFile(file);
    });
  } else {
    auto file = juce::File(juce::String(path));
  if (file.existsAsFile()) loadFile(file);
  }
}

// TODO: re-enable me when recording is back in
//void GRainbowAudioProcessorEditor::startRecording() {
//  if (!juce::RuntimePermissions::isGranted(juce::RuntimePermissions::writeExternalStorage)) {
//    SafePointer<GRainbowAudioProcessorEditor> safeThis(this);
//
//    juce::RuntimePermissions::request(juce::RuntimePermissions::writeExternalStorage, [safeThis](bool granted) mutable {
//      if (granted) safeThis->startRecording();
//    });
//    return;
//  }
//
//  mAudioDeviceManager.initialiseWithDefaultDevices(1, 0);
//  mAudioDeviceManager.addAudioCallback(&mRecorder);
//
//  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
//  parentDir.getChildFile(FILE_RECORDING).deleteFile();
//  mRecordedFile = parentDir.getChildFile(FILE_RECORDING);
//
//  mRecorder.startRecording(mRecordedFile);
//
//  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
//  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f, juce::Colours::red, recordIcon, 1.0f, juce::Colours::red, recordIcon,
//                       1.0f, juce::Colours::red);
//  repaint();
//}
//
//void GRainbowAudioProcessorEditor::stopRecording() {
//  mRecorder.stop();
//
//  loadFile(mRecordedFile);
//
//  mAudioDeviceManager.removeAudioCallback(&mRecorder);
//  mAudioDeviceManager.closeAudioDevice();
//
//  mRecordedFile = juce::File();
//
//  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
//  juce::Image recordOver = juce::PNGImageFormat::loadFrom(BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
//  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f, juce::Colours::transparentBlack, recordOver, 1.0f,
//                       juce::Colours::transparentBlack, recordOver, 1.0f, juce::Colours::transparentBlack);
//  repaint();
//}

void GRainbowAudioProcessorEditor::loadFile(juce::File file) {

  if (file.getFileExtension() == ".gbow") {
    Utils::Result r = mSynth.loadPreset(file);
    if (r.success) {
      mTitlePresetPanel.btnSavePreset.setEnabled(true);
      mArcSpec.loadPreset();
      updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
      mPianoPanel.waveform.load(mSynth.getAudioBuffer());
      mParameters.ui.loadedFileName = file.getFullPathName();
      mParameters.ui.fileName = mParameters.ui.loadedFileName;
      mTitlePresetPanel.labelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);
      resized();
    } else {
      displayError(r.message);
    }
  } else {
    Utils::Result r = mSynth.loadAudioFile(file, true);
    if (r.success) {
      // Show users which file is being loaded/processed
      mParameters.ui.fileName = file.getFullPathName();
      mTitlePresetPanel.labelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);
      mTrimSelection.parse(mSynth.getInputBuffer(), mSynth.getSampleRate(), mErrorMessage);
      if (mErrorMessage.isEmpty()) {
        // display screen to trim sample
        updateCenterComponent(ParamUI::CenterComponent::TRIM_SELECTION);
      } else {
        displayError(mErrorMessage);
        mErrorMessage.clear();
        return;
      }
    } else {
      displayError(r.message);
    }
  }
}

void GRainbowAudioProcessorEditor::savePreset() {
  mFileChooser = std::make_unique<juce::FileChooser>("Save gRainbow presets to a file", juce::File::getCurrentWorkingDirectory(),
                                                     "*.gbow", true);

  int saveFlags =
      juce::FileBrowserComponent::FileChooserFlags::saveMode | juce::FileBrowserComponent::FileChooserFlags::warnAboutOverwriting;

  mFileChooser->launchAsync(saveFlags, [this](const juce::FileChooser& fc) {
    juce::File file = fc.getResult().withFileExtension("gbow");
    file.deleteFile();  // clear file if replacing
    juce::FileOutputStream outputStream(file);

    if (file.hasWriteAccess() && outputStream.openedOk()) {
      Preset::Header header;
      header.magic = Preset::MAGIC;
      header.versionMajor = Preset::VERSION_MAJOR;
      header.versionMinor = Preset::VERSION_MINOR;
      // Audio buffer data is grabbed from current synth
      const juce::AudioBuffer<float>& audioBuffer = mSynth.getAudioBuffer();
      header.audioBufferSamplerRate = mSynth.getSampleRate();
      header.audioBufferNumberOfSamples = audioBuffer.getNumSamples();
      header.audioBufferChannel = audioBuffer.getNumChannels();
      header.audioBufferSize = header.audioBufferNumberOfSamples * header.audioBufferChannel * sizeof(float);

      // There is no way in JUCE to be able to know the size of the
      // png/imageFormat blob until after it is written into the outstream which
      // is too late. To keep things working, just do a double copy to a
      // internal memory object so the size is know prior to writing the image
      // data to the stream.
      juce::MemoryOutputStream spectrogramStaging;
      if (!mParameters.ui.saveSpecImage(spectrogramStaging, ParamUI::SpecType::SPECTROGRAM)) {
        displayError("Unable to write spectrogram image out the file");
        return;
      }
      header.specImageSpectrogramSize = spectrogramStaging.getDataSize();

      juce::MemoryOutputStream hpcpStaging;
      if (!mParameters.ui.saveSpecImage(hpcpStaging, ParamUI::SpecType::HPCP)) {
        displayError("Unable to write HPCP image out the file");
        return;
      }
      header.specImageHpcpSize = hpcpStaging.getDataSize();

      juce::MemoryOutputStream detectedStaging;
      if (!mParameters.ui.saveSpecImage(detectedStaging, ParamUI::SpecType::DETECTED)) {
        displayError("Unable to write Detected image out the file");
        return;
      }
      header.specImageDetectedSize = detectedStaging.getDataSize();

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
  });
}

void GRainbowAudioProcessorEditor::displayError(juce::String message) {
  juce::DialogWindow::LaunchOptions options;
  juce::Label* label = new juce::Label();
  label->setText(message, juce::dontSendNotification);
  label->setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
  options.content.setOwned(label);

  juce::Rectangle<int> area(0, 0, 400, 300);
  options.content->setSize(area.getWidth(), area.getHeight());
  options.dialogTitle = "gRainbow Error";
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
