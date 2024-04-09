#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <chowdsp_logging/chowdsp_logging.h>

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

    if (auto& player_opt = *player; player_opt.has_value())
    {
        needs_repaint |= compare_exchange (play_percent, player_opt->get_song_progress_percent());
        needs_repaint |= compare_exchange (playing_seconds, player_opt->get_seconds_played());

        const auto player_song_length_seconds = static_cast<double> (player_opt->song_length_samples.load()) / static_cast<double> (player_opt->song_sample_rate.load());
        needs_repaint |= compare_exchange (song_length_seconds, static_cast<size_t> (juce::roundToInt (player_song_length_seconds)));
    }
    else
    {
        needs_repaint |= compare_exchange (play_percent, 0.0);
        needs_repaint |= compare_exchange (playing_seconds, (size_t) 0);
        needs_repaint |= compare_exchange (song_length_seconds, (size_t) 0);
    }
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

    g.setColour (juce::Colours::white);
    chowdsp::ArenaAllocator<std::array<std::byte, 128>> timeline_arena {};
    const auto timeline_label = chowdsp::format (timeline_arena,
                                                 "{:02d}:{:02d} || {:02d}:{:02d}",
                                                 playing_seconds / 60,
                                                 playing_seconds % 60,
                                                 song_length_seconds / 60,
                                                 song_length_seconds % 60);
    g.drawFittedText (chowdsp::toString (timeline_label),
                      local_bounds,
                      juce::Justification::left,
                      1);
}

void Transport_Timeline::mouseDrag (const juce::MouseEvent& e)
{
    movePlayhead (e);
}

void Transport_Timeline::mouseUp (const juce::MouseEvent& e)
{
    movePlayhead (e);
}

void Transport_Timeline::movePlayhead (const juce::MouseEvent& e)
{
    const auto new_play_percent = (double) std::clamp (e.getPosition().x, 0, getWidth()) / (double) getWidth();
    action_router->route_action ({
        .action_type = audio::Audio_Player_Action_Type::Move_Playhead,
        .action_value = new_play_percent,
    });
}
} // namespace chow_tunes::gui
