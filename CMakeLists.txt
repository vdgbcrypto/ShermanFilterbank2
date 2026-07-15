cmake_minimum_required(VERSION 3.15)
project(ShermanPlugin VERSION 0.1.0 LANGUAGES CXX)

set(JUCE_DIR "C:/JUCE" CACHE PATH "Path to the JUCE framework (override with -DJUCE_DIR=...)")

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(ShermanPlugin
    COMPANY_NAME "vdgbcrypto"
    PLUGIN_MANUFACTURER_CODE vdgb
    PLUGIN_CODE sf02
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    PRODUCT_NAME "Sherman Filterbank 2")

target_sources(ShermanPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h)

target_link_libraries(ShermanPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra)

juce_generate_juce_header(ShermanPlugin)

target_compile_definitions(ShermanPlugin PRIVATE JUCE_VST3_CAN_REPLACE_VST2=0)
