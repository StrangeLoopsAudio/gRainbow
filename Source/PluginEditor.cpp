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
#include "Utils/DSP.h"
#include "Utils/Files.h"

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
mArcSpec(synth.getParams()),
mTrimSelection(synth.getFormatManager(), synth.getParamUI()),
mProgressBar(PROGRESS_VALUE),
mTabsGrains(juce::TabbedButtonBar::Orientation::TabsAtTop),
mTabsLFOs(juce::TabbedButtonBar::Orientation::TabsAtTop),
mTabsEnvs(juce::TabbedButtonBar::Orientation::TabsAtTop),
mEnvAdsr(synth.getParams()),
mEnvGrain(synth.getParams()),
mAdjustPanel(synth.getParams()),
mModEnv1(0, synth.getParams()),
mModEnv2(1, synth.getParams()),
mModLFO1(0, synth.getParams()),
mModLFO2(1, synth.getParams()),
mModLFO3(2, synth.getParams()),
mMasterPanel(synth.getParams(), synth.getMeterSource()),
mPianoPanel(synth.getKeyboardState(), synth.getParams()) {
  setLookAndFeel(&mRainbowLookAndFeel);
  mRainbowLookAndFeel.setColour(juce::ComboBox::ColourIds::backgroundColourId, Utils::Colour::GLOBAL);
  mRainbowLookAndFeel.setColour(juce::PopupMenu::ColourIds::backgroundColourId, Utils::Colour::GLOBAL);
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
  juce::Image tabImage = juce::PNGImageFormat::loadFrom(BinaryData::grainEnv_png, BinaryData::grainEnv_pngSize);
  auto* tabImageComp = new juce::ImageComponent();
  tabImageComp->setInterceptsMouseClicks(false, false);
  tabImageComp->setImage(tabImage, juce::RectanglePlacement::onlyReduceInSize);
  mTabsGrains.addTab("grain env", Utils::Colour::BACKGROUND, &mEnvGrain, false);
  mTabsGrains.getTabbedButtonBar().getTabButton(0)->setExtraComponent(tabImageComp, juce::TabBarButton::ExtraComponentPlacement::beforeText);

  tabImage = juce::PNGImageFormat::loadFrom(BinaryData::adjust_png, BinaryData::adjust_pngSize);
  tabImageComp = new juce::ImageComponent();
  tabImageComp->setInterceptsMouseClicks(false, false);
  tabImageComp->setImage(tabImage, juce::RectanglePlacement::onlyReduceInSize);
  mTabsGrains.addTab("adjust", Utils::Colour::BACKGROUND, &mAdjustPanel, false);
  mTabsGrains.getTabbedButtonBar().getTabButton(1)->setExtraComponent(tabImageComp, juce::TabBarButton::ExtraComponentPlacement::beforeText);
  mTabsGrains.getTabbedButtonBar().setColour(juce::TabbedButtonBar::ColourIds::tabTextColourId, Utils::Colour::GLOBAL);
  mTabsGrains.getTabbedButtonBar().setColour(juce::TabbedButtonBar::ColourIds::frontTextColourId, Utils::Colour::GLOBAL);
  mTabsGrains.setTabBarDepth(Utils::TAB_HEIGHT);
  mTabsGrains.setOutline(0);
  addAndMakeVisible(mTabsGrains);

  // Mod LFO tabs
  mTabsLFOs.setTabBarDepth(Utils::TAB_HEIGHT);
  mTabsLFOs.addTab("lfo 1", Utils::Colour::BACKGROUND, &mModLFO1, false);
  mTabsLFOs.getTabbedButtonBar().getTabButton(0)->setColour(juce::TextButton::ColourIds::textColourOnId, mParameters.global.modLFOs[0].colour);
  mTabsLFOs.addTab("lfo 2", Utils::Colour::BACKGROUND, &mModLFO2, false);
  mTabsLFOs.getTabbedButtonBar().getTabButton(1)->setColour(juce::TextButton::ColourIds::textColourOnId, mParameters.global.modLFOs[1].colour);
  mTabsLFOs.addTab("lfo 3", Utils::Colour::BACKGROUND, &mModLFO3, false);
  mTabsLFOs.getTabbedButtonBar().getTabButton(2)->setColour(juce::TextButton::ColourIds::textColourOnId, mParameters.global.modLFOs[2].colour);
  mTabsLFOs.setOutline(0);
  addAndMakeVisible(mTabsLFOs);

  // Mod enveople tabs
  mTabsEnvs.setTabBarDepth(Utils::TAB_HEIGHT);
  mTabsEnvs.addTab("env amp", Utils::Colour::BACKGROUND, &mEnvAdsr, false);
  mTabsEnvs.addTab("env 2", Utils::Colour::BACKGROUND, &mModEnv1, false);
  mTabsEnvs.getTabbedButtonBar().getTabButton(1)->setColour(juce::TextButton::ColourIds::textColourOnId, mParameters.global.modEnvs[0].colour);
  mTabsEnvs.addTab("env 3", Utils::Colour::BACKGROUND, &mModEnv2, false);
  mTabsEnvs.getTabbedButtonBar().getTabButton(2)->setColour(juce::TextButton::ColourIds::textColourOnId, mParameters.global.modEnvs[1].colour);
  mTabsEnvs.setOutline(0);
  addAndMakeVisible(mTabsEnvs);

  mTrimSelection.onCancel = [this]() {
    // if nothing was ever loaded, got back to the logo
    updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
    mParameters.ui.fileName = mParameters.ui.loadedFileName;
    mTitlePresetPanel.labelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);
  };

  mTrimSelection.onProcessSelection = [this](juce::Range<double> range) {
    // If small enough, it will get stuck trying to load
    if (range.getLength() == 0.0) {
      displayError("Attempted to select an empty range");
    } else {
      mSynth.trimAndExtractPitches(range);
      // Reset any UI elements that will need to wait until processing
      mArcSpec.reset();
      mTitlePresetPanel.btnSavePreset.setEnabled(false);
      updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
      mArcSpec.loadWaveformBuffer(&mSynth.getAudioBuffer());
      mParameters.ui.loadedFileName = mParameters.ui.fileName;
    }
  };

  addAndMakeVisible(mPianoPanel);

  // These share the same space, but only 1 is seen at a time
  addChildComponent(mArcSpec);
  mProgressBar.setColour(juce::ProgressBar::ColourIds::foregroundColourId, juce::Colours::blue.withSaturation(0.55f));
  mProgressBar.setColour(juce::ProgressBar::ColourIds::backgroundColourId, juce::Colours::whitesmoke);
  addChildComponent(mProgressBar);
  addChildComponent(mTrimSelection);

  mAdjustPanel.onRefToneOn = [this](){
    mSynth.startReferenceTone(mParameters.getSelectedPitchClass());
  };
  mAdjustPanel.onRefToneOff = [this](){
    mSynth.stopReferenceTone();
  };
  addAndMakeVisible(mMasterPanel);

  mRainImage = juce::PNGImageFormat::loadFrom(BinaryData::rain_png, BinaryData::rain_pngSize);
  // Use different offsets to start to make look like different images
  mLeftRainDeltY = mRainImage.getHeight() / 2;
  mRightRainDeltY = mRainImage.getHeight() / 3;

  // Only want keyboard input focus for standalone as DAW will have own input
  // mappings
  if (mSynth.wrapperType == GranularSynth::WrapperType::wrapperType_Standalone) {
    setWantsKeyboardFocus(true);
  }

  mPianoPanel.waveform.load(mSynth.getAudioBuffer());

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
//  mAudioDeviceManager.removeAudioCallback(&mRecorder);
  setLookAndFeel(nullptr);

  mSynth.stopReferenceTone();
  mParameters.setMappingModSource(nullptr);
}

void GRainbowAudioProcessorEditor::updateCenterComponent(ParamUI::CenterComponent component) {
  mParameters.ui.centerComponent = component;
  mArcSpec.setVisible(component == ParamUI::CenterComponent::ARC_SPEC);
  mTrimSelection.setVisible(component == ParamUI::CenterComponent::TRIM_SELECTION);
}

void GRainbowAudioProcessorEditor::timerCallback() {
  // Update progress bar when loading audio clip
  // Will overlay on the other center components
  if (mParameters.ui.isLoading) {
    mProgressBar.setVisible(true);
  } else if (mProgressBar.isVisible()) {
    mPianoPanel.waveform.load(mSynth.getAudioBuffer());
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
  } else if (mParameters.ui.isLoading) {
    // Spec complete, we're done loading
    mArcSpec.setSpecType(ParamUI::SpecType::HPCP);
    mTitlePresetPanel.btnSavePreset.setEnabled(true);
    mParameters.ui.isLoading = false;
  }

  bool isNotLoaded = mParameters.ui.fileName != mParameters.ui.loadedFileName;
  if (mTitlePresetPanel.labelFileName.getText() != mParameters.ui.loadedFileName || isNotLoaded) {
    // Set title bar to display new preset name
    mTitlePresetPanel.labelFileName.setText(isNotLoaded ? mParameters.ui.fileName : mParameters.ui.loadedFileName, juce::dontSendNotification);
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
  g.setColour(Utils::Colour::BACKGROUND);
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

  // Rain
  if (mParameters.ui.centerComponent == ParamUI::CenterComponent::ARC_SPEC) {
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
      const int speed = 10;
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
  mTabsGrains.setBounds(leftPanel.removeFromTop(Utils::PANEL_HEIGHT));
  mTabsEnvs.setBounds(leftPanel.removeFromBottom(Utils::PANEL_HEIGHT));


  auto rightPanel = r.removeFromRight(Utils::PANEL_WIDTH).reduced(Utils::PADDING, Utils::PADDING);
  // TODO: add back in resource usage
  mMasterPanel.setBounds(rightPanel.removeFromTop(Utils::PANEL_HEIGHT));
  mTabsLFOs.setBounds(rightPanel.removeFromBottom(Utils::PANEL_HEIGHT));

  // Center middle space
  auto centerPanel = r.reduced(0, Utils::PADDING);
  mPianoPanel.setBounds(centerPanel.removeFromBottom(Utils::PANEL_HEIGHT));
  mArcSpec.setBounds(centerPanel.removeFromTop(Utils::PANEL_HEIGHT));
  mTrimSelection.setBounds(mArcSpec.getBounds());
  mProgressBar.setBounds(mArcSpec.getBounds().withSizeKeepingCentre(PROGRESS_SIZE, PROGRESS_SIZE));
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
    auto recentFiles = Utils::getRecentFiles();
    auto mostRecentFile = juce::File(recentFiles.getArray()->getLast().toString()).getParentDirectory();

    mFileChooser = std::make_unique<juce::FileChooser>("Select a file to granulize...", mostRecentFile,
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
    Utils::Result res = mSynth.loadPreset(file);
    if (res.success) {
      mTitlePresetPanel.btnSavePreset.setEnabled(true);
      mArcSpec.loadPreset();
      updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
      mPianoPanel.waveform.load(mSynth.getAudioBuffer());
      mTitlePresetPanel.labelFileName.setText(mParameters.ui.loadedFileName, juce::sendNotificationAsync);
      resized();
    } else {
      displayError(res.message);
    }
  } else {
    Utils::Result r = mSynth.loadAudioFile(file);
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
  auto recentFiles = Utils::getRecentFiles();
  auto mostRecentFile = juce::File(recentFiles.getArray()->getLast().toString()).getParentDirectory();
  mFileChooser = std::make_unique<juce::FileChooser>("Save gRainbow presets to a file", mostRecentFile,
                                                     "*.gbow", true);

  int saveFlags =
      juce::FileBrowserComponent::FileChooserFlags::saveMode | juce::FileBrowserComponent::FileChooserFlags::warnAboutOverwriting;

  mFileChooser->launchAsync(saveFlags, [this](const juce::FileChooser& fc) {
    juce::File file = fc.getResult().withFileExtension("gbow");
    if (file.hasWriteAccess()) {
      Utils::Result r = mSynth.savePreset(file);  // Ask synth to do the proper saving
      if (!r.success) {
        displayError(
                     juce::String::formatted("%s failed to save: %s", file.getFullPathName().toRawUTF8(), r.message.toStdString().c_str()));
      }
    } else {
      displayError(juce::String::formatted("%s does not have write access", file.getFullPathName().toRawUTF8()));
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
