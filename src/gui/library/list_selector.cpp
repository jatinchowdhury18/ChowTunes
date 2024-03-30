#include "list_selector.h"

namespace chow_tunes::gui
{
static constexpr auto cell_height = 30;

template <typename Cell_Data>
List_Selector<Cell_Data>::List_Selector()
{
    internal.parent = this;
    setViewedComponent (&internal, false);
    setScrollBarsShown (true, false);
}

template <typename Cell_Data>
void List_Selector<Cell_Data>::update_size()
{
    internal.setBounds (0,
                        0,
                        getWidth(),
                        juce::jmax (cell_height * (int) cell_entries.size(), getHeight()));
    internal.resized();
}

template <typename Cell_Data>
void List_Selector<Cell_Data>::add_cell (Cell_Entry& entry, Cell_Component* cell)
{
    cell->list = this;
    cell->data = entry.data;
    internal.addAndMakeVisible (cell);
    entry.component = cell;
}

template <typename Cell_Data>
void List_Selector<Cell_Data>::resized()
{
    update_size();
}

template <typename Cell_Data>
void List_Selector<Cell_Data>::clear_selection()
{
    for (auto& cell : cell_entries)
        cell.component->is_selected = false;
}

template <typename Cell_Data>
void List_Selector<Cell_Data>::List_Selector_Internal::resized()
{
    for (auto [idx, cell] : chowdsp::enumerate (parent->cell_entries))
        cell.component->setBounds (0, (int) idx * cell_height, getWidth(), cell_height);
}

template struct List_Selector<library::Song>;
template struct List_Selector<library::Album>;
template struct List_Selector<library::Artist>;

//=============================================
template <typename Cell_Data>
void Cell_Base<Cell_Data>::paint (juce::Graphics& g)
{
    if (is_selected)
        g.fillAll (selection_fill_colour);

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);

    const auto text_str = juce::String::fromUTF8 ((const char*) label_text.data(), (int) label_text.size());
    g.drawFittedText (text_str, getLocalBounds(), juce::Justification::centredLeft, 1);
}

template <typename Cell_Data>
void Cell_Base<Cell_Data>::select_cell (bool clear_existing_selection)
{
    if (is_selected)
        return;

    if (clear_existing_selection)
        list->clear_selection();
    is_selected = true;
    list->repaint();
}

template <typename Cell_Data>
void Cell_Base<Cell_Data>::mouseDown (const juce::MouseEvent& e)
{
    latest_mouse_event.emplace (e);

    if (list->select_on_click)
        select_cell();

    if (e.mods.isPopupMenu())
    {
        cell_right_clicked (*data);
        return;
    }

    cell_clicked (*data);
}

template <typename Cell_Data>
void Cell_Base<Cell_Data>::mouseDoubleClick (const juce::MouseEvent& e)
{
    latest_mouse_event.emplace (e);

    if (list->select_on_click)
        select_cell();

    cell_double_clicked (*data);
}

template struct Cell_Base<library::Song>;
template struct Cell_Base<library::Album>;
template struct Cell_Base<library::Artist>;
} // namespace chow_tunes::gui
