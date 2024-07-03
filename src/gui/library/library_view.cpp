#include <chowdsp_logging/chowdsp_logging.h>

#include "library_view.h"
#include "play_queue/play_queue.h"
#include "library/taglib.h"

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

    juce::PopupMenu::Item open_item;
    open_item.text = "Open in Browser";
    open_item.itemID = 103;
    open_item.action = [provider = std::forward<Songs_Provider> (songs_provider)]
    {
        auto songs = provider();
        const auto filepath = songs.front()->filepath;
        juce::File { juce::String::fromUTF8 ((const char*) filepath.data(), (int) filepath.size()) }.revealToUser();
    };
    menu.addItem (std::move (open_item));
}

void Library_View::load_song_list (std::span<const size_t> song_ids, const library::Music_Library& library)
{
    const auto num_cells = song_ids.size();

    song_list.allocator.clear_all();
    song_list.cell_entries = song_list.allocator.allocate_n<List_Selector<library::Song>::Cell_Entry> (num_cells);
    song_list_length_seconds = 0;

    for (const auto& [idx, song_id] : chowdsp::enumerate (song_ids))
    {
        const auto& song = library.songs[song_id];
        song_list_length_seconds += get_file_length (song.filepath);

        auto& new_cell_entry = song_list.cell_entries[idx];
        new_cell_entry.data = &song;

        auto* new_cell_component = song_list.allocator.allocate<Cell_Component<library::Song>>();
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
        song_list.add_cell (new_cell_entry, new_cell_component);
    }
    std::sort (song_list.cell_entries.begin(), song_list.cell_entries.end(), [] (auto& song_cell1, auto& song_cell2)
               { return song_cell1.data->track_number < song_cell2.data->track_number; });
    song_list.update_size();
    repaint();
}

void Library_View::load_album_list (std::span<const size_t> album_ids, const library::Music_Library& library)
{
    const auto num_cells = album_ids.size();

    album_list.allocator.clear_all();

    album_list.cell_entries = album_list.allocator.allocate_n<List_Selector<library::Album>::Cell_Entry> (num_cells);

    for (const auto& [idx, album_id] : chowdsp::enumerate (album_ids))
    {
        auto& new_cell_entry = album_list.cell_entries[idx];
        new_cell_entry.data = &library.albums[album_id];

        auto* new_cell_component = album_list.allocator.allocate<Cell_Component<library::Album>>();
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
        album_list.add_cell (new_cell_entry, new_cell_component);
    }
    std::sort (album_list.cell_entries.begin(), album_list.cell_entries.end(), [] (auto& album_cell1, auto& album_cell2)
               { return album_cell1.data->year < album_cell2.data->year; });
    album_list.update_size();
    repaint();
}

void Library_View::load_artist_list (std::span<const library::Artist> artists, const library::Music_Library& library)
{
    const auto num_cells = artists.size();
    artist_list.allocator.clear_all();

    artist_list.cell_entries = artist_list.allocator.allocate_n<List_Selector<library::Artist>::Cell_Entry> (num_cells);

    for (const auto& [idx, artist] : chowdsp::enumerate (artists))
    {
        auto& new_cell_entry = artist_list.cell_entries[idx];
        new_cell_entry.data = &artist;

        auto* new_cell_component = artist_list.allocator.allocate<Cell_Component<library::Artist>>();
        new_cell_component->label_text = new_cell_entry.data->name;
        new_cell_component->cell_clicked = [this, &library] (const library::Artist& artist_selected)
        {
            load_song_list ({}, library);
            load_album_list (artist_selected.album_ids, library);
        };
        artist_list.add_cell (new_cell_entry, new_cell_component);
    }
    std::sort (artist_list.cell_entries.begin(),
               artist_list.cell_entries.end(),
               [] (auto& artist_cell1, auto& artist_cell2)
               {
                   for (const auto& [c1, c2] : chowdsp::zip (artist_cell1.data->name, artist_cell2.data->name))
                   {
                       if (c1 != c2)
                           return std::tolower (c1) < std::tolower (c2);
                   }
                   return artist_cell1.data->name < artist_cell2.data->name;
               });
    artist_list.update_size();
    repaint();
}

Library_View::Library_View (play_queue::Play_Queue& _play_queue)
    : play_queue (_play_queue)
{
    addAndMakeVisible (song_list);
    addAndMakeVisible (album_list);
    addAndMakeVisible (artist_list);
}

constexpr int footer_height = 30;
void Library_View::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey.brighter (0.1f));
    auto bounds = getLocalBounds();
    auto label_bounds = bounds.removeFromBottom (footer_height);

    const auto artist_label_bounds = label_bounds.removeFromLeft (proportionOfWidth (0.33f));
    const auto artist_view_label = chowdsp::format (artist_list.allocator.allocator, "{} Artists", artist_list.cell_entries.size());
    g.drawFittedText (chowdsp::toString (artist_view_label), artist_label_bounds, juce::Justification::left, 1);

    const auto album_label_bounds = label_bounds.removeFromLeft (proportionOfWidth (0.33f));
    const auto album_view_label = chowdsp::format (album_list.allocator.allocator, "{} Albums", album_list.cell_entries.size());
    g.drawFittedText (chowdsp::toString (album_view_label), album_label_bounds, juce::Justification::left, 1);

    const auto song_label_bounds = label_bounds.removeFromLeft (proportionOfWidth (0.33f));
    const auto song_view_label = chowdsp::format (song_list.allocator.allocator,
                                                  "{} Songs || {:02d}:{:02d}",
                                                  song_list.cell_entries.size(),
                                                  song_list_length_seconds / 60,
                                                  song_list_length_seconds % 60);
    g.drawFittedText (chowdsp::toString (song_view_label), song_label_bounds, juce::Justification::left, 1);
}

void Library_View::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromBottom (footer_height);

    artist_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
    album_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
    song_list.setBounds (bounds.removeFromLeft (proportionOfWidth (0.33f)));
}
} // namespace chow_tunes::gui
