#include "play_queue_view.h"
#include <chowdsp_logging/chowdsp_logging.h>

namespace chow_tunes::gui
{
void Play_Queue_List::update_list (const play_queue::Play_Queue& queue)
{
    if (std::find (selected_cell_idxs.begin(), selected_cell_idxs.end(), queue.currently_playing_song_index) != selected_cell_idxs.end()
        || previous_play_queue_size != queue.queue.size())
    {
        previous_play_queue_size = queue.queue.size();
        selected_cell_idxs.clear();
    }

    elapsed_length_seconds = 0;
    queue_length_seconds = 0;
    for (auto [idx, entry] : chowdsp::enumerate (cell_entries))
    {
        auto* cell = entry.component;
        cell->is_selected = false;

        if ((int) idx == queue.currently_playing_song_index)
        {
            cell->select_cell (false);
            cell->selection_fill_colour = juce::Colours::green.withAlpha (0.5f);
        }

        if (std::find (selected_cell_idxs.begin(), selected_cell_idxs.end(), idx) != selected_cell_idxs.end())
        {
            cell->select_cell (false);
            cell->selection_fill_colour = juce::Colours::orange.withAlpha (0.5f);
        }

        queue_length_seconds += static_cast<size_t> (cell->data->track_length_seconds);
        if ((int) idx < queue.currently_playing_song_index)
            elapsed_length_seconds += static_cast<size_t> (cell->data->track_length_seconds);
    }
}

Play_Queue_View::Play_Queue_View (play_queue::Play_Queue& queue)
    : play_queue (queue)
{
    clear_queue_button.onClick = [&queue]
    {
        queue.clear_queue();
    };
    addAndMakeVisible (clear_queue_button);

    queue_list.select_on_click = false;
    addAndMakeVisible (queue_list);
    queue_change_callback = play_queue.queue_changed.connect (
        [this]
        {
            queue_list.allocator.clear_all();
            queue_list.cell_entries = queue_list.allocator.allocate_n<List_Selector<library::Song>::Cell_Entry> (play_queue.queue.size());

            for (const auto& [idx, song] : chowdsp::enumerate (play_queue.queue))
            {
                auto& new_cell_entry = queue_list.cell_entries[idx];
                new_cell_entry.data = song;

                auto* new_cell_component = queue_list.allocator.allocate<Cell_Component<library::Song>>();
                new_cell_component->label_text = new_cell_entry.data->name;
                if (play_queue.currently_playing_song_index != (int) idx)
                {
                    new_cell_component->cell_clicked = [this, i = idx, &e = new_cell_component->latest_mouse_event] (const library::Song&)
                    {
                        jassert (e.has_value());

                        auto& selected_cells = queue_list.selected_cell_idxs;
                        if (e->mods.isRightButtonDown())
                        {
                            if (selected_cells.empty())
                                selected_cells = { i };
                        }
                        else if (auto test_iter = std::find (selected_cells.begin(), selected_cells.end(), i);
                                 test_iter != selected_cells.end())
                        {
                            selected_cells.erase (test_iter);
                        }
                        else
                        {
                            if (! e->mods.isAnyModifierKeyDown())
                            {
                                selected_cells = { i };
                            }
                            else if (e->mods.isCommandDown())
                            {
                                chowdsp::VectorHelpers::insert_sorted (selected_cells, static_cast<size_t> (i));
                            }
                            else if (e->mods.isShiftDown())
                            {
                                if (selected_cells.empty())
                                {
                                    selected_cells = { i };
                                }
                                else if (i < selected_cells.front())
                                {
                                    for (size_t index_to_insert = selected_cells.front() - 1; index_to_insert >= i; --index_to_insert)
                                        selected_cells.insert (selected_cells.begin(), index_to_insert);
                                }
                                else
                                {
                                    for (size_t index_to_insert = selected_cells.back() + 1; index_to_insert <= i; ++index_to_insert)
                                        selected_cells.emplace_back (index_to_insert);
                                }
                            }
                        }

                        queue_list.update_list (play_queue);
                        repaint();
                    };
                    new_cell_component->cell_right_clicked = [this, i = idx, selection_logic = new_cell_component->cell_clicked] (const library::Song& selected_song)
                    {
                        selection_logic (selected_song);

                        juce::PopupMenu menu;

                        if (i > 0 && queue_list.selected_cell_idxs.front() != 0)
                        {
                            juce::PopupMenu::Item move_up_item;
                            move_up_item.text = "Move Up";
                            move_up_item.itemID = 100;
                            move_up_item.action = [this]
                            {
                                play_queue.move_songs_up (queue_list.selected_cell_idxs);
                                for (auto& song_index : queue_list.selected_cell_idxs)
                                    song_index--;
                                play_queue.queue_changed();
                            };
                            menu.addItem (std::move (move_up_item));
                        }

                        if (i < play_queue.queue.size() - 1
                            && queue_list.selected_cell_idxs.back() != play_queue.queue.size() - 1)
                        {
                            juce::PopupMenu::Item move_down_item;
                            move_down_item.text = "Move Down";
                            move_down_item.itemID = 101;
                            move_down_item.action = [this]
                            {
                                play_queue.move_songs_down (queue_list.selected_cell_idxs);
                                for (auto& song_index : queue_list.selected_cell_idxs)
                                    song_index++;
                                play_queue.queue_changed();
                            };
                            menu.addItem (std::move (move_down_item));
                        }

                        {
                            juce::PopupMenu::Item remove_item;
                            remove_item.text = "Remove";
                            remove_item.itemID = 103;
                            remove_item.action = [this]
                            {
                                play_queue.remove_songs (queue_list.selected_cell_idxs);
                            };
                            menu.addItem (std::move (remove_item));
                        }

                        menu.showMenuAsync (juce::PopupMenu::Options {});
                    };
                }

                queue_list.add_cell (new_cell_entry, new_cell_component);
            }
            queue_list.update_list (play_queue);
            queue_list.update_size();
            repaint();
        });
}

constexpr int footer_item_height = 30;
void Play_Queue_View::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    bounds.removeFromBottom (footer_item_height);
    const auto label_bounds = bounds.removeFromBottom (footer_item_height);
    g.setColour (juce::Colours::grey);
    const auto label = chowdsp::format (queue_list.allocator.allocator,
                                        "{:02d}:{:02d} || {:02d}:{:02d}",
                                        queue_list.elapsed_length_seconds / 60,
                                        queue_list.elapsed_length_seconds % 60,
                                        queue_list.queue_length_seconds / 60,
                                        queue_list.queue_length_seconds % 60);
    g.drawFittedText (chowdsp::toString (label),
                      label_bounds,
                      juce::Justification::left,
                      1);
}

void Play_Queue_View::resized()
{
    auto bounds = getLocalBounds();
    clear_queue_button.setBounds (bounds.removeFromBottom (footer_item_height));
    bounds.removeFromBottom (footer_item_height);
    queue_list.setBounds (bounds);
}
} // namespace chow_tunes::gui
