#pragma once

#include "audio/audio_player_actions.h"
#include "library/music_library.h"
#include <chowdsp_listeners/chowdsp_listeners.h>

namespace chow_tunes::play_queue
{
struct Play_Queue
{
    std::vector<const library::Song*> queue;
    int currently_playing_song_index = -1;

    audio::Audio_Player_Action_Router* action_router = nullptr;
    chowdsp::Broadcaster<void()> queue_changed;

    enum class Add_To_Queue_Action
    {
        Play_Now,
        Insert_Next,
        Insert_Last,
    };

    void init();
    void play_previous_song();
    void play_next_song();
    void restart_current_song();

    void add_to_queue (std::span<const library::Song*> songs_to_add, Add_To_Queue_Action action);
    void move_song_up (const library::Song* song);
    void move_song_down (const library::Song* song);
    void remove_song (size_t song_idx);
    void clear_queue();
};
} // namespace chow_tunes::play_queue
