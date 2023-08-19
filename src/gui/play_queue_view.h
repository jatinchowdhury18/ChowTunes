#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "play_queue/play_queue.h"
#include "library/list_selector.h"

namespace chow_tunes::gui
{
struct Play_Queue_View : juce::Viewport
{
    explicit Play_Queue_View (play_queue::Play_Queue& queue);

    void resized() override;

    play_queue::Play_Queue& play_queue;
    List_Selector<Song_Cell> queue_list;
    chowdsp::ScopedCallback queue_change_callback;
};
} // namespace chow_tunes::gui
