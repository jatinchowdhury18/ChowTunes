#include <chowdsp_data_structures/chowdsp_data_structures.h>

#include "search_view.h"
#include "library/music_library.h"

namespace chow_tunes::gui
{
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
    search_database.reset();
    search_database.setThreshold (0.5f);

    for (auto [idx, artist] : chowdsp::enumerate (library.artists))
        search_database.addEntry ((int) idx, { std::u8string { artist.name } });

    search_entry.onTextChange = [this, &library, &library_view]
    {
        const auto search_text = search_entry.getText();
        const auto search_results = search_database.search (std::u8string { reinterpret_cast<const char8_t*> (search_text.toRawUTF8()) });

        for (auto [result, label] : chowdsp::zip (search_results, results))
        {
            const auto artist_name = library.artists[(size_t) result.key].name;
            const auto text_str = juce::String::fromUTF8 ((const char*) artist_name.data(), (int) artist_name.size());
            label.setText (text_str, juce::dontSendNotification);
            label.setVisible (true);
            label.onClick = [this, &library_view, artist_name] (const juce::MouseEvent& e)
            {
                for (auto cell : library_view.artist_list.cell_entries)
                {
                    if (cell.data->name == artist_name)
                    {
                        auto& cell_component = *library_view.artist_list.cell_components.find (cell.component_locator);
                        cell_component->mouseDown (e);
                        library_view.artist_list.setViewPosition (cell_component->getBoundsInParent().getTopLeft());
                        break;
                    }
                }
                setVisible (false);
            };
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
