#include "list_selector.h"

namespace chow_tunes::gui
{
static constexpr auto cell_height = 30;

template <typename Cell_Data>
List_Selector<Cell_Data>::List_Selector()
{
    cell_entries.reserve (100);
    cell_components.reserve (100);
    internal.parent = this;
    setViewedComponent (&internal, false);
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
void List_Selector<Cell_Data>::add_cell (Cell_Entry& entry, Cell_Component& cell)
{
    cell.list = this;
    cell.data = entry.data;
    internal.addAndMakeVisible (cell);
    entry.component_id = cell_components.size() - 1;
    jassert (cell_components[entry.component_id].get() == &cell);
}

template <typename Cell_Data>
void List_Selector<Cell_Data>::resized()
{
    update_size();
}

template <typename Cell_Data>
void List_Selector<Cell_Data>::clear_selection()
{
    for (auto& cell : cell_components)
        cell->is_selected = false;
}

template <typename Cell_Data>
void List_Selector<Cell_Data>::List_Selector_Internal::resized()
{
    for (auto [idx, cell] : chowdsp::enumerate (parent->cell_entries))
        parent->cell_components[cell.component_id]->setBounds (0, (int) idx * cell_height, getWidth(), cell_height);
}

template struct List_Selector<library::Song>;
template struct List_Selector<library::Album>;
template struct List_Selector<library::Artist>;

//=============================================
template <typename Cell_Data>
void Cell_Base<Cell_Data>::paint (juce::Graphics& g)
{
    if (is_selected)
        g.fillAll (juce::Colours::dodgerblue.withAlpha (0.5f));

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawFittedText (chowdsp::toString (label_text), getLocalBounds(), juce::Justification::centredLeft, 1);
}

template <typename Cell_Data>
void Cell_Base<Cell_Data>::select_cell()
{
    if (is_selected)
        return;

    list->clear_selection();
    is_selected = true;
    list->repaint();
}

template <typename Cell_Data>
void Cell_Base<Cell_Data>::mouseDown (const juce::MouseEvent& e)
{
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
void Cell_Base<Cell_Data>::mouseDoubleClick (const juce::MouseEvent&)
{
    if (list->select_on_click)
        select_cell();

    cell_double_clicked (*data);
}

template struct Cell_Base<library::Song>;
template struct Cell_Base<library::Album>;
template struct Cell_Base<library::Artist>;
} // namespace chow_tunes::gui
