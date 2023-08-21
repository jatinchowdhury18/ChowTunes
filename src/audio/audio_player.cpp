#include "audio_player.h"

#include <chowdsp_buffers/chowdsp_buffers.h>
#include <chowdsp_math/chowdsp_math.h>
#include <juce_dsp/juce_dsp.h>

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
    handle_incoming_messages();

    auto out_buffer = chowdsp::BufferView<float> { outputChannelData, numOutputChannels, numSamples };
    if (playing_buffer == nullptr || state.load() != State::Playing)
    {
        out_buffer.clear();
        return;
    }

    const auto is_finished = read_samples (out_buffer);
    if (is_finished)
    {
        Audio_Player_Action action;
        action.action_type = Audio_Player_Action_Type::Song_Finished;
        action.audio_buffer.swap (playing_buffer);
        audio_to_ui_queue.enqueue (std::move (action));
    }

    if (playing_buffer == nullptr)
        state.store (State::Stopped);
}

void Audio_Player::handle_incoming_messages()
{
    Audio_Player_Action action;
    if (ui_to_audio_queue.try_dequeue (action))
    {
        if (action.action_type == Audio_Player_Action_Type::Start_New_Song)
        {
            song_sample_rate.store (std::get<double> (action.action_value));
            jassert (song_sample_rate.load() > 0.0);
            playing_buffer.swap (action.audio_buffer);
            leftover_samples.setCurrentSize (2, 0);

            action.action_type = Audio_Player_Action_Type::Dead_Song;
            audio_to_ui_queue.try_enqueue (std::move (action));

            sample_counter.store (0);
            song_length_samples.store (playing_buffer->getNumSamples());
            state.store (State::Playing);

            for (auto& resampler : resamplers)
                src_reset (resampler.get());
        }
        else if (action.action_type == Audio_Player_Action_Type::Move_Playhead)
        {
            const auto new_play_percent = std::get<double> (action.action_value);
            sample_counter.store (juce::roundToIntAccurate (new_play_percent * (double) song_length_samples.load()));
        }
    }

    // If this is false, then we will have a de-allocation here, which would be bad!!
    jassert (action.audio_buffer == nullptr); // NOLINT
}

bool Audio_Player::read_samples (const chowdsp::BufferView<float>& write_buffer) noexcept
{
    write_buffer.clear();

    const auto resample_ratio = device_sample_rate / song_sample_rate.load();
    int write_index = 0;

    const auto write_from_leftovers = [this, &write_buffer, &write_index]
    {
        jassert (leftover_samples.getNumSamples() > 0);

        const auto leftover_samples_to_copy = juce::jmin (leftover_samples.getNumSamples(), write_buffer.getNumSamples() - write_index);
        chowdsp::BufferMath::copyBufferData (leftover_samples, write_buffer, 0, write_index, leftover_samples_to_copy);
        write_index += leftover_samples_to_copy;

        const auto leftovers_remaining = leftover_samples.getNumSamples() - leftover_samples_to_copy;
        if (leftovers_remaining > 0)
            chowdsp::BufferMath::copyBufferData (leftover_samples, leftover_samples, leftover_samples_to_copy, 0, leftovers_remaining);
        leftover_samples.setCurrentSize (2, leftovers_remaining);
    };

    // write old data from leftover_samples
    if (write_index < write_buffer.getNumSamples() && leftover_samples.getNumSamples() > 0)
        write_from_leftovers();

    while (sample_counter.load() < playing_buffer->getNumSamples() && write_index < write_buffer.getNumSamples())
    {
        leftover_samples.setCurrentSize (2, small_block_size * (int) std::ceil (resample_ratio) + 1);
        const auto num_samples_to_read = juce::jmin (small_block_size, playing_buffer->getNumSamples() - sample_counter);

        int samples_pulled = 0;
        for (int ch = 0; ch < write_buffer.getNumChannels(); ++ch)
        {
            SRC_DATA src_data {
                .data_in = playing_buffer->getReadPointer (ch) + sample_counter.load(),
                .data_out = leftover_samples.getWritePointer (ch),
                .input_frames = num_samples_to_read,
                .output_frames = leftover_samples.getNumSamples(),
                .end_of_input = sample_counter.load() + num_samples_to_read == playing_buffer->getNumSamples() ? 1 : 0,
                .src_ratio = resample_ratio
            };

            src_process (resamplers[(size_t) ch].get(), &src_data);

            jassert (src_data.input_frames_used == num_samples_to_read);
            jassert (samples_pulled == 0 || samples_pulled == src_data.output_frames_gen);
            samples_pulled = (int) src_data.output_frames_gen;
        }

        if (samples_pulled > 0)
        {
            sample_counter.fetch_add (num_samples_to_read);
            leftover_samples.setCurrentSize (2, samples_pulled);
            write_from_leftovers();
        }
    }

    process_effects (write_buffer);

    return sample_counter.load() == playing_buffer->getNumSamples() && leftover_samples.getNumSamples() == 0;
}

void Audio_Player::process_effects (const chowdsp::BufferView<float>& buffer) noexcept
{
    // @TODO: add effects here:
    // - EQ
    // - safety limiter/clipper

    volume_gain.setGainLinear (juce::Decibels::decibelsToGain (volume_db.load(), min_gain_db));
    volume_gain.process (buffer);

    chowdsp::BufferMath::sanitizeBuffer (buffer, 5.0f);
}

double Audio_Player::get_song_progress_percent() const noexcept
{
    if (song_length_samples.load() == 0)
        return 0.0;
    return (double) sample_counter.load() / (double) song_length_samples.load();
}

size_t Audio_Player::get_seconds_played() const noexcept
{
    return (size_t) juce::roundToIntAccurate ((double) sample_counter.load() / song_sample_rate.load());
}

void Audio_Player::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    juce::Logger::writeToLog ("Audio device starting: " + device->getName()
                              + ", with sample rate: " + juce::String { device->getCurrentSampleRate() }
                              + ", and block size: " + juce::String { device->getCurrentBufferSizeSamples() });

    device_sample_rate = device->getCurrentSampleRate();

    const auto spec = juce::dsp::ProcessSpec {
        .sampleRate = device_sample_rate,
        .maximumBlockSize = (uint32_t) device->getCurrentBufferSizeSamples(),
        .numChannels = (uint32_t) device->getActiveOutputChannels().toInteger()
    };

    volume_gain.setRampDurationSeconds (0.05);
    volume_gain.setGainDecibels (juce::Decibels::decibelsToGain (volume_db.load(), min_gain_db));
    volume_gain.prepare (spec);
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
