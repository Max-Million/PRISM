cmake_minimum_required(VERSION 3.24)

project(Morph VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    find_package(JUCE CONFIG REQUIRED)
endif()

juce_add_plugin(Morph
    COMPANY_NAME "Max Million"
    BUNDLE_ID "com.maxmillion.prism"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE MxMl
    PLUGIN_CODE Prsm
    FORMATS VST3 Standalone
    PRODUCT_NAME "PRISM"
)

juce_generate_juce_header(Morph)

target_sources(Morph
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters.h

        Source/DSP/MorphEngine.cpp
        Source/DSP/MorphEngine.h

        Source/DSP/Algorithms/DistortionAlgorithm.h
        Source/DSP/Algorithms/TubeAlgorithm.cpp
        Source/DSP/Algorithms/TubeAlgorithm.h
        Source/DSP/Algorithms/HardClipAlgorithm.cpp
        Source/DSP/Algorithms/HardClipAlgorithm.h
        Source/DSP/Algorithms/FoldbackAlgorithm.cpp
        Source/DSP/Algorithms/FoldbackAlgorithm.h
        Source/DSP/Algorithms/FuzzAlgorithm.cpp
        Source/DSP/Algorithms/FuzzAlgorithm.h
        Source/DSP/Algorithms/DriveAlgorithm.cpp
        Source/DSP/Algorithms/DriveAlgorithm.h
        Source/DSP/Algorithms/BitcrushAlgorithm.cpp
        Source/DSP/Algorithms/BitcrushAlgorithm.h
        Source/DSP/Algorithms/WavefolderAlgorithm.cpp
        Source/DSP/Algorithms/WavefolderAlgorithm.h
        Source/DSP/Algorithms/RectifierAlgorithm.cpp
        Source/DSP/Algorithms/RectifierAlgorithm.h
)

target_compile_definitions(Morph
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Morph
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)