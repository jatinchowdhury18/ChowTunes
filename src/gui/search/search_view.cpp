#include <chowdsp_data_structures/chowdsp_data_structures.h>

#include "library/music_library.h"
#include "search_view.h"

namespace chow_tunes::gui
{
void Search_View::Search_Result::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
    g.setColour (juce::Colours::slategrey);
    g.drawRect (getLocalBounds());

    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    g.drawFittedText (getText(), getLocalBounds(), juce::Justification::centred, 1);
}

void Search_View::Search_Result::mouseUp (const juce::MouseEvent& e)
{
    onClick (e);
}

//=================================================
template <typename List_Selector>
static void select_cell (List_Selector& list,
                         std::u8string_view name,
                         const juce::MouseEvent& e)
{
    for (auto cell : list.cell_entries)
    {
        if (cell.data->name == name)
        {
            cell.component->mouseDown (e);
            list.setViewPosition (cell.component->getBoundsInParent().getTopLeft());
            break;
        }
    }
}

Search_View::Search_View()
{
    search_entry.setWantsKeyboardFocus (true);
    search_entry.setSelectAllWhenFocused (true);
    search_entry.setMultiLine (false);
    search_entry.setFont (18.0f);
    search_entry.setJustification (juce::Justification::centred);
    search_entry.onEscapeKey = [this]
    { setVisible (false); };
    addAndMakeVisible (search_entry);

    for (auto& result_label : results)
    {
        result_label.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
        result_label.setText ("Result", juce::dontSendNotification);
        addChildComponent (result_label);
    }
}

void Search_View::initialize_search_database (library::Music_Library& library, Library_View& library_view)
{
    const auto num_entries = library.artists.size() + library.albums.size();
    search_database.resetEntries (num_entries, num_entries * 5);
    search_database.setThreshold (0.5f);

    namespace chrono = std::chrono;
    for (const auto& [idx, artist] : chowdsp::enumerate (library.artists))
        search_database.addEntry (std::make_pair (Search_Result_Type::Artist, (int) idx),
                                  { std::string_view { reinterpret_cast<const char*> (artist.name.data()), artist.name.size() } });
    for (const auto& [idx, album] : chowdsp::enumerate (library.albums))
        search_database.addEntry (std::make_pair (Search_Result_Type::Album, (int) idx),
                                  { std::string_view { reinterpret_cast<const char*> (album.name.data()), album.name.size() } });
    search_database.prepareForSearch();

    search_entry.onTextChange = [this, &library, &library_view]
    {
        const auto search_text = search_entry.getText();
        const auto search_results = search_database.search ({ search_text.toRawUTF8(), search_text.getNumBytesAsUTF8() });

        for (auto [result, label] : chowdsp::zip (search_results, results))
        {
            const auto [result_type, result_id] = result.key;
            if (result_type == Search_Result_Type::Artist)
            {
                const auto artist_name = library.artists[(size_t) result_id].name;
                const auto text_str = juce::String::fromUTF8 ((const char*) artist_name.data(), (int) artist_name.size());
                label.setText (text_str, juce::dontSendNotification);
                label.setVisible (true);
                label.onClick = [this, &library_view, artist_name] (const juce::MouseEvent& e)
                {
                    select_cell (library_view.artist_list, artist_name, e);
                    setVisible (false);
                };
            }
            else if (result_type == Search_Result_Type::Album)
            {
                const auto& album = library.albums[(size_t) result_id];
                const auto album_name = album.name;
                const auto artist_name = library.artists[album.artist_id].name;
                const auto text_str = juce::String::fromUTF8 ((const char*) album_name.data(), (int) album_name.size());
                label.setText (text_str, juce::dontSendNotification);
                label.setVisible (true);
                label.onClick = [this, &library_view, album_name, artist_name] (const juce::MouseEvent& e)
                {
                    select_cell (library_view.artist_list, artist_name, e);
                    select_cell (library_view.album_list, album_name, e);
                    setVisible (false);
                };
            }
        }

        for (size_t i = search_results.size(); i < results.size(); ++i)
            results[i].setVisible (false);
    };
}

void Search_View::resized()
{
    auto bounds = getLocalBounds();
    search_entry.setBounds (bounds.reduced (getWidth() / 2 - 300, getHeight() / 2 - 20).translated (0, -getHeight() / 3));
    for (auto [idx, result_label] : chowdsp::enumerate (results))
        result_label.setBounds (search_entry.getBoundsInParent().translated (0, (int) (idx + 1) * search_entry.getHeight()));
}

void Search_View::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.5f));
}

void Search_View::mouseDown (const juce::MouseEvent&)
{
    setVisible (false);
}
} // namespace chow_tunes::gui
