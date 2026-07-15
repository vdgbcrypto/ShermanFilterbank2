cmake_minimum_required(VERSION 3.24)
project(sfb-vst VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.5
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(juce)

juce_add_plugin(sfb-vst
  COMPANY_NAME "Hermes Education"
  PLUGIN_MANUFACTURER_CODE HeEd
  PLUGIN_CODE SFBV
  FORMATS VST3 Standalone
  PRODUCT_NAME "Sherman Filterbank"
)

juce_generate_juce_header(sfb-vst)

target_sources(sfb-vst PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginProcessor.h
  Source/PluginEditor.cpp
  Source/PluginEditor.h
)

target_compile_definitions(sfb-vst PRIVATE
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(sfb-vst PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)

target_compile_features(sfb-vst PRIVATE cxx_std_17)
