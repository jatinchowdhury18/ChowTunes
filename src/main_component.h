#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace chow_tunes
{
struct Main_Component : juce::Component
{
    Main_Component()
    {
        setSize (400, 400);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::red);
    }
};
}
