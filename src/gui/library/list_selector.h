#pragma once

#include "library/music_library.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes::gui
{
template <typename Cell_Type>
struct List_Selector : juce::Viewport
{
    using Cell_List = std::vector<std::unique_ptr<Cell_Type>>;
    Cell_List cells;

    List_Selector();

    void update_size();
    void add_cell (Cell_Type& cell);

    void resized() override;
    void clear_selection();

    struct List_Selector_Internal : juce::Component
    {
        List_Selector* parent = nullptr;
        void resized() override;
    } internal;
};

struct Song_Cell : juce::Component
{
    const library::Song* song = nullptr;
    List_Selector<Song_Cell>* list = nullptr;
    bool is_selected = false;
    std::function<void (const library::Song&)> song_selected_callback {};

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override;
};

struct Album_Cell : juce::Component
{
    const library::Album* album = nullptr;
    List_Selector<Album_Cell>* list = nullptr;
    bool is_selected = false;
    std::function<void (const library::Album&)> album_selected_callback {};

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override;
};

struct Artist_Cell : juce::Component
{
    const library::Artist* artist = nullptr;
    List_Selector<Artist_Cell>* list = nullptr;
    bool is_selected = false;
    std::function<void (const library::Artist&)> artist_selected_callback {};

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override;
};
} // namespace chow_tunes::gui
