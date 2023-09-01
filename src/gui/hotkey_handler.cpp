#include "hotkey_handler.h"
#include "main_component.h"

#if JUCE_WINDOWS
#include <Windows.h>
#endif

namespace chow_tunes::gui
{
#if JUCE_WINDOWS
void Hotkey_Handler::register_hotkeys() // NOLINT
{
    RegisterHotKey ((HWND) main_comp->getWindowHandle(), Hotkey_Action::PLAY_PAUSE, 0, VK_MEDIA_PLAY_PAUSE);
    RegisterHotKey ((HWND) main_comp->getWindowHandle(), Hotkey_Action::PREVIOUS_SONG, 0, VK_MEDIA_PREV_TRACK);
    RegisterHotKey ((HWND) main_comp->getWindowHandle(), Hotkey_Action::NEXT_SONG, 0, VK_MEDIA_NEXT_TRACK);
}
#elif JUCE_MAC
static OSStatus event_handler (EventHandlerCallRef, EventRef in_event, void* in_user_data)
{
    if (GetEventClass (in_event) == kEventClassKeyboard && GetEventKind (in_event) == kEventHotKeyPressed)
    {
        EventHotKeyID hotkey_id;
        GetEventParameter (in_event,
                           kEventParamDirectObject,
                           typeEventHotKeyID,
                           nullptr,
                           sizeof (EventHotKeyID),
                           nullptr,
                           &hotkey_id);
        static_cast<Hotkey_Handler*> (in_user_data)->handle_hotkey_callback (hotkey_id.id);
    }

    return noErr;
}

void Hotkey_Handler::register_hotkeys()
{
    EventTypeSpec press_event_spec;
    press_event_spec.eventClass = kEventClassKeyboard;
    press_event_spec.eventKind = kEventHotKeyPressed;
    InstallApplicationEventHandler (&event_handler, 1, &press_event_spec, this, nullptr);

    EventHotKeyID hotkey_id;
    EventHotKeyRef event_ref {};
    hotkey_id.id = Hotkey_Action::PLAY_PAUSE;
    RegisterEventHotKey (kVK_F8, 0, hotkey_id, GetApplicationEventTarget(), 0, &event_ref);

    hotkey_id.id = Hotkey_Action::PREVIOUS_SONG;
    RegisterEventHotKey (kVK_F7, 0, hotkey_id, GetApplicationEventTarget(), 0, &event_ref);

    hotkey_id.id = Hotkey_Action::NEXT_SONG;
    RegisterEventHotKey (kVK_F9, 0, hotkey_id, GetApplicationEventTarget(), 0, &event_ref);
}
#endif

void Hotkey_Handler::handle_hotkey_callback (uint64_t key_id) const
{
    using Action_Type = audio::Audio_Player_Action_Type;
    using Play_State = audio::Audio_Player::State;
    if (key_id == Hotkey_Action::PLAY_PAUSE)
    {
        const auto play_state = main_comp->audio_player.has_value() ? main_comp->audio_player->state.load() : Play_State::Stopped;
        if (play_state == Play_State::Playing)
        {
            main_comp->action_router.route_action ({ .action_type = Action_Type::Pause_Song });
        }
        else if (play_state == Play_State::Paused)
        {
            main_comp->action_router.route_action ({ .action_type = Action_Type::Play_Song });
        }
    }
    else if (key_id == Hotkey_Action::PREVIOUS_SONG)
    {
        main_comp->action_router.route_action ({ .action_type = Action_Type::Previous_Song });
    }
    else if (key_id == Hotkey_Action::NEXT_SONG)
    {
        main_comp->action_router.route_action ({ .action_type = Action_Type::Next_Song });
    }
}
} // namespace chow_tunes::gui
