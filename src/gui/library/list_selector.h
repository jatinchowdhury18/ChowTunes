#pragma once

#include "library/music_library.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <chowdsp_gui/chowdsp_gui.h>

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
        Cell_Component* component = nullptr;
    };

    List_Selector();

    void update_size();
    void add_cell (Cell_Entry& entry, Cell_Component* cell);

    void resized() override;
    void clear_selection();

    struct List_Selector_Internal : juce::Component
    {
        List_Selector* parent = nullptr;
        void resized() override;
    } internal;

    std::span<Cell_Entry> cell_entries;
    chowdsp::ComponentArena<8192, 200> allocator;

    bool select_on_click = true;
};

template <typename Data_Type>
struct Cell_Base : juce::Component
{
    const Data_Type* data = nullptr;
    List_Selector<Data_Type>* list = nullptr;
    bool is_selected = false;
    juce::Colour selection_fill_colour = juce::Colours::dodgerblue.withAlpha (0.5f);
    std::u8string_view label_text {};
    std::optional<juce::MouseEvent> latest_mouse_event;

    std::function<void (const Data_Type&)> cell_clicked = [] (const Data_Type&) {};
    std::function<void (const Data_Type&)> cell_right_clicked = [] (const Data_Type&) {};
    std::function<void (const Data_Type&)> cell_double_clicked = [] (const Data_Type&) {};

    void select_cell (bool clear_existing_selection = true);

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
