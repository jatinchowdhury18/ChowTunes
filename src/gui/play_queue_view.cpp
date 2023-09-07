#include "play_queue_view.h"

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

    for (auto [idx, entry] : chowdsp::enumerate (cell_entries))
    {
        auto* cell = cell_components.find (entry.component_locator)->get();
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
    queue_list.cell_entries.reserve (100);
    queue_change_callback = play_queue.queue_changed.connect (
        [this]
        {
            queue_list.cell_entries.clear();
            queue_list.cell_components.reset();
            for (auto [idx, song] : chowdsp::enumerate (play_queue.queue))
            {
                auto& new_cell_entry = queue_list.cell_entries.emplace_back();
                new_cell_entry.data = song;

                auto [new_cell_locator, new_cell_ptr] = queue_list.cell_components.emplace();
                auto* new_cell_component = new_cell_ptr->emplace();
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
                        queue_list.repaint();
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

                queue_list.add_cell (new_cell_entry, new_cell_locator, new_cell_component);
            }
            queue_list.update_list (play_queue);
            queue_list.update_size();
        });
}

void Play_Queue_View::resized()
{
    auto bounds = getLocalBounds();
    clear_queue_button.setBounds (bounds.removeFromBottom (30));
    queue_list.setBounds (bounds);
}
} // namespace chow_tunes::gui
