#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include <chowdsp_sources/chowdsp_sources.h>

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

    void handle_incoming_messages();
    bool read_samples (const chowdsp::BufferView<float>& write_buffer) noexcept;
    void process_effects (const chowdsp::BufferView<float>& buffer) noexcept;

    std::unique_ptr<juce::AudioBuffer<float>> playing_buffer {};
    double song_sample_rate = 48000.0;
    int sample_counter = 0;

    juce::AudioDeviceManager audio_device_manager;
    double device_sample_rate = 48000.0;

    moodycamel::ReaderWriterQueue<Audio_Player_Action> ui_to_audio_queue { 32 };
    moodycamel::ReaderWriterQueue<Audio_Player_Action> audio_to_ui_queue { 32 };

    using SRCResamplerPtr = std::unique_ptr<SRC_STATE, decltype (&src_delete)>;
    int src_error {};
    std::array<SRCResamplerPtr, 2> resamplers {
        SRCResamplerPtr { src_new (SRC_SINC_BEST_QUALITY, 1, &src_error), &src_delete },
        SRCResamplerPtr{ src_new (SRC_SINC_BEST_QUALITY, 1, &src_error), &src_delete }
    };

    static constexpr auto small_block_size = 64;
    chowdsp::StaticBuffer<float, 2, 4 * small_block_size> leftover_samples { 2, 4 * small_block_size };
};
} // namespace chow_tunes::audio
