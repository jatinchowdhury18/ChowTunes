#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "audio/audio_player.h"
#include "gui/library/library_view.h"
#include "gui/library/list_selector.h"
#include "gui/play_queue_view.h"
#include "gui/transport_view.h"
#include "library/music_library.h"
#include "play_queue/play_queue.h"

namespace chow_tunes
{
struct Main_Component : juce::Component,
                        juce::Timer
{
    Main_Component();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
#if JUCE_MAC
    static constexpr std::string_view library_path { "/Users/jatin/test_music" };
#elif JUCE_WINDOWS
#if JUCE_DEBUG
//    static constexpr std::string_view library_path { "C:/Users/Jatin/test_music" };
    static constexpr std::string_view library_path { "C:/Users/Jatin/Music" };
#else
    static constexpr std::string_view library_path { "C:/Users/Jatin/Music" };
#endif
#endif

    chowdsp::SharedAudioFileSaveLoadHelper audio_file_helper;
    juce::AudioFormatManager audio_format_manager;
    library::Music_Library library = library::index_directory (library_path);
    audio::Audio_Player audio_player;
    play_queue::Play_Queue play_queue;
    audio::Audio_Player_Action_Router action_router { .audio_player = audio_player, .play_queue = play_queue };
    gui::Library_View library_view { library, play_queue };
    gui::Transport_View transport_view { action_router };
    gui::Play_Queue_View play_queue_view { play_queue };
};
} // namespace chow_tunes
