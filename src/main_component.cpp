#include "main_component.h"

namespace chow_tunes
{
Main_Component::Main_Component()
{
    juce::Logger::writeToLog (chow_tunes::library::print_library (library));
    audio_format_manager.registerBasicFormats();
    juce::Logger::writeToLog ("Registered audio formats: " + audio_format_manager.getWildcardForAllFormats());

    addAndMakeVisible (library_view);
    addAndMakeVisible (transport_view);

    startTimer (100);
    setSize (1250, 750);
}

void Main_Component::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void Main_Component::resized()
{
    auto bounds = getLocalBounds();
    library_view.setBounds (bounds.removeFromTop (proportionOfHeight (0.8f)));
    transport_view.setBounds (bounds);
}

void Main_Component::timerCallback()
{
    audio::Audio_Player_Action action;
    while (audio_player.audio_to_ui_queue.try_dequeue (action))
    {
        if (action.action_type == audio::Audio_Player_Action_Type::Dead_Song)
        {
            action.audio_buffer.reset();
        }
        else if (action.action_type == audio::Audio_Player_Action_Type::Song_Finished)
        {
            action.audio_buffer.reset();
            // go to next song in play queue...
        }
    }
}
}
