cmake_minimum_required(VERSION 3.26)
project(ChowTunes VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

include(cmake/CPM.cmake)
CPMAddPackage(
    NAME juce
    GITHUB_REPOSITORY juce-framework/juce
    GIT_TAG 7.0.6
)
CPMAddPackage(
    NAME chowdsp_utils
    GITHUB_REPOSITORY Chowdhury-DSP/chowdsp_utils
    GIT_TAG f20e74286aaba57bd6cc874a398efacb8a46fd91
)
CPMAddPackage(
    NAME taglib
    GITHUB_REPOSITORY taglib/taglib
    GIT_TAG v1.13.1
    OPTIONS "BUILD_TESTING OFF"
)
target_compile_options(tag PRIVATE -Wno-delete-abstract-non-virtual-dtor)

juce_add_gui_app(ChowTunes
    PRODUCT_NAME "ChowTunes"
)

target_sources(ChowTunes
    PRIVATE
        src/main.cpp
)

target_link_libraries(ChowTunes
    PRIVATE
        juce::juce_audio_utils
        chowdsp::chowdsp_data_structures
        tag
    PUBLIC
        juce::juce_recommended_warning_flags
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
)

target_compile_definitions(ChowTunes
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_include_directories(ChowTunes
    PRIVATE
        ${taglib_SOURCE_DIR}/taglib
        ${taglib_SOURCE_DIR}/taglib/toolkit
        ${taglib_SOURCE_DIR}/taglib/ape
        ${taglib_SOURCE_DIR}/taglib/mpeg
        ${taglib_SOURCE_DIR}/taglib/mpeg/id3v1
        ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2
        ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2/frames

        ${CMAKE_CURRENT_SOURCE_DIR}/src
#        ${taglib_SOURCE_DIR}/bindings/c/

#        ${taglib_SOURCE_DIR}/taglib
#        ${taglib_SOURCE_DIR}/taglib/toolkit
)
