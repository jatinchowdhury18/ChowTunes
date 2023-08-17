#include "list_selector.h"

namespace chow_tunes::gui
{
static constexpr auto cell_height = 30;

template <typename Cell_Type>
List_Selector<Cell_Type>::List_Selector()
{
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
static void paint_cell (juce::Graphics& g,
                        bool is_selected,
                        const juce::String& text,
                        const juce::Rectangle<int>& bounds)
{
    if (is_selected)
        g.fillAll (juce::Colours::dodgerblue.withAlpha (0.5f));

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawFittedText (text, bounds, juce::Justification::centredLeft, 1);
}

template <typename Cell_Type>
static bool select_cell (Cell_Type& cell)
{
    if (cell.is_selected)
        return false;

    cell.list->clear_selection();
    cell.is_selected = true;
    cell.list->repaint();
    return true;
}

void Song_Cell::paint (juce::Graphics& g)
{
    paint_cell (g, is_selected, chowdsp::toString (song->name), getLocalBounds());
}

void Song_Cell::mouseDown (const juce::MouseEvent&)
{
    if (select_cell (*this))
        song_selected_callback (*song);
}

void Album_Cell::paint (juce::Graphics& g)
{
    paint_cell (g, is_selected, chowdsp::toString (album->name), getLocalBounds());
}

void Album_Cell::mouseDown (const juce::MouseEvent&)
{
    if (select_cell (*this))
        album_selected_callback (*album);
}

void Artist_Cell::paint (juce::Graphics& g)
{
    paint_cell (g, is_selected, chowdsp::toString (artist->name), getLocalBounds());
}

void Artist_Cell::mouseDown (const juce::MouseEvent&)
{
    if (select_cell (*this))
        artist_selected_callback (*artist);
}
} // namespace chow_tunes::gui
