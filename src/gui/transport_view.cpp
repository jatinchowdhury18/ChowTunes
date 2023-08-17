#include "transport_view.h"
#include "audio/audio_player.h"

namespace chow_tunes::gui
{
Transport_View::Transport_View (audio::Audio_Player& player)
{
    using Play_State = audio::Audio_Player::State;

    addAndMakeVisible (playButton);
    playButton.onClick = [&player]
    {
        chowdsp::AtomicHelpers::compareExchange (player.state, Play_State::Paused, Play_State::Playing);
    };

    addAndMakeVisible (pauseButton);
    pauseButton.onClick = [&player]
    {
        chowdsp::AtomicHelpers::compareExchange (player.state, Play_State::Playing, Play_State::Paused);
    };
}

void Transport_View::resized()
{
     auto bounds = getLocalBounds();
     playButton.setBounds (bounds.removeFromLeft (80).withHeight (35));
     pauseButton.setBounds (bounds.removeFromLeft (80).withHeight (35));
}
}
