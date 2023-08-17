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
    GIT_TAG master
)
CPMAddPackage(
    NAME taglib
    GITHUB_REPOSITORY taglib/taglib
    GIT_TAG v1.13.1
    OPTIONS "BUILD_TESTING OFF"
)
target_compile_options(tag
    PRIVATE
        -Wno-suggest-override -Wno-suggest-destructor-override -Wno-non-virtual-dtor
        -Wno-delete-abstract-non-virtual-dtor -Wno-deprecated-copy-with-user-provided-dtor
        -Wno-exit-time-destructors -Wno-global-constructors
        -Wno-documentation -Wno-documentation-deprecated-sync
        -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-zero-as-null-pointer-constant
        -Wno-sign-conversion -Wno-sign-compare -Wno-implicit-int-conversion -Wno-implicit-int-float-conversion
        -Wno-nonportable-system-include-path -Wno-extra-semi-stmt -Wno-unused-member-function
        -Wno-microsoft-unqualified-friend
        -Wno-shadow -Wno-shadow-field -Wno-shadow-field-in-constructor
        -Wno-old-style-cast -Wno-atomic-implicit-seq-cst -Wno-shorten-64-to-32
        -Wno-unused-parameter -Wno-missing-variable-declarations -Wno-format-nonliteral -Wno-deprecated-dynamic-exception-spec
        -Wno-implicit-fallthrough -Wno-covered-switch-default -Wno-unreachable-code-loop-increment
)
CPMAddPackage(
    NAME samplerate
    GITHUB_REPOSITORY libsndfile/libsamplerate
    GIT_TAG 0.2.2
    OPTIONS "LIBSAMPLERATE_EXAMPLES OFF" "LIBSAMPLERATE_INSTALL OFF" "BUILD_TESTING OFF"
)

juce_add_gui_app(ChowTunes
    PRODUCT_NAME "ChowTunes"
)

target_sources(ChowTunes
    PRIVATE
        src/main.cpp
        src/main_component.cpp
        src/library/music_library.cpp
        src/audio/audio_player.cpp

        src/gui/library/library_view.cpp
        src/gui/library/list_selector.cpp
)

target_link_libraries(ChowTunes
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        chowdsp::chowdsp_data_structures
        chowdsp::chowdsp_buffers
        chowdsp::chowdsp_math
        chowdsp::chowdsp_dsp_utils
        chowdsp::chowdsp_sources
        chowdsp::chowdsp_plugin_utils
        tag
        samplerate
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
        CHOWDSP_USE_LIBSAMPLERATE=1
        TAGLIB_STATIC=1
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
)
