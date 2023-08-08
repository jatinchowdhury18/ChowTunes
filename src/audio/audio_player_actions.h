#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace chow_tunes::audio
{
enum class Audio_Player_Action_Type
{
    Start_New_Song,
    Play_Song,
    Pause_Song,
    Previous_Song,
    Next_Song,
    Song_Finished,
    Dead_Song,
};

struct Audio_Player_Action
{
    Audio_Player_Action_Type action_type;
    std::unique_ptr<juce::AudioBuffer<float>> audio_buffer;
};
}
