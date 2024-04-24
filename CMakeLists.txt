cmake_minimum_required(VERSION 3.26)
project(ChowTunes VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

include(CPM)
CPMAddPackage(
    NAME juce
    GITHUB_REPOSITORY jatinchowdhury18/juce
    GIT_TAG global-hotkeys
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
        -Wno-unsafe-buffer-usage
)
CPMAddPackage(
    NAME samplerate
    GITHUB_REPOSITORY libsndfile/libsamplerate
    GIT_TAG 0.2.2
    OPTIONS "LIBSAMPLERATE_EXAMPLES OFF" "LIBSAMPLERATE_INSTALL OFF" "BUILD_TESTING OFF"
)

juce_add_gui_app(ChowTunes
    PRODUCT_NAME "ChowTunes"
    ICON_SMALL res/icon.png
)

include(SourceFileGroup)
setup_source_group(ChowTunes LIBRARY_AUDIO_SRCS SOURCES
    src/main.cpp
    src/main_component.cpp
    src/state.cpp

    src/library/music_library.cpp
    src/play_queue/play_queue.cpp

    src/audio/audio_player.cpp
    src/audio/audio_player_actions.cpp
    src/audio/audio_file_reader_ffmpeg.h
)

setup_source_group(ChowTunes LIBRARY_AUDIO_SRCS SOURCES
    src/gui/library/library_view.cpp
    src/gui/library/list_selector.cpp
    src/gui/search/search_view.cpp
    src/gui/transport/transport_timeline.cpp
    src/gui/transport/transport_view.cpp
    src/gui/command_line.cpp
    src/gui/hotkey_handler.cpp
    src/gui/play_queue_view.cpp
)

include(CMakeRC)
add_subdirectory(res)

find_package(FFMPEG REQUIRED)
message(STATUS "Found FFMPEG libs: ${FFMPEG_LIBRARIES}")
message(STATUS "Found FFMPEG include dirs: ${FFMPEG_INCLUDE_DIR}")

target_link_libraries(ChowTunes
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        chowdsp::chowdsp_data_structures
        chowdsp::chowdsp_logging
        chowdsp::chowdsp_buffers
        chowdsp::chowdsp_math
        chowdsp::chowdsp_dsp_utils
        chowdsp::chowdsp_sources
        chowdsp::chowdsp_plugin_utils
        chowdsp::chowdsp_plugin_state
        chowdsp::chowdsp_fuzzy_search
        chowdsp::chowdsp_gui
        tag
        samplerate
        chow_tunes::gui-resources
        ${FFMPEG_LIBRARIES}
    PUBLIC
        juce::juce_recommended_warning_flags
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
)
if(APPLE)
    target_link_libraries(ChowTunes PRIVATE "-framework Carbon")
elseif(WIN32)
    target_link_libraries(ChowTunes
        PRIVATE
            strmiids
            mfuuid
            bcrypt
            secur32
        # -lws2_32 -lole32 -luser32
    )
endif()


target_compile_definitions(ChowTunes
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_COREIMAGE_LOADER=0
        CHOWDSP_USE_LIBSAMPLERATE=1
        CHOWDSP_BUFFER_MAX_NUM_CHANNELS=2
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
        ${taglib_SOURCE_DIR}/3rdparty

        ${FFMPEG_INCLUDE_DIR}

        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# @TODO: figure out better "install" paths
if(WIN32)
    set(install_dir "C:/Users/$ENV{USERNAME}")
else()
    set(install_dir "$ENV{HOME}")
endif()

add_custom_target(ChowTunes_Install
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ChowTunes
        COMMAND ${CMAKE_COMMAND} -E echo "Copying $<TARGET_FILE:ChowTunes> to ${install_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:ChowTunes>" ${install_dir}
)
