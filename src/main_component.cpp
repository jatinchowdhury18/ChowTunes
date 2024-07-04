#include "main_component.h"
#include <chowdsp_logging/chowdsp_logging.h>

namespace chow_tunes
{
Main_Component::Main_Component()
{
    audio_player.emplace();

    audio_format_manager.registerBasicFormats();
    chowdsp::log ("Registered audio formats: {}", audio_format_manager.getWildcardForAllFormats());

    hotkey_handler.main_comp = this;
    hotkey_handler.register_hotkeys();

    play_queue.action_router = &action_router;

    app_state.load_state (*this);
    if (app_state.library_filepath.get().empty())
        app_state.select_library_folder();

    addAndMakeVisible (library_view);
    addAndMakeVisible (transport_view);
    addAndMakeVisible (play_queue_view);
    addChildComponent (search_view);
    addChildComponent (library_view.metadata_editor.get());

    startTimerHz (25);
    setSize (1250, 750);
}

Main_Component::~Main_Component()
{
    app_state.save_state();
}

void Main_Component::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void Main_Component::resized()
{
    auto bounds = getLocalBounds();
    search_view.setBounds (bounds);
    transport_view.setBounds (bounds.removeFromBottom (proportionOfHeight (0.2f)));
    play_queue_view.setBounds (bounds.removeFromRight (proportionOfWidth (0.3f)));
    library_view.setBounds (bounds);
    library_view.metadata_editor->setBounds (getLocalBounds());
}

void Main_Component::timerCallback()
{
    if (audio_player.has_value())
    {
        audio::Audio_Player_Action action;
        while (audio_player->audio_to_ui_queue.try_dequeue (action))
            action_router.route_action (std::move (action));
    }

    transport_view.timeline.update();
}

bool Main_Component::keyPressed (const juce::KeyPress& key)
{
    const auto character = key.getTextCharacter();
    if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z'))
    {
        search_view.setVisible (true);
        search_view.search_entry.clear();
#if JUCE_WINDOWS
        search_view.search_entry.setText (juce::String { (const wchar_t*) &character, 1 });
#else
        search_view.search_entry.setText (juce::String { &character, 1 });
#endif
        search_view.search_entry.grabKeyboardFocus();
        search_view.search_entry.setCaretPosition (1);
        return true;
    }

    return false;
}
} // namespace chow_tunes
