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

void Library_View::load_song_list (std::span<const size_t> song_ids, const library::Music_Library& library)
{
    song_list.cell_entries.clear();
    song_list.cell_entries.reserve (song_ids.size());
    song_list.cell_components.reset();
    for (auto song_id : song_ids)
    {
        auto& new_cell_entry = song_list.cell_entries.emplace_back();
        new_cell_entry.data = &library.songs[song_id];

        auto [new_cell_locator, new_cell_ptr] = song_list.cell_components.emplace();
        auto* new_cell_component = new_cell_ptr->emplace();
        new_cell_component->label_text = new_cell_entry.data->name;
        new_cell_component->cell_double_clicked = [this] (const library::Song& selected_song)
        {
            std::array<const library::Song*, 1> songs { &selected_song };
            play_queue.add_to_queue (songs, Queue_Action::Play_Now);
        };
        new_cell_component->cell_right_clicked = [this] (const library::Song& selected_song)
        {
            juce::PopupMenu menu;
            setup_play_menu (menu, play_queue, [&selected_song]
                             { return std::array<const library::Song*, 1> { &selected_song }; });
            menu.showMenuAsync (juce::PopupMenu::Options {});
        };
        song_list.add_cell (new_cell_entry, new_cell_locator, new_cell_component);
    }
    std::sort (song_list.cell_entries.begin(), song_list.cell_entries.end(), [] (auto& song_cell1, auto& song_cell2)
               { return song_cell1.data->track_number < song_cell2.data->track_number; });
    song_list.update_size();
}

void Library_View::load_album_list (std::span<const size_t> album_ids, const library::Music_Library& library)
{
    album_list.cell_entries.clear();
    album_list.cell_entries.reserve (album_ids.size());
    album_list.cell_components.reset();
    for (auto album_id : album_ids)
    {
        auto& new_cell_entry = album_list.cell_entries.emplace_back();
        new_cell_entry.data = &library.albums[album_id];

        auto [new_cell_locator, new_cell_ptr] = album_list.cell_components.emplace();
        auto* new_cell_component = new_cell_ptr->emplace();
        new_cell_component->label_text = new_cell_entry.data->name;
        new_cell_component->cell_clicked = [this, &library] (const library::Album& album_selected)
        {
            load_song_list (album_selected.song_ids, library);
        };
        new_cell_component->cell_double_clicked = [this, &library] (const library::Album& album_selected)
        {
            load_song_list (album_selected.song_ids, library);

            auto album_songs = get_album_songs (album_selected, library);
            play_queue.add_to_queue (album_songs, Queue_Action::Play_Now);
        };
        new_cell_component->cell_right_clicked = [this, &library] (const library::Album& album_selected)
        {
            load_song_list (album_selected.song_ids, library);

            juce::PopupMenu menu;
            setup_play_menu (menu, play_queue, [&library, &album_selected]
                             { return get_album_songs (album_selected, library); });
            menu.showMenuAsync (juce::PopupMenu::Options {});
        };
        album_list.add_cell (new_cell_entry, new_cell_locator, new_cell_component);
    }
    std::sort (album_list.cell_entries.begin(), album_list.cell_entries.end(), [] (auto& album_cell1, auto& album_cell2)
               { return album_cell1.data->year < album_cell2.data->year; });
    album_list.update_size();
}

void Library_View::load_artist_list (std::span<const library::Artist> artists, const library::Music_Library& library)
{
    artist_list.cell_entries.clear();
    artist_list.cell_entries.reserve (artists.size());
    artist_list.cell_components.reset();
    for (const auto& artist : artists)
    {
        auto& new_cell_entry = artist_list.cell_entries.emplace_back();
        new_cell_entry.data = &artist;

        auto [new_cell_locator, new_cell_ptr] = artist_list.cell_components.emplace();
        auto* new_cell_component = new_cell_ptr->emplace();
        new_cell_component->label_text = new_cell_entry.data->name;
        new_cell_component->cell_clicked = [this, &library] (const library::Artist& artist_selected)
        {
            load_song_list ({}, library);
            load_album_list (artist_selected.album_ids, library);
        };
        artist_list.add_cell (new_cell_entry, new_cell_locator, new_cell_component);
    }
    std::sort (artist_list.cell_entries.begin(),
               artist_list.cell_entries.end(),
               [] (auto& artist_cell1, auto& artist_cell2)
               {
                   for (auto [c1, c2] : chowdsp::zip (artist_cell1.data->name, artist_cell2.data->name))
                   {
                       if (c1 != c2)
                           return std::tolower (c1) < std::tolower (c2);
                   }
                   return artist_cell1.data->name < artist_cell2.data->name;
               });
    artist_list.update_size();
}

Library_View::Library_View (const library::Music_Library& lib, play_queue::Play_Queue& _play_queue)
    : play_queue (_play_queue)
{
    addAndMakeVisible (song_list);
    addAndMakeVisible (album_list);
    addAndMakeVisible (artist_list);

    load_artist_list (lib.artists, lib);
}

void Library_View::resized()
{
    auto bounds = getLocalBounds();

    artist_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
    album_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
    song_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
}
} // namespace chow_tunes::gui
