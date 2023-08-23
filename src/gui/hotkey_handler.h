#pragma once

#include <cstdint>

namespace chow_tunes
{
struct Main_Component;
}

namespace chow_tunes::gui
{
struct Hotkey_Handler
{
    void register_hotkeys() const;
    void handle_hotkey_callback (uint64_t key_id) const;

    Main_Component* main_comp = nullptr;
};
}
