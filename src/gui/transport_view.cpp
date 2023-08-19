#include "transport_view.h"
#include "audio/audio_player.h"

namespace chow_tunes::gui
{
Transport_View::Transport_View (audio::Audio_Player& player)
{
    using Play_State = audio::Audio_Player::State;

    addAndMakeVisible (prevButton);
    addAndMakeVisible (restartButton);

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

    addAndMakeVisible (nextButton);
}

void Transport_View::resized()
{
     auto bounds = getLocalBounds();
     prevButton.setBounds (bounds.removeFromLeft (80).withHeight (35));
     restartButton.setBounds (bounds.removeFromLeft (80).withHeight (35));
     playButton.setBounds (bounds.removeFromLeft (80).withHeight (35));
     pauseButton.setBounds (bounds.removeFromLeft (80).withHeight (35));
     nextButton.setBounds (bounds.removeFromLeft (80).withHeight (35));
}
}
