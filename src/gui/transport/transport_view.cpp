#include "transport_view.h"
#include "audio/audio_player.h"
#include "audio/audio_player_actions.h"
#include "gui/gui_resources.h"
#include "play_queue/play_queue.h"
#include "state.h"

namespace chow_tunes::gui
{
Transport_View::Transport_View (state::State& app_state, audio::Audio_Player_Action_Router& action_router)
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
        pause_button.setEnabled (play_state == Play_State::Playing);
    };

    const auto load_fallback_artwork = [this]
    {
        const auto fs = cmrc::gui::get_filesystem();
        const auto icon_file = fs.open ("icon.png");
        song_artwork = juce::ImageCache::getFromMemory (icon_file.begin(), (int) icon_file.size());
    };

    const auto update_artwork = [this, load_fallback_artwork, &play_queue = action_router.play_queue]
    {
        const auto defer_paint = chowdsp::EndOfScopeAction { [this] { repaint(); } };

        if (play_queue.currently_playing_song_index == -1)
        {
            load_fallback_artwork();
            return;
        }

        const auto* song = play_queue.queue[(size_t) play_queue.currently_playing_song_index];
        if (song->artwork_file.empty())
        {
            load_fallback_artwork();
            return;
        }

        const auto artwork_path_str = juce::String::fromUTF8 ((const char*) song->artwork_file.data(), (int) song->artwork_file.size());
        song_artwork = juce::ImageCache::getFromFile (juce::File { artwork_path_str });

        if (song_artwork.isNull())
            load_fallback_artwork();
    };

    button_change_callbacks += {
        action_router.play_queue.queue_changed.connect (
            [update_button_states, update_artwork]
            {
                update_button_states();
                update_artwork();
            }),
        action_router.play_state_changed.connect (update_button_states),
    };
    update_button_states();

    volume_slider.setSliderStyle (juce::Slider::SliderStyle::LinearHorizontal);
    volume_slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    volume_slider.setTextValueSuffix (" dB");
    volume_slider.setRange ({ audio::Audio_Player::min_gain_db, 6.0 }, 0.1);
    volume_slider.setValue (app_state.volume_db, juce::dontSendNotification);
    volume_slider.onValueChange = [this, &app_state]
    {
        app_state.volume_db = (float) volume_slider.getValue();
    };
    addAndMakeVisible (volume_slider);

    load_fallback_artwork();

    timeline.action_router = &action_router;
    timeline.player = &action_router.audio_player;
    addAndMakeVisible (timeline);
}

void Transport_View::resized()
{
    auto bounds = getLocalBounds();
    prev_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    restart_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    play_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    pause_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    next_button.setBounds (bounds.removeFromLeft (80).withHeight (35));
    volume_slider.setBounds (bounds.removeFromLeft (200).withHeight (50).reduced (5));

    timeline.setBounds (0, 60, getWidth() * 2 / 3, 30);
}

void Transport_View::paint (juce::Graphics& g)
{
    const auto artwork_bounds = getLocalBounds().removeFromRight (getHeight());
    g.drawImage (song_artwork, artwork_bounds.toFloat());
}
} // namespace chow_tunes::gui
