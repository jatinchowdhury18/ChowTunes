#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

#include "audio_file_reader_ffmpeg.h"
#include "audio_player.h"
#include "audio_player_actions.h"
#include "library/music_library.h"
#include "play_queue/play_queue.h"

namespace chow_tunes::audio
{
Audio_Player_Action create_play_song_action (const library::Song& song)
{
    const auto filepath_str = juce::String::fromUTF8 ((const char*) song.filepath.data(), (int) song.filepath.size());
    try
    {
        auto [buffer, fs] = ffmpeg_reader::read_file (filepath_str.toStdString());
        Audio_Player_Action action;
        action.action_type = audio::Audio_Player_Action_Type::Start_New_Song;
        action.audio_buffer = buffer;
        action.action_value = static_cast<double> (fs);
        return action;
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog (e.what());
        return {
            .action_type = Audio_Player_Action_Type::Invalid
        };
    }
}

static void free_buffer (const Read_Buffer& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        chowdsp::aligned_free (buffer.getWritePointer (ch));
}

void Audio_Player_Action_Router::route_action (Audio_Player_Action&& action)
{
    using Action_Type = Audio_Player_Action_Type;
    using Play_State = audio::Audio_Player::State;

    if (action.action_type == Action_Type::Start_New_Song
        || action.action_type == Action_Type::Move_Playhead
        || action.action_type == Action_Type::Stop_Song)
    {
        if (audio_player.has_value())
            audio_player->ui_to_audio_queue.enqueue (std::move (action));
    }
    else if (action.action_type == Action_Type::Dead_Song)
    {
        free_buffer (action.audio_buffer);
        action.audio_buffer = {};
    }
    else if (action.action_type == Action_Type::Song_Finished)
    {
        free_buffer (action.audio_buffer);
        action.audio_buffer = {};
        play_queue.play_next_song();
    }
    else if (action.action_type == Action_Type::Play_Song)
    {
        if (audio_player.has_value())
            chowdsp::AtomicHelpers::compareExchange (audio_player->state, Play_State::Paused, Play_State::Playing);
    }
    else if (action.action_type == Action_Type::Pause_Song)
    {
        if (audio_player.has_value())
            chowdsp::AtomicHelpers::compareExchange (audio_player->state, Play_State::Playing, Play_State::Paused);
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
