#pragma once

#include <chowdsp_listeners/chowdsp_listeners.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "transport_timeline.h"

namespace chow_tunes::audio
{
struct Audio_Player_Action_Router;
}

namespace chow_tunes::state
{
struct State;
}

namespace chow_tunes::gui
{
struct Transport_View : juce::Component
{
    explicit Transport_View (state::State& app_state, audio::Audio_Player_Action_Router& action_router);

    void resized() override;
    void paint (juce::Graphics& g) override;

    juce::TextButton prev_button { "PREVIOUS" };
    juce::TextButton restart_button { "RESTART" };
    juce::TextButton play_button { "PLAY" };
    juce::TextButton pause_button { "PAUSE" };
    juce::TextButton next_button { "NEXT" };
    chowdsp::ScopedCallbackList button_change_callbacks;

    juce::Slider volume_slider;
    Transport_Timeline timeline;

    juce::Image song_artwork;
};
} // namespace chow_tunes::gui
