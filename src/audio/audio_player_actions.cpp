#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

#include "audio_player.h"
#include "audio_player_actions.h"
#include "library/music_library.h"
#include "play_queue/play_queue.h"

namespace chow_tunes::audio
{
Audio_Player_Action create_play_song_action (const library::Song& song)
{
    chowdsp::SharedAudioFileSaveLoadHelper audio_file_helper;
    auto [buffer, fs] = audio_file_helper->loadFile (juce::File { juce::String { (const char*) song.filepath.data(), song.filepath.size() } });
    if (buffer.getNumSamples() == 0)
    {
        jassertfalse;
        return {
            .action_type = Audio_Player_Action_Type::Invalid
        };
    }

    Audio_Player_Action action;
    action.action_type = audio::Audio_Player_Action_Type::Start_New_Song;
    action.audio_buffer = std::make_unique<juce::AudioBuffer<float>> (std::move (buffer));
    action.sample_rate = fs;
    return action;
}

void Audio_Player_Action_Router::route_action (Audio_Player_Action&& action)
{
    using Action_Type = Audio_Player_Action_Type;
    using Play_State = audio::Audio_Player::State;

    if (action.action_type == Action_Type::Start_New_Song)
    {
        audio_player.ui_to_audio_queue.enqueue (std::move (action));
    }
    else if (action.action_type == Action_Type::Dead_Song)
    {
        action.audio_buffer.reset();
    }
    else if (action.action_type == Action_Type::Song_Finished)
    {
        action.audio_buffer.reset();
        play_queue.play_next_song();
    }
    else if (action.action_type == Action_Type::Play_Song)
    {
        chowdsp::AtomicHelpers::compareExchange (audio_player.state, Play_State::Paused, Play_State::Playing);
    }
    else if (action.action_type == Action_Type::Pause_Song)
    {
        chowdsp::AtomicHelpers::compareExchange (audio_player.state, Play_State::Playing, Play_State::Paused);
    }
    else if (action.action_type == Action_Type::Restart_Song)
    {
        play_queue.restart_current_song();
    }
    else if (action.action_type == Action_Type::Previous_Song)
    {
        play_queue.play_previous_song();
    }
    else if (action.action_type == Action_Type::Next_Song)
    {
        play_queue.play_next_song();
    }

    play_state_changed();
}
} // namespace chow_tunes::audio
