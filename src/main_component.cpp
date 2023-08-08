#include "main_component.h"

namespace chow_tunes
{
Main_Component::Main_Component()
{
    juce::Logger::writeToLog (chow_tunes::library::print_library (library));
    audio_format_manager.registerBasicFormats();
    juce::Logger::writeToLog ("Registered audio formats: " + audio_format_manager.getWildcardForAllFormats());

    addAndMakeVisible (song_list);
    for (const auto& song : library.songs)
    {
        auto& new_cell = song_list.cells.emplace_back (std::make_unique<gui::Song_Cell>());
        new_cell->list = &song_list;
        new_cell->song = &song;
        new_cell->play_song_callback = [this] (std::string_view filepath)
        {
            auto [buffer, fs] = audio_file_helper->loadFile (juce::File { chowdsp::toString (filepath) });
            if (buffer.getNumSamples() == 0)
            {
                jassertfalse;
                return;
            }

            audio::Audio_Player_Action action;
            action.action_type = audio::Audio_Player_Action_Type::Start_New_Song;
            action.audio_buffer = std::make_unique<juce::AudioBuffer<float>> (std::move (buffer));
            audio_player.ui_to_audio_queue.enqueue (std::move (action));
        };
        song_list.addAndMakeVisible (new_cell.get());
    }

    startTimer (100);
    setSize (1250, 750);
}

void Main_Component::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void Main_Component::resized()
{
    song_list.setBounds (0, 0, 300, getHeight());
}

void Main_Component::timerCallback()
{
    audio::Audio_Player_Action action;
    while (audio_player.audio_to_ui_queue.try_dequeue (action))
    {
        if (action.action_type == audio::Audio_Player_Action_Type::Dead_Song)
            action.audio_buffer.reset();
    }
}
}
