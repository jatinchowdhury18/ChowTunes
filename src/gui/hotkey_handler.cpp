#include "hotkey_handler.h"
#include "main_component.h"

#if JUCE_WINDOWS
#include <Windows.h>
#endif

namespace chow_tunes::gui
{
#if JUCE_WINDOWS
enum
{
    PLAY_PAUSE = 1,
    PREVIOUS_SONG,
    NEXT_SONG,
};

void Hotkey_Handler::register_hotkeys() const
{
    RegisterHotKey ((HWND) main_comp->getWindowHandle(), PLAY_PAUSE, 0, VK_MEDIA_PLAY_PAUSE);
    RegisterHotKey ((HWND) main_comp->getWindowHandle(), PREVIOUS_SONG, 0, VK_MEDIA_PREV_TRACK);
    RegisterHotKey ((HWND) main_comp->getWindowHandle(), NEXT_SONG, 0, VK_MEDIA_NEXT_TRACK);
}

void Hotkey_Handler::handle_hotkey_callback (uint64_t key_id) const
{
    using Action_Type = audio::Audio_Player_Action_Type;
    if (key_id == PLAY_PAUSE)
    {
        const auto play_state = main_comp->audio_player.state.load();
        if (play_state == audio::Audio_Player::State::Playing)
        {
            main_comp->action_router.route_action ({ .action_type = Action_Type::Pause_Song });
        }
        else if (play_state == audio::Audio_Player::State::Paused)
        {
            main_comp->action_router.route_action ({ .action_type = Action_Type::Play_Song });
        }
    }
    else if (key_id == PREVIOUS_SONG)
    {
        main_comp->action_router.route_action ({ .action_type = Action_Type::Previous_Song });
    }
    else if (key_id == NEXT_SONG)
    {
        main_comp->action_router.route_action ({ .action_type = Action_Type::Next_Song });
    }
}
#endif
} // namespace chow_tunes::gui
