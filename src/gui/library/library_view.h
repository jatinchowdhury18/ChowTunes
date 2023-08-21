#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include "list_selector.h"

namespace chow_tunes
{
namespace library
{
    struct Music_Library;
}
namespace play_queue
{
    struct Play_Queue;
}
}

namespace chow_tunes::gui
{
struct Library_View : juce::Component
{
    Library_View (const library::Music_Library& library, play_queue::Play_Queue& play_queue);

    void resized() override;

    void load_artist_list (std::span<const library::Artist> artists, const library::Music_Library& library);
    void load_album_list (std::span<const size_t> album_ids, const library::Music_Library& library);
    void load_song_list (std::span<const size_t> song_ids, const library::Music_Library& library);

    gui::List_Selector<library::Song> song_list;
    gui::List_Selector<library::Album> album_list;
    gui::List_Selector<library::Artist> artist_list;

    play_queue::Play_Queue& play_queue;
    chowdsp::SharedAudioFileSaveLoadHelper audio_file_helper;
};
} // namespace chow_tunes::gui
