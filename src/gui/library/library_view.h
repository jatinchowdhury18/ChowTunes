#pragma once

#include "list_selector.h"
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

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
namespace state
{
    struct State;
}
} // namespace chow_tunes

namespace chow_tunes::gui
{
inline auto get_album_songs (const library::Album& album, const library::Music_Library& library)
{
    chowdsp::SmallVector<const library::Song*, 20> album_songs;
    album_songs.reserve (album.song_ids.size());
    for (auto song_id : album.song_ids)
        album_songs.push_back (&library.songs[song_id]);
    std::sort (album_songs.begin(), album_songs.end(), [] (auto& song1, auto& song2)
               { return song1->track_number < song2->track_number; });
    return album_songs;
}

inline auto get_artist_songs (const library::Artist& artist, const library::Music_Library& library)
{
    size_t songs_count = 0;
    for (auto album_id : artist.album_ids)
        songs_count += library.albums[album_id].song_ids.size();

    chowdsp::SmallVector<const library::Song*, 100> artist_songs;
    artist_songs.reserve (songs_count);
    for (auto album_id : artist.album_ids)
        for (auto song_id : library.albums[album_id].song_ids)
                artist_songs.push_back (&library.songs[song_id]);
    std::sort (artist_songs.begin(), artist_songs.end(), [] (auto& song1, auto& song2)
               { return song1->track_number < song2->track_number; });
    return artist_songs;
}

struct Metadata_Editor;
struct Library_View : juce::Component
{
    explicit Library_View (play_queue::Play_Queue& play_queue, state::State& app_state);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void load_artist_list (std::span<const library::Artist> artists, const library::Music_Library& library);
    void load_album_list (std::span<const size_t> album_ids, const library::Music_Library& library);
    void load_song_list (std::span<const size_t> song_ids, const library::Music_Library& library);

    List_Selector<library::Song> song_list;
    List_Selector<library::Album> album_list;
    List_Selector<library::Artist> artist_list;
    int song_list_length_seconds {};

    play_queue::Play_Queue& play_queue;
    chowdsp::SharedAudioFileSaveLoadHelper audio_file_helper;

    chowdsp::LocalPointer<juce::Component, 8192, 32> metadata_editor;
    Metadata_Editor* metadata_editor_internal = nullptr;
};
} // namespace chow_tunes::gui
