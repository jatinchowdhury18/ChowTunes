#pragma once

#include "gui/library/library_view.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <chowdsp_fuzzy_search/chowdsp_fuzzy_search.h>

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
        void paint (juce::Graphics& g) override;
        void mouseUp (const juce::MouseEvent& e) override;
        std::function<void (const juce::MouseEvent&)> onClick;
    };
    std::array<Search_Result, 10> results;

    enum class Search_Result_Type
    {
        Artist,
        Album,
    };
    chowdsp::SearchDatabase<std::pair<Search_Result_Type, int>> search_database;
};
} // namespace chow_tunes::gui
