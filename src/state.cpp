#include "state.h"
#include "main_component.h"

namespace chow_tunes::state
{
const auto state_save_path = juce::File { juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("ChowdhuryDSP/ChowTunes/app_state.json") };

void State::load_state (Main_Component& main)
{
    library_filepath.changeBroadcaster.connect (
        [this, &main]()
        {
            main.play_queue.clear_queue();
            main.library = library::index_directory (
                library_filepath.get(),
                [&main] (const library::Music_Library& library, bool is_loading_complete)
                {
                    juce::MessageManager::callAsync ([&main, &library, is_loading_complete]()
                    {
                        main.library_view.load_song_list ({}, library);
                        main.library_view.load_album_list ({}, library);
                        main.library_view.load_artist_list (library.artists, library);

                        if (is_loading_complete)
                        {
                            main.transport_view.library = main.library.get();
                            main.search_view.initialize_search_database (*main.library, main.library_view);
                        }
                    });
                });
        });
    volume_db.changeBroadcaster.connect (
        [this, &main]()
        {
            if (main.audio_player.has_value())
            {
                main.audio_player->volume_db.store (volume_db.get());
                main.transport_view.volume_slider.setValue ((double) volume_db.get(), juce::dontSendNotification);
            }
        });

    if (state_save_path.existsAsFile())
        chowdsp::Serialization::deserialize<chowdsp::JSONSerializer> (state_save_path, *this);
}

void State::save_state() const
{
    state_save_path.deleteFile();
    state_save_path.create();
    chowdsp::Serialization::serialize<chowdsp::JSONSerializer> (*this, state_save_path);
}

void State::select_library_folder()
{
    file_chooser = std::make_shared<juce::FileChooser> ("Choose library folder");
    file_chooser->launchAsync (juce::FileBrowserComponent::canSelectDirectories,
                               [this] (const juce::FileChooser& fc)
                               {
                                   if (fc.getResults().isEmpty())
                                       return;
                                   library_filepath = fc.getResult().getFullPathName().toStdString();
                               });
}
} // namespace chow_tunes::state
