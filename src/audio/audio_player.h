#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

#include "audio_player_actions.h"

namespace chow_tunes::audio
{
struct Audio_Player : juce::AudioIODeviceCallback
{
    Audio_Player();
    ~Audio_Player() override;

    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart (juce::AudioIODevice *device) override;
    void audioDeviceStopped() override;
    void audioDeviceError (const juce::String &errorMessage) override;

    std::unique_ptr<juce::AudioBuffer<float>> playing_buffer {};
    juce::AudioDeviceManager audio_device_manager;
    int sample_counter = 0;

    moodycamel::ReaderWriterQueue<Audio_Player_Action> ui_to_audio_queue { 32 };
    moodycamel::ReaderWriterQueue<Audio_Player_Action> audio_to_ui_queue { 32 };
};
} // namespace chow_tunes::audio
