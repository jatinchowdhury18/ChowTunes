#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include "list_selector.h"

namespace chow_tunes::library
{
struct Music_Library;
}

namespace chow_tunes::audio
{
struct Audio_Player;
}

namespace chow_tunes::gui
{
struct Library_View : juce::Component
{
    Library_View (const library::Music_Library& library, audio::Audio_Player& audio_player);

    void resized() override;

    void load_artist_list (std::span<const library::Artist> artists);
    void load_album_list (std::span<const size_t> album_ids);
    void load_song_list (std::span<const size_t> song_ids);

    gui::List_Selector<gui::Song_Cell> song_list;
    gui::List_Selector<gui::Album_Cell> album_list;
    gui::List_Selector<gui::Artist_Cell> artist_list;

    const library::Music_Library& library;
    audio::Audio_Player& audio_player;
    chowdsp::SharedAudioFileSaveLoadHelper audio_file_helper;
};
} // namespace chow_tunes::gui
