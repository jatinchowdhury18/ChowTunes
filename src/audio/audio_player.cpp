#include "audio_player.h"

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_simd/chowdsp_simd.h>
#include <chowdsp_math/chowdsp_math.h>
#include <chowdsp_buffers/chowdsp_buffers.h>

namespace chow_tunes::audio
{
Audio_Player::Audio_Player()
{
    audio_device_manager.initialiseWithDefaultDevices (0, 2);
    audio_device_manager.addAudioCallback (this);
}

Audio_Player::~Audio_Player()
{
    audio_device_manager.removeAudioCallback (this);
}

void Audio_Player::audioDeviceIOCallbackWithContext ([[maybe_unused]] const float* const* inputChannelData,
                                                     [[maybe_unused]] int numInputChannels,
                                                     float* const* outputChannelData,
                                                     int numOutputChannels,
                                                     int numSamples,
                                                     [[maybe_unused]] const juce::AudioIODeviceCallbackContext& context)
{
    chowdsp::BufferView<float> buffer { outputChannelData, numOutputChannels, numSamples };
    buffer.clear();

    Audio_Player_Action action;
    if (ui_to_audio_queue.try_dequeue (action))
    {
        if (action.action_type == Audio_Player_Action_Type::Start_New_Song)
        {
            playing_buffer.swap (action.audio_buffer);
            action.action_type = Audio_Player_Action_Type::Dead_Song;
            audio_to_ui_queue.try_dequeue (action);
        }
    }

    if (playing_buffer == nullptr)
        return;

    const auto num_samples = buffer.getNumSamples();
    chowdsp::BufferMath::copyBufferData (*playing_buffer, buffer, sample_counter, 0, num_samples);
    sample_counter += num_samples;
}

void Audio_Player::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    juce::Logger::writeToLog ("Audio device starting: " + device->getName());
}

void Audio_Player::audioDeviceStopped()
{
    juce::Logger::writeToLog ("Audio device stopping");
}

void Audio_Player::audioDeviceError (const juce::String& errorMessage)
{
    juce::Logger::writeToLog ("Audio device error: " + errorMessage);
}
} // namespace chow_tunes::audio
