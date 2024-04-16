#pragma once

#include <chowdsp_listeners/chowdsp_listeners.h>
#include <chowdsp_buffers/chowdsp_buffers.h>

// forward declarations
namespace chow_tunes
{
namespace library
{
    struct Library;
    struct Song;
}
namespace audio
{
    struct Audio_Player;
}
namespace play_queue
{
    struct Play_Queue;
}
}

namespace chow_tunes::audio
{
enum class Audio_Player_Action_Type
{
    Invalid,
    Start_New_Song,
    Play_Song,
    Pause_Song,
    Stop_Song,
    Restart_Song,
    Previous_Song,
    Next_Song,
    Song_Finished,
    Dead_Song,
    Move_Playhead,
};

struct Audio_Player_Action
{
    Audio_Player_Action_Type action_type;
    std::unique_ptr<chowdsp::Buffer<int16_t>> audio_buffer;
    std::variant<double> action_value; // this value will be different depending on the action!
};

Audio_Player_Action create_play_song_action (const library::Song& song);

struct Audio_Player_Action_Router
{
    void route_action (Audio_Player_Action&& action);

    std::optional<audio::Audio_Player>& audio_player;
    play_queue::Play_Queue& play_queue;
    chowdsp::Broadcaster<void()> play_state_changed;
};
}
