#include "list_selector.h"

namespace chow_tunes::gui
{
static constexpr auto cell_height = 30;

template <typename Cell_Type>
List_Selector<Cell_Type>::List_Selector()
{
    cells.reserve (100);
    internal.parent = this;
    setViewedComponent (&internal, false);
}

template <typename Cell_Type>
void List_Selector<Cell_Type>::update_size()
{
    internal.setBounds (0,
                        0,
                        getWidth(),
                        juce::jmax (cell_height * (int) cells.size(), getHeight()));
    internal.resized();
}

template <typename Cell_Type>
void List_Selector<Cell_Type>::add_cell (Cell_Type& cell)
{
    cell.list = this;
    internal.addAndMakeVisible (cell);
}

template <typename Cell_Type>
void List_Selector<Cell_Type>::resized()
{
    update_size();
}

template <typename Cell_Type>
void List_Selector<Cell_Type>::clear_selection()
{
    for (auto& cell : cells)
        cell->is_selected = false;
}

template <typename Cell_Type>
void List_Selector<Cell_Type>::List_Selector_Internal::resized()
{
    for (auto [idx, cell] : chowdsp::enumerate (parent->cells))
        cell->setBounds (0, (int) idx * cell_height, getWidth(), cell_height);
}

template struct List_Selector<Song_Cell>;
template struct List_Selector<Album_Cell>;
template struct List_Selector<Artist_Cell>;

//=============================================
template <typename Data_Type, typename Cell_Type>
void Cell_Base<Data_Type, Cell_Type>::paint (juce::Graphics& g)
{
    if (is_selected)
        g.fillAll (juce::Colours::dodgerblue.withAlpha (0.5f));

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawFittedText (chowdsp::toString (label_text), getLocalBounds(), juce::Justification::centredLeft, 1);
}

template <typename Data_Type, typename Cell_Type>
void Cell_Base<Data_Type, Cell_Type>::select_cell()
{
    if (is_selected)
        return;

    list->clear_selection();
    is_selected = true;
    list->repaint();
}

template <typename Data_Type, typename Cell_Type>
void Cell_Base<Data_Type, Cell_Type>::mouseDown (const juce::MouseEvent& e)
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

template <typename Data_Type, typename Cell_Type>
void Cell_Base<Data_Type, Cell_Type>::mouseDoubleClick (const juce::MouseEvent&)
{
    if (list->select_on_click)
        select_cell();

    cell_double_clicked (*data);
}

template struct Cell_Base<library::Song, Song_Cell>;
template struct Cell_Base<library::Album, Album_Cell>;
template struct Cell_Base<library::Artist, Artist_Cell>;
} // namespace chow_tunes::gui
