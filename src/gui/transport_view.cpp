#include "transport_view.h"
#include "audio/audio_player.h"
#include "audio/audio_player_actions.h"
#include "play_queue/play_queue.h"

namespace chow_tunes::gui
{
Transport_View::Transport_View (audio::Audio_Player_Action_Router& action_router)
{
    addAndMakeVisible (prev_button);
    addAndMakeVisible (restart_button);
    addAndMakeVisible (play_button);
    addAndMakeVisible (pause_button);
    addAndMakeVisible (next_button);

    using Action_Type = audio::Audio_Player_Action_Type;
    prev_button.onClick = [&action_router]
    {
        action_router.route_action ({ .action_type = Action_Type::Previous_Song });
    };
    restart_button.onClick = [&action_router]
    {
        action_router.route_action ({ .action_type = Action_Type::Restart_Song });
    };
    play_button.onClick = [&action_router]
    {
        action_router.route_action ({ .action_type = Action_Type::Play_Song });
    };
    pause_button.onClick = [&action_router]
    {
        action_router.route_action ({ .action_type = Action_Type::Pause_Song });
    };
    next_button.onClick = [&action_router]
    {
        action_router.route_action ({ .action_type = Action_Type::Next_Song });
    };

    const auto update_button_states = [this, &play_queue = action_router.play_queue, &player = action_router.audio_player]
    {
        prev_button.setEnabled (! play_queue.queue.empty() && play_queue.currently_playing_song_index > 0);
        next_button.setEnabled (! play_queue.queue.empty() && play_queue.currently_playing_song_index < (int) play_queue.queue.size() - 1);
        restart_button.setEnabled (! play_queue.queue.empty() && play_queue.currently_playing_song_index >= 0);

        using Play_State = audio::Audio_Player::State;
        const auto play_state = player.state.load();

        play_button.setEnabled (play_state == Play_State::Paused);
//        play_button.setToggleState (play_state == Play_State::Playing, juce::dontSendNotification);

        pause_button.setEnabled (play_state == Play_State::Playing);
//        pause_button.setToggleState (play_state == Play_State::Paused, juce::dontSendNotification);
    };

    button_change_callbacks += {
        action_router.play_queue.queue_changed.connect (update_button_states),
        action_router.play_state_changed.connect (update_button_states),
    };
    update_button_states();
}

void Transport_View::resized()
{
    auto bounds = getLocalBounds();
    prev_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    restart_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    play_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    pause_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    next_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
}
} // namespace chow_tunes::gui
