#pragma once

#include <cstdint>
#include <juce_core/juce_core.h>

#if JUCE_MAC
#include <Carbon/Carbon.h>
#endif

namespace chow_tunes
{
struct Main_Component;
}

namespace chow_tunes::gui
{
struct Hotkey_Handler
{
    void register_hotkeys();
    void handle_hotkey_callback (uint64_t key_id) const;

    Main_Component* main_comp = nullptr;

#if JUCE_MAC
private:
#endif
};
}
