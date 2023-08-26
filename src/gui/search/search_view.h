#pragma once

#include "fuzzy_search_database.h"
#include "gui/library/library_view.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes::library
{
struct Music_Library;
}

namespace chow_tunes::gui
{
struct Search_View : juce::Component
{
    Search_View();

    void initialize_search_database (library::Music_Library&, Library_View&);

    void resized() override;
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

    juce::TextEditor search_entry;

    struct Search_Result : juce::Label
    {
        void paint (juce::Graphics& g) override
        {
            g.fillAll (juce::Colours::darkgrey);
            g.setColour (juce::Colours::slategrey);
            g.drawRect (getLocalBounds());

            g.setColour (juce::Colours::white);
            g.setFont (16.0f);
            g.drawFittedText (getText(), getLocalBounds(), juce::Justification::centred, 1);
        }

        void mouseUp (const juce::MouseEvent& e) override
        {
            onClick (e);
        }

        std::function<void (const juce::MouseEvent&)> onClick;
    };
    std::array<Search_Result, 10> results;

    fuzzysearch::Database<int> search_database;
};
} // namespace chow_tunes::gui
