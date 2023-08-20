#include "main_component.h"

namespace chow_tunes
{
Main_Component::Main_Component()
{
//    juce::Logger::writeToLog (chow_tunes::library::print_library (library));
    audio_format_manager.registerBasicFormats();
    juce::Logger::writeToLog ("Registered audio formats: " + audio_format_manager.getWildcardForAllFormats());

    play_queue.action_router = &action_router;

    addAndMakeVisible (library_view);
    addAndMakeVisible (transport_view);
    addAndMakeVisible (play_queue_view);

    startTimer (5);
    setSize (1250, 750);
}

void Main_Component::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void Main_Component::resized()
{
    auto bounds = getLocalBounds();
    transport_view.setBounds (bounds.removeFromBottom (proportionOfHeight (0.2f)));
    play_queue_view.setBounds (bounds.removeFromRight (proportionOfWidth (0.3f)));
    library_view.setBounds (bounds);
}

void Main_Component::timerCallback()
{
    audio::Audio_Player_Action action;
    while (audio_player.audio_to_ui_queue.try_dequeue (action))
        action_router.route_action (std::move (action));
}
}
