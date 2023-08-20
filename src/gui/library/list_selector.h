#pragma once

#include "library/music_library.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes::gui
{
template <typename Cell_Data>
struct Cell_Component;

template <typename Cell_Data>
struct List_Selector : juce::Viewport
{
    using Cell_Component = Cell_Component<Cell_Data>;

    struct Cell_Entry
    {
        const Cell_Data* data = nullptr;
        size_t component_id = 0;
    };

    List_Selector();

    void update_size();
    void add_cell (Cell_Entry& entry, Cell_Component& cell);

    void resized() override;
    void clear_selection();

    struct List_Selector_Internal : juce::Component
    {
        List_Selector* parent = nullptr;
        void resized() override;
    } internal;

    std::vector<Cell_Entry> cell_entries;
    std::vector<chowdsp::LocalPointer<Cell_Component, 800>> cell_components;

    bool select_on_click = true;
};

template <typename Data_Type>
struct Cell_Base : juce::Component
{
    const Data_Type* data = nullptr;
    List_Selector<Data_Type>* list = nullptr;
    bool is_selected = false;
    std::u8string_view label_text {};

    std::function<void (const Data_Type&)> cell_clicked = [] (const Data_Type&) {};
    std::function<void (const Data_Type&)> cell_right_clicked = [] (const Data_Type&) {};
    std::function<void (const Data_Type&)> cell_double_clicked = [] (const Data_Type&) {};

    void select_cell();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
};

template <>
struct Cell_Component<library::Song> : Cell_Base<library::Song>
{
};

template <>
struct Cell_Component<library::Album> : Cell_Base<library::Album>
{
};

template <>
struct Cell_Component<library::Artist> : Cell_Base<library::Artist>
{
};
} // namespace chow_tunes::gui
