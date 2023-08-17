#include "library_view.h"
#include "audio/audio_player.h"

namespace chow_tunes::gui
{
void Library_View::load_song_list (std::span<const size_t> song_ids)
{
    song_list.cells.clear();
    for (auto song_id : song_ids)
    {
        auto& new_cell = song_list.cells.emplace_back (std::make_unique<gui::Song_Cell>());
        new_cell->list = &song_list;
        new_cell->song = &library.songs[song_id];
        new_cell->song_selected_callback = [this] (const library::Song& selected_song)
        {
            auto [buffer, fs] = audio_file_helper->loadFile (juce::File { chowdsp::toString (selected_song.filepath) });
            if (buffer.getNumSamples() == 0)
            {
                jassertfalse;
                return;
            }

            audio::Audio_Player_Action action;
            action.action_type = audio::Audio_Player_Action_Type::Start_New_Song;
            action.audio_buffer = std::make_unique<juce::AudioBuffer<float>> (std::move (buffer));
            action.sample_rate = fs;
            audio_player.ui_to_audio_queue.enqueue (std::move (action));
        };
        song_list.addAndMakeVisible (new_cell.get());
    }
    std::sort (song_list.cells.begin(), song_list.cells.end(), [] (auto& song_cell1, auto& song_cell2)
               { return song_cell1->song->track_number < song_cell2->song->track_number; });

    song_list.resized();
}

void Library_View::load_album_list (std::span<const size_t> album_ids)
{
    album_list.cells.clear();
    for (auto album_id : album_ids)
    {
        auto& new_cell = album_list.cells.emplace_back (std::make_unique<gui::Album_Cell>());
        new_cell->list = &album_list;
        new_cell->album = &library.albums[album_id];
        new_cell->album_selected_callback = [this] (const library::Album& album_selected)
        {
            load_song_list (album_selected.song_ids);
        };

        album_list.addAndMakeVisible (new_cell.get());
    }
    // @TODO: maybe we should sort by year instead?
    std::sort (album_list.cells.begin(), album_list.cells.end(), [] (auto& album_cell1, auto& album_cell2)
               { return album_cell1->album->name < album_cell2->album->name; });

    album_list.resized();
}

void Library_View::load_artist_list (std::span<const library::Artist> artists)
{
    artist_list.cells.clear();
    for (const auto& artist : artists)
    {
        auto& new_cell = artist_list.cells.emplace_back (std::make_unique<gui::Artist_Cell>());
        new_cell->list = &artist_list;
        new_cell->artist = &artist;
        new_cell->artist_selected_callback = [this] (const library::Artist& artist_selected)
        {
            load_album_list (artist_selected.album_ids);
        };

        artist_list.addAndMakeVisible (new_cell.get());
    }
    std::sort (artist_list.cells.begin(), artist_list.cells.end(), [] (auto& artist_cell1, auto& artist_cell2)
               { return artist_cell1->artist->name < artist_cell2->artist->name; });
}

Library_View::Library_View (const library::Music_Library& lib, audio::Audio_Player& player)
    : library (lib),
      audio_player (player)
{
    addAndMakeVisible (song_list);
    addAndMakeVisible (album_list);
    addAndMakeVisible (artist_list);
    load_artist_list (library.artists);
}

void Library_View::resized()
{
    auto bounds = getLocalBounds();

    artist_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.25f)));
    album_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.25f)));
    song_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.25f)));
}
} // namespace chow_tunes::gui
