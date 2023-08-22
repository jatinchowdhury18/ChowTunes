#pragma once

#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

namespace chow_tunes
{
struct Main_Component;
}

namespace chow_tunes::state
{
struct State : chowdsp::NonParamState
{
    State()
    {
        addStateValues ({ &library_filepath, &volume_db });
    }

    chowdsp::StateValue<std::string> library_filepath { "library_path", {} };
    chowdsp::StateValue<float> volume_db { "volume_db", -6.0f };

    void load_state (Main_Component& main_comp);
    void save_state() const;
    void select_library_folder();

    std::shared_ptr<juce::FileChooser> file_chooser;
};
}
