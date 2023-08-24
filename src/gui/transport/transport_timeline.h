#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes::audio
{
struct Audio_Player;
struct Audio_Player_Action_Router;
}

namespace chow_tunes::gui
{
struct Transport_Timeline : juce::Component
{
    void paint (juce::Graphics& g) override;
    void update();

    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void movePlayhead (const juce::MouseEvent& e);

    audio::Audio_Player_Action_Router* action_router = nullptr;
    audio::Audio_Player* player = nullptr;
    double play_percent = 0.0;
    size_t playing_seconds = 0;
};
}
