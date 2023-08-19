#pragma once

#include "library/music_library.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes::gui
{
template <typename Cell_Type>
struct List_Selector : juce::Viewport
{
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

    using Cell_List = std::vector<std::unique_ptr<Cell_Type>>;
    Cell_List cells;

    bool select_on_click = true;
};

template <typename Data_Type, typename Cell_Type>
struct Cell_Base : juce::Component
{
    const Data_Type* data = nullptr;
    List_Selector<Cell_Type>* list = nullptr;
    bool is_selected = false;
    std::string_view label_text {};

    std::function<void (const Data_Type&)> cell_clicked = [] (const Data_Type&) {};
    std::function<void (const Data_Type&)> cell_right_clicked = [] (const Data_Type&) {};
    std::function<void (const Data_Type&)> cell_double_clicked = [] (const Data_Type&) {};

    void select_cell();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
};

struct Song_Cell : Cell_Base<library::Song, Song_Cell>
{
};
struct Album_Cell : Cell_Base<library::Album, Album_Cell>
{
};
struct Artist_Cell : Cell_Base<library::Artist, Artist_Cell>
{
};
} // namespace chow_tunes::gui
