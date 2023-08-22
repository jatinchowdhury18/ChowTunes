#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <sstream>

#include "audio/audio_player.h"
#include "transport_timeline.h"

namespace chow_tunes::gui
{
void Transport_Timeline::update()
{
    bool needs_repaint = false;
    const auto _ = chowdsp::EndOfScopeAction {
        [this, &needs_repaint]
        {
            if (needs_repaint)
                repaint();
        }
    };

    const auto compare_exchange = [] (auto& val, auto compare_val)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
        const auto result = val != compare_val;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        val = compare_val;
        return result;
    };

    needs_repaint |= compare_exchange (play_percent, player->get_song_progress_percent());
    needs_repaint |= compare_exchange (playing_seconds, player->get_seconds_played());
}

void Transport_Timeline::paint (juce::Graphics& g)
{
    auto local_bounds = getLocalBounds();
    local_bounds.removeFromTop (5);

    g.setColour (juce::Colours::darkgrey);
    auto track_bounds = local_bounds.removeFromTop (5).toFloat();
    g.fillRoundedRectangle (track_bounds, 5.0f);

    const auto thumb_x = (float) getWidth() * (0.005f + 0.99f * (float) play_percent);
    g.setColour (juce::Colours::lightgrey);
    g.fillRoundedRectangle (track_bounds.removeFromLeft (thumb_x), 5.0f);

    g.setColour (juce::Colours::red.darker (0.1f));
    g.fillEllipse (juce::Rectangle { 10.0f, 10.0f }.withCentre ({ thumb_x, 7.5f }));

    const auto time_str = [this] () -> std::string
    {
        const auto minutes_to_show = playing_seconds / 60;
        const auto seconds_to_show = playing_seconds % 60;

        std::stringstream ss;
        ss << std::internal << std::setfill('0') << std::setw(2) << minutes_to_show;
        ss << ':';
        ss << std::internal << std::setfill('0') << std::setw(2) << seconds_to_show;
        return ss.str();
    }();

    g.setColour (juce::Colours::white);
    g.drawFittedText (time_str, local_bounds, juce::Justification::left, 1);
}

void Transport_Timeline::mouseDrag (const juce::MouseEvent& e)
{
    const auto new_play_percent = (double) e.getPosition().x / (double) getWidth();
    action_router->route_action ({
        .action_type = audio::Audio_Player_Action_Type::Move_Playhead,
        .action_value = new_play_percent,
    });
}

void Transport_Timeline::mouseUp (const juce::MouseEvent& e)
{
    const auto new_play_percent = (double) e.getPosition().x / (double) getWidth();
    action_router->route_action ({
        .action_type = audio::Audio_Player_Action_Type::Move_Playhead,
        .action_value = new_play_percent,
    });
}
} // namespace chow_tunes::gui
