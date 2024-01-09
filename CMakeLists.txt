cmake_minimum_required(VERSION 3.4)

# Set the plugin formats you'll be building here.
# Valid formats: AAX Unity VST AU AUv3 Standalone
set(FORMATS VST3 Standalone)

# Build LV2 only on Linux
if(UNIX AND NOT APPLE)
    list(APPEND FORMATS LV2)
endif()

# Build AU only on MacOS plus some other configurations
if(APPLE)
    list(APPEND FORMATS AU)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Support macOS down to High Sierra")
    # Building universal binaries on macOS increases build time
    # This is set on CI but not during local dev
    if ((DEFINED ENV{CI}) AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
        message("Building for Apple Silicon and x86_64")
        set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
    endif ()
    set (CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "11.0" CACHE STRING "arm 64 minimum deployment target" FORCE)
    # By default we don't want Xcode schemes to be made for modules, etc
    set(CMAKE_XCODE_GENERATE_SCHEME OFF)
endif()

# Reads in VERSION file and sticks in it CURRENT_VERSION variable
# Be sure the file has no newlines
file(STRINGS VERSION CURRENT_VERSION)

# For simplicity, the name of the project is also the name of the target
project(gRainbow VERSION ${CURRENT_VERSION})

SET(GRAINBOW_BINARY_NAME "${CMAKE_PROJECT_NAME}" CACHE STRING "Override plugin name")

# Adds all the module sources so they appear correctly in the IDE
# Must be set before JUCE is added as a sub-dir (or any targets are made)
# https://github.com/juce-framework/JUCE/commit/6b1b4cf7f6b1008db44411f2c8887d71a3348889
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

# Create a /Modules directory in the IDE with the JUCE Module code
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

# JUCE is setup as a submodule in the /JUCE folder
# Locally, you'll need to run `git submodule update --init --recursive` once
# and `git submodule update --remote --merge` to keep it up to date
# On Github Actions, it's managed by actions/checkout
add_subdirectory(external/JUCE)

juce_add_module(${CMAKE_CURRENT_SOURCE_DIR}/external/ff_meters)

# Check the readme at `docs/CMake API.md` in the JUCE repo for full config
juce_add_plugin(gRainbow
    # VERSION ...                               # Set this if the plugin version is different to the project version
    ICON_BIG Source/Resources/logo.png                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    ICON_SMALL Source/Resources/logo.png
    COMPANY_NAME StrangeLoops
    BUNDLE_ID com.StrangeLoops.gRainbow
    IS_SYNTH TRUE                       # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE               # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT TRUE              # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE    # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD TRUE # On MacOS, plugin will be copied to /Users/you/Library/Audio/Plug-Ins/
    PLUGIN_MANUFACTURER_CODE SLoo               # This has to be one uppercase, rest lower for AU formats
    PLUGIN_CODE SL01                            # A unique four-character plugin id with at least one upper-case character
    FORMATS "${FORMATS}"
    PRODUCT_NAME "${GRAINBOW_BINARY_NAME}")     # The name of the final executable, which can differ from the target name

# C++20, please
target_compile_features(gRainbow PRIVATE cxx_std_20)

# Create a header file with the version information
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Version.h
)

set(SOURCE_UTILS
    Source/Utils/Utils.h
    Source/Utils/Envelope.h
    Source/Utils/BPF.h
    Source/Utils/Timer.h
    Source/Utils/Colour.h
    Source/Utils/MidiNote.h
    Source/Utils/PitchClass.h
    Source/Utils/Presets.h
)

# Manually list all .h and .cpp files for the plugin
set(SOURCE_COMPONENTS
    Source/Components/TitlePresetPanel.h
    Source/Components/TitlePresetPanel.cpp
    Source/Components/EnvelopeADSR.h
    Source/Components/EnvelopeADSR.cpp
    Source/Components/EnvelopeGrain.h
    Source/Components/EnvelopeGrain.cpp
    Source/Components/AdjustPanel.h
    Source/Components/AdjustPanel.cpp
    Source/Components/MasterPanel.h
    Source/Components/MasterPanel.cpp
    Source/Components/FxPanel.h
    Source/Components/FxPanel.cpp
    Source/Components/ArcSpectrogram.h
    Source/Components/ArcSpectrogram.cpp
    Source/Components/TrimSelection.h
    Source/Components/TrimSelection.cpp
    Source/Components/Settings.h
    Source/Components/Settings.cpp
    Source/Components/Piano/PianoPanel.h
    Source/Components/Piano/PianoPanel.cpp
    Source/Components/Piano/RainbowKeyboard.h
    Source/Components/Piano/RainbowKeyboard.cpp
    Source/Components/Piano/WaveformPanel.h
    Source/Components/Piano/WaveformPanel.cpp
    Source/Components/Sliders.h
    Source/Components/Sliders.cpp
    Source/Components/RainbowLookAndFeel.h
    Source/Components/RainbowLookAndFeel.cpp
    Source/Components/MeterLookAndFeel.h
    Source/Components/FilterControl.h
    Source/Components/FilterControl.cpp
    Source/Components/Modulators/LFOs.h
    Source/Components/Modulators/LFOs.cpp
    Source/Components/Modulators/Envelopes.h
    Source/Components/Modulators/Envelopes.cpp
    Source/Components/Modulators/KeyboardMod.h
    Source/Components/Modulators/KeyboardMod.cpp
)

set(SOURCE_DSP
    Source/DSP/AudioRecorder.h
    Source/DSP/AudioRecorder.cpp
    Source/DSP/TransientDetector.h
    Source/DSP/TransientDetector.cpp
    Source/DSP/PitchDetector.h
    Source/DSP/PitchDetector.cpp
    Source/DSP/Fft.h
    Source/DSP/Fft.cpp
    Source/DSP/Grain.h
    Source/DSP/Grain.cpp
    Source/DSP/GranularSynth.h
    Source/DSP/GranularSynth.cpp
)

set(SOURCE_PLUGIN
    Source/Version.h
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/Parameters.h
    Source/Parameters.cpp
    Source/Preset.h
    Source/Modulators.h
    Source/Modulators.cpp
)

target_sources(gRainbow PRIVATE ${SOURCE_UTILS} ${SOURCE_COMPONENTS} ${SOURCE_DSP} ${SOURCE_PLUGIN})

# No, we don't want our source buried in extra nested folders
set_target_properties(gRainbow PROPERTIES FOLDER "")

target_include_directories(gRainbow PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/external)

# The Xcode source tree should uhhh, still look like the source tree, yo
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "" FILES
    ${SOURCE_UTILS}
    ${SOURCE_COMPONENTS}
    ${SOURCE_DSP}
    ${SOURCE_PLUGIN})

# Setup our binary data as a target
set(BINARY_RESOURCES
    Source/Resources/logo.png
    Source/Resources/company.png
    Source/Resources/ampEnv.png
    Source/Resources/grainEnv.png
    Source/Resources/adjust.png
    Source/Resources/btnPowerOff.png
    Source/Resources/btnPowerOn.png
    Source/Resources/lock.png
    Source/Resources/unlock.png
    Source/Resources/microphone.png
    Source/Resources/microphoneOver.png
    Source/Resources/openFileNormal.png
    Source/Resources/openFileOver.png
    Source/Resources/saveNormal.png
    Source/Resources/saveOver.png
    Source/Resources/cloudLeftWait.png
    Source/Resources/cloudLeftSing.png
    Source/Resources/cloudLeftTouch.png
    Source/Resources/cloudRightWait.png
    Source/Resources/cloudRightSing.png
    Source/Resources/cloudRightTouch.png
    Source/Resources/infoNormal.png
    Source/Resources/infoOver.png
    Source/Resources/rain.png
    Source/Resources/LeagueSpartan-SemiBold.ttf
)

# Set preset paths (also binary resources)
file(GLOB PRESETS "presets/*")

list(APPEND BINARY_RESOURCES ${PRESETS})

juce_add_binary_data(Assets SOURCES ${BINARY_RESOURCES})

# Required for Linux happiness:
# See https://forum.juce.com/t/loading-pytorch-model-using-binarydata/39997/2
set_target_properties(Assets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# Clean up folder organization on Xcode.
# It tucks the Plugin varieties into a "Targets" folder and generate an Xcode Scheme manually
# Xcode scheme generation is turned off globally to limit noise from other targets
# The non-hacky way of doing this is via the global PREDEFINED_TARGETS_FOLDER property
# However that doesn't seem to be working in Xcode
# Not all plugin types (au, vst) available on each build type (win, macos, linux)
foreach(target ${FORMATS} "All")
    if(TARGET ${CMAKE_PROJECT_NAME}_${target})
        set_target_properties(${CMAKE_PROJECT_NAME}_${target} PROPERTIES
            # Tuck the actual plugin targets into a folder where they won't bother us
            FOLDER "Targets"

            # MacOS only: Sets the default executable that Xcode will open on build
            # For this exact path to to work, manually build the AudioPluginHost.xcodeproj in the JUCE subdir
            # XCODE_SCHEME_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app"
            XCODE_SCHEME_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/build/gRainbow_artefacts/Debug/Standalone/gRainbow.app"

            # Let us build the target in Xcode
            XCODE_GENERATE_SCHEME ON)
    endif()
endforeach()
set_target_properties(Assets PROPERTIES FOLDER "Targets")

# We'll need to link to these from our plugin as well as our tests
set(JUCE_DEPENDENCIES
    juce::juce_audio_utils
    juce::juce_dsp
    ff_meters)

target_compile_definitions(gRainbow
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_MP3AUDIOFORMAT=1
)

target_link_libraries(gRainbow
    PRIVATE
    Assets
    ${JUCE_DEPENDENCIES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# When present, use Intel IPP for performance on Windows
if(MSVC)
    find_package(IPP)
    if(IPP_FOUND)
        target_link_libraries(gRainbow PUBLIC IPP::ipps IPP::ippcore IPP::ippi IPP::ippcv)
        message("IPP LIBRARIES FOUND")
        target_compile_definitions(gRainbow PUBLIC PAMPLEJUCE_IPP=1)
    else()
        message("IPP LIBRARIES *NOT* FOUND")
    endif()
endif()

# Use Catch2 v3 on the devel branch
Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG v3.3.2)
FetchContent_MakeAvailable(Catch2) # find_package equivalent

# Color our warnings and errors
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_compile_options (-fcolor-diagnostics)
endif ()
