#pragma once

#include "library/music_library.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes::gui
{
struct Component_Arena
{
    static constexpr size_t arena_size_bytes = 8192;
    chowdsp::ChainedArenaAllocator<std::array<std::byte, arena_size_bytes>> arena {};

    chowdsp::SmallVector<juce::Component*, 200> component_list {};

    ~Component_Arena()
    {
        clear_all();
    }

    template <typename T, typename... Args>
    T* allocate (Args&&... args)
    {
        auto* bytes = arena.allocate_bytes (sizeof (T), alignof (T));
        auto* new_component = new (bytes) T { std::forward<Args> (args)... };

        if constexpr (std::is_base_of_v<juce::Component, T>)
            component_list.emplace_back (new_component);

        return new_component;
    }

    template <typename T, typename... Args>
    std::span<T> allocate_n (size_t n, Args&&... args)
    {
        auto* bytes = arena.allocate_bytes (sizeof (T) * n, alignof (T));
        auto span = std::span<T> { reinterpret_cast<T*> (bytes), n };
        for (auto& ptr : span)
        {
            auto* new_component = new (&ptr) T { std::forward<Args> (args)... };
            if constexpr (std::is_base_of_v<juce::Component, T>)
                component_list.emplace_back (new_component);
        }
        return span;
    }

    void clear_all()
    {
        for (auto* c : component_list)
            c->~Component();
        component_list.clear();
        arena.clear();
    }
};

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
    Component_Arena allocator;

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
