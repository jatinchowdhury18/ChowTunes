#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "library/music_library.h"

namespace chow_tunes::gui
{
template <typename Cell_Type>
struct List_Selector : juce::Viewport
{
    std::vector<std::unique_ptr<Cell_Type>> cells;

    void resized() override
    {
        static constexpr auto cell_height = 30;
        for (auto [idx, cell] : chowdsp::enumerate (cells))
        {
            cell->setBounds (0, (int) idx * cell_height, getWidth(), cell_height);
        }
    }

    void clear_selection()
    {
        for (auto& cell : cells)
            cell->is_selected = false;
    }

    //    void paint (juce::Graphics& g) override
    //    {
    //        g.fillAll (juce::Colours::yellow.withAlpha (0.15f));
    //    }
};

struct Song_Cell : juce::Component
{
    const library::Song* song = nullptr;
    List_Selector<Song_Cell>* list = nullptr;
    bool is_selected = false;
    std::function<void (std::string_view)> play_song_callback {};

    void paint (juce::Graphics& g) override
    {
        if (is_selected)
            g.fillAll (juce::Colours::dodgerblue.withAlpha (0.5f));

        g.setColour (juce::Colours::white);
        g.setFont (14.0f);
        g.drawFittedText (chowdsp::toString (song->name), getLocalBounds(), juce::Justification::centredLeft, 1);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        if (is_selected)
            return;

        list->clear_selection();
        is_selected = true;
        list->repaint();
        play_song_callback (song->filepath);
    }
};
}
