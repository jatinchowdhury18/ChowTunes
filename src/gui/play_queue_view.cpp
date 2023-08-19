#include "play_queue_view.h"

namespace chow_tunes::gui
{
Play_Queue_View::Play_Queue_View (play_queue::Play_Queue& queue)
    : play_queue (queue)
{
    queue_list.select_on_click = false;
    addAndMakeVisible (queue_list);
    queue_change_callback = play_queue.queue_changed.connect (
        [this]
        {
            queue_list.cells.clear();
            for (auto [idx, song] : chowdsp::enumerate (play_queue.queue))
            {
                auto& new_cell = queue_list.cells.emplace_back (std::make_unique<gui::Song_Cell>());
                new_cell->data = song;
                new_cell->label_text = new_cell->data->name;
                new_cell->setWantsKeyboardFocus (true);
                new_cell->cell_right_clicked = [this, i = idx] (const library::Song& selected_song)
                {
                    juce::PopupMenu menu;

                    if (i > 0)
                    {
                        juce::PopupMenu::Item move_up_item;
                        move_up_item.text = "Move Up";
                        move_up_item.itemID = 100;
                        move_up_item.action = [this, &selected_song]
                        {
                            play_queue.move_song_up (&selected_song);
                        };
                        menu.addItem (std::move (move_up_item));
                    }

                    if (i < play_queue.queue.size() - 1)
                    {
                        juce::PopupMenu::Item move_down_item;
                        move_down_item.text = "Move Down";
                        move_down_item.itemID = 101;
                        move_down_item.action = [this, &selected_song]
                        {
                            play_queue.move_song_down (&selected_song);
                        };
                        menu.addItem (std::move (move_down_item));
                    }

                    menu.showMenuAsync (juce::PopupMenu::Options{});
                };

                queue_list.add_cell (*new_cell);
                if ((int) idx == play_queue.currently_playing_song_index)
                    new_cell->select_cell();
            }
            queue_list.update_size();
        });
}

void Play_Queue_View::resized()
{
    queue_list.setBounds (getLocalBounds());
}
} // namespace chow_tunes::gui
