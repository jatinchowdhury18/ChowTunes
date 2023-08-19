#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes::audio
{
struct Audio_Player;
}

namespace chow_tunes::gui
{
struct Transport_View : juce::Component
{
    explicit Transport_View (audio::Audio_Player& player);
    void resized() override;

    juce::TextButton prevButton { "PREVIOUS" };
    juce::TextButton restartButton { "RESTART" };
    juce::TextButton playButton { "PLAY" };
    juce::TextButton pauseButton { "PAUSE" };
    juce::TextButton nextButton { "NEXT" };
};
} // namespace chow_tunes::gui
