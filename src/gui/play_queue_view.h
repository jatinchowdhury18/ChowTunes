#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "library/list_selector.h"
#include "play_queue/play_queue.h"

namespace chow_tunes::gui
{
struct Play_Queue_List : List_Selector<library::Song>
{
    size_t previous_play_queue_size = 0;
    std::vector<size_t> selected_cell_idxs {};
    size_t elapsed_length_seconds = 0;
    size_t queue_length_seconds = 0;

    void update_list (const play_queue::Play_Queue& queue);
};

struct Play_Queue_View : juce::Component
{
    explicit Play_Queue_View (play_queue::Play_Queue& queue);

    void paint (juce::Graphics& g) override;
    void resized() override;

    play_queue::Play_Queue& play_queue;
    Play_Queue_List queue_list;
    chowdsp::ScopedCallback queue_change_callback;
    juce::TextButton clear_queue_button { "Clear Queue" };
};
} // namespace chow_tunes::gui
