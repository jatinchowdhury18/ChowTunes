#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "audio/audio_player.h"
#include "gui/list_selector.h"
#include "library/music_library.h"

namespace chow_tunes
{
struct Main_Component : juce::Component,
                        juce::Timer
{
    Main_Component();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    library::Music_Library library = library::index_directory ("/Users/jatin/test_music");
    gui::List_Selector<gui::Song_Cell> song_list;
    audio::Audio_Player audio_player;
    juce::AudioFormatManager audio_format_manager;
    chowdsp::SharedAudioFileSaveLoadHelper audio_file_helper;
};
} // namespace chow_tunes
