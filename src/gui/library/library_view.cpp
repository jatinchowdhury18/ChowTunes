#include "library_view.h"
#include "play_queue/play_queue.h"

namespace chow_tunes::gui
{
using Queue_Action = play_queue::Play_Queue::Add_To_Queue_Action;

template <typename Songs_Provider>
static void setup_play_menu (juce::PopupMenu& menu,
                             play_queue::Play_Queue& play_queue,
                             Songs_Provider&& songs_provider)
{
    juce::PopupMenu::Item play_now_item;
    play_now_item.text = "Play Now";
    play_now_item.itemID = 100;
    play_now_item.action = [&play_queue, provider = std::forward<Songs_Provider> (songs_provider)]
    {
        auto songs = provider();
        play_queue.add_to_queue (songs, Queue_Action::Play_Now);
    };
    menu.addItem (std::move (play_now_item));

    juce::PopupMenu::Item insert_next_item;
    insert_next_item.text = "Insert Next";
    insert_next_item.itemID = 101;
    insert_next_item.action = [&play_queue, provider = std::forward<Songs_Provider> (songs_provider)]
    {
        auto songs = provider();
        play_queue.add_to_queue (songs, Queue_Action::Insert_Next);
    };
    menu.addItem (std::move (insert_next_item));

    juce::PopupMenu::Item insert_last_item;
    insert_last_item.text = "Insert Last";
    insert_last_item.itemID = 102;
    insert_last_item.action = [&play_queue, provider = std::forward<Songs_Provider> (songs_provider)]
    {
        auto songs = provider();
        play_queue.add_to_queue (songs, Queue_Action::Insert_Last);
    };
    menu.addItem (std::move (insert_last_item));
}

static auto get_album_songs (const library::Album& album, const library::Music_Library& library)
{
    chowdsp::SmallVector<const library::Song*, 20> album_songs;
    for (auto song_id : album.song_ids)
        album_songs.push_back (&library.songs[song_id]);
    return album_songs;
}

void Library_View::load_song_list (std::span<const size_t> song_ids)
{
    song_list.cells.clear();
    for (auto song_id : song_ids)
    {
        auto& new_cell = song_list.cells.emplace_back (std::make_unique<gui::Song_Cell>());
        new_cell->data = &library.songs[song_id];
        new_cell->label_text = new_cell->data->name;
        new_cell->cell_double_clicked = [this] (const library::Song& selected_song)
        {
            std::array<const library::Song*, 1> songs { &selected_song };
            play_queue.add_to_queue (songs, Queue_Action::Play_Now);
        };
        new_cell->cell_right_clicked = [this] (const library::Song& selected_song)
        {
            juce::PopupMenu menu;
            setup_play_menu (menu, play_queue, [&selected_song]
                             { return std::array<const library::Song*, 1> { &selected_song }; });
            menu.showMenuAsync (juce::PopupMenu::Options {});
        };
        song_list.add_cell (*new_cell);
    }
    std::sort (song_list.cells.begin(), song_list.cells.end(), [] (auto& song_cell1, auto& song_cell2)
               { return song_cell1->data->track_number < song_cell2->data->track_number; });

    song_list.update_size();
}

void Library_View::load_album_list (std::span<const size_t> album_ids)
{
    album_list.cells.clear();
    for (auto album_id : album_ids)
    {
        auto& new_cell = album_list.cells.emplace_back (std::make_unique<gui::Album_Cell>());
        new_cell->data = &library.albums[album_id];
        new_cell->label_text = new_cell->data->name;
        new_cell->cell_clicked = [this] (const library::Album& album_selected)
        {
            load_song_list (album_selected.song_ids);
        };
        new_cell->cell_double_clicked = [this] (const library::Album& album_selected)
        {
            load_song_list (album_selected.song_ids);

            auto album_songs = get_album_songs (album_selected, library);
            play_queue.add_to_queue (album_songs, Queue_Action::Play_Now);
        };
        new_cell->cell_right_clicked = [this] (const library::Album& album_selected)
        {
            load_song_list (album_selected.song_ids);

            juce::PopupMenu menu;
            setup_play_menu (menu, play_queue, [this, &album_selected]
                             { return get_album_songs (album_selected, library); });
            menu.showMenuAsync (juce::PopupMenu::Options {});
        };
        album_list.add_cell (*new_cell);
    }
    // @TODO: maybe we should sort by year instead?
    std::sort (album_list.cells.begin(), album_list.cells.end(), [] (auto& album_cell1, auto& album_cell2)
               { return album_cell1->data->name < album_cell2->data->name; });

    album_list.update_size();
}

void Library_View::load_artist_list (std::span<const library::Artist> artists)
{
    artist_list.cells.clear();
    for (const auto& artist : artists)
    {
        auto& new_cell = artist_list.cells.emplace_back (std::make_unique<gui::Artist_Cell>());
        new_cell->data = &artist;
        new_cell->label_text = new_cell->data->name;
        new_cell->cell_clicked = [this] (const library::Artist& artist_selected)
        {
            load_song_list ({});
            load_album_list (artist_selected.album_ids);
        };
        artist_list.add_cell (*new_cell);
    }
    std::sort (artist_list.cells.begin(), artist_list.cells.end(), [] (auto& artist_cell1, auto& artist_cell2)
               { return artist_cell1->data->name < artist_cell2->data->name; });
    artist_list.update_size();
}

Library_View::Library_View (const library::Music_Library& lib, play_queue::Play_Queue& _play_queue)
    : library (lib),
      play_queue (_play_queue)
{
    addAndMakeVisible (song_list);
    addAndMakeVisible (album_list);
    addAndMakeVisible (artist_list);
    load_artist_list (library.artists);
}

void Library_View::resized()
{
    auto bounds = getLocalBounds();

    artist_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
    album_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
    song_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
}
} // namespace chow_tunes::gui
