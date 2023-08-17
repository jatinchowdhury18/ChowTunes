#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes::gui
{
struct Transport_View : juce::Component
{
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::green);
    }
};
} // namespace chow_tunes::gui
