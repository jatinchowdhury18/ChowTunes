#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "audio/audio_player.h"
#include "gui/hotkey_handler.h"
#include "gui/library/library_view.h"
#include "gui/library/list_selector.h"
#include "gui/play_queue_view.h"
#include "gui/search/search_view.h"
#include "gui/transport/transport_view.h"
#include "library/music_library.h"
#include "play_queue/play_queue.h"
#include "state.h"

namespace chow_tunes
{
struct Main_Component : juce::Component,
                        juce::Timer
{
    Main_Component();
    ~Main_Component() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;
    void timerCallback() override;

    chowdsp::SharedAudioFileSaveLoadHelper audio_file_helper;
    juce::AudioFormatManager audio_format_manager;

    std::once_flag load_state_flag {};
    state::State app_state;

    std::shared_ptr<library::Music_Library> library {};
    std::optional<audio::Audio_Player> audio_player { std::in_place };
    play_queue::Play_Queue play_queue;
    audio::Audio_Player_Action_Router action_router { .audio_player = audio_player, .play_queue = play_queue };

    gui::Library_View library_view { play_queue };
    gui::Transport_View transport_view { app_state, action_router };
    gui::Play_Queue_View play_queue_view { play_queue };
    gui::Hotkey_Handler hotkey_handler;
    gui::Search_View search_view;
};
} // namespace chow_tunes
