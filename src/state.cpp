#include "state.h"
#include "main_component.h"

namespace chow_tunes::state
{
const auto state_save_path = juce::File { juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("ChowdhuryDSP/ChowTunes/app_state.json") };

void State::load_state (Main_Component& main)
{
    library_filepath.changeBroadcaster.connect (
        [this, &main] ()
        {
            main.library = library::index_directory (library_filepath.get());

            main.library_view.load_song_list ({}, main.library);
            main.library_view.load_album_list ({}, main.library);
            main.library_view.load_artist_list (main.library.artists, main.library);
        });
    volume_db.changeBroadcaster.connect (
        [this, &main] ()
        {
            main.audio_player.volume_db.store (volume_db.get());
            main.transport_view.volume_slider.setValue ((double) volume_db.get(), juce::dontSendNotification);
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
}
