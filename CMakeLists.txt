cmake_minimum_required(VERSION 3.22)

project(TheNoizzPassage VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# macOS Universal Binary (Apple Silicon arm64 + Intel x86_64)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build universal macOS binary" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

# Fetch latest JUCE automatically if not provided locally
include(FetchContent)
if(DEFINED ENV{JUCE_PATH} AND EXISTS "$ENV{JUCE_PATH}/CMakeLists.txt")
    add_subdirectory("$ENV{JUCE_PATH}" JUCE EXCLUDE_FROM_ALL)
else()
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(TheNoizzPassage
    COMPANY_NAME                "NOIZZ STUDIO"
    COMPANY_WEBSITE             "https://noizzstudio.audio"
    COMPANY_EMAIL               "support@noizzstudio.audio"
    PLUGIN_MANUFACTURER_CODE    Noiz
    PLUGIN_CODE                 NzPs
    FORMATS                     VST3 AU Standalone
    PRODUCT_NAME                "THE NOIZZ PASSAGE"
    BUNDLE_ID                   "com.noizzstudio.thenoizzpassage"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Fx" "Distortion" "Dynamics" "Spatial"
    AU_MAIN_TYPE                "kAudioUnitType_Effect"
)

target_sources(TheNoizzPassage
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/DSP/EmbeddedIRs.h
        Source/DSP/NoizzDSPChain.h
        Source/GUI/NoizzLookAndFeel.h
        Source/Presets/FactoryPresets.h
)

target_compile_definitions(TheNoizzPassage
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# High-performance CPU & SIMD compiler optimizations for ultra-low DSP CPU hit
if(MSVC)
    target_compile_options(TheNoizzPassage PRIVATE /O2 /Oi /Ot /fp:fast /arch:AVX2 /W3)
else()
    target_compile_options(TheNoizzPassage PRIVATE -O3 -ffast-math -funroll-loops -Wall -Wextra -Wno-unused-parameter)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64" AND NOT APPLE)
        target_compile_options(TheNoizzPassage PRIVATE -mavx2 -mfma)
    endif()
endif()

target_link_libraries(TheNoizzPassage
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
)
