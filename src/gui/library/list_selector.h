#pragma once

#include "library/music_library.h"
#include <juce_gui_basics/juce_gui_basics.h>

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
    std::function<void (const library::Song&)> song_selected_callback {};

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
        song_selected_callback (*song);
    }
};

struct Album_Cell : juce::Component
{
    const library::Album* album = nullptr;
    List_Selector<Album_Cell>* list = nullptr;
    bool is_selected = false;
    std::function<void (const library::Album&)> album_selected_callback {};

    void paint (juce::Graphics& g) override
    {
        if (is_selected)
            g.fillAll (juce::Colours::dodgerblue.withAlpha (0.5f));

        g.setColour (juce::Colours::white);
        g.setFont (14.0f);
        g.drawFittedText (chowdsp::toString (album->name), getLocalBounds(), juce::Justification::centredLeft, 1);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        if (is_selected)
            return;

        list->clear_selection();
        is_selected = true;
        list->repaint();
        album_selected_callback (*album);
    }
};

struct Artist_Cell : juce::Component
{
    const library::Artist* artist = nullptr;
    List_Selector<Artist_Cell>* list = nullptr;
    bool is_selected = false;
    std::function<void (const library::Artist&)> artist_selected_callback {};

    void paint (juce::Graphics& g) override
    {
        if (is_selected)
            g.fillAll (juce::Colours::dodgerblue.withAlpha (0.5f));

        g.setColour (juce::Colours::white);
        g.setFont (14.0f);
        g.drawFittedText (chowdsp::toString (artist->name), getLocalBounds(), juce::Justification::centredLeft, 1);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        if (is_selected)
            return;

        list->clear_selection();
        is_selected = true;
        list->repaint();
        artist_selected_callback (*artist);
    }
};
} // namespace chow_tunes::gui
