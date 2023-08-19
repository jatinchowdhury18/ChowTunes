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
    auto [buffer, fs] = audio_file_helper->loadFile (juce::File { chowdsp::toString (song.filepath) });
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
    if (action.action_type == Audio_Player_Action_Type::Start_New_Song)
    {
        audio_player.ui_to_audio_queue.enqueue (std::move (action));
    }
    else if (action.action_type == audio::Audio_Player_Action_Type::Dead_Song)
    {
        action.audio_buffer.reset();
    }
    else if (action.action_type == audio::Audio_Player_Action_Type::Song_Finished)
    {
        action.audio_buffer.reset();
        play_queue.play_next_song();
    }
}
} // namespace chow_tunes::audio
