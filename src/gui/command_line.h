#pragma once

#include <juce_core/juce_core.h>

namespace chow_tunes::library
{
struct Music_Library;
}

namespace chow_tunes::play_queue
{
struct Play_Queue;
}

namespace chow_tunes::cli
{
void play_command (const juce::StringArray& args,
                   const library::Music_Library& library,
                   play_queue::Play_Queue& play_queue);
}
