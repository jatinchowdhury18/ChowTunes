#pragma once

#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <chowdsp_core/chowdsp_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace ffmpeg_reader
{
static auto read_file (const std::string& file_name)
{
    // Get a buffer for writing errors to
    static constexpr size_t errbuf_size = 256;
    char errbuf[errbuf_size];

    // Initialize variables
    AVCodecContext* codec_context = nullptr;
    AVFormatContext* format_context = nullptr;
    SwrContext* resample_context = nullptr;
    AVFrame* frame = nullptr;
    AVPacket* packet = nullptr;

    auto _ = chowdsp::runAtEndOfScope (
        [&]
        {
            avcodec_close (codec_context);
            avcodec_free_context (&codec_context);
            avio_closep (&format_context->pb);
            avformat_free_context (format_context);
            swr_free (&resample_context);
            av_frame_free (&frame);
            av_packet_free (&packet);
        });

    // Open the file and get format information
    int error = avformat_open_input (&format_context, file_name.c_str(), nullptr, 0);
    if (error != 0)
    {
        av_strerror (error, errbuf, errbuf_size);
        throw std::invalid_argument (
            "Could not open audio file: " + file_name + "\n" + "Error: " + std::string (errbuf));
    }

    // Get stream info
    if ((error = avformat_find_stream_info (format_context, nullptr)) < 0)
    {
        av_strerror (error, errbuf, errbuf_size);
        throw std::runtime_error (
            "Could not get information about the stream in file: " + file_name + "\n" + "Error: " + std::string (errbuf));
    }

    // Find an audio stream and its decoder
    AVCodec* codec = nullptr;
    int audio_stream_index = av_find_best_stream (
        format_context,
        AVMEDIA_TYPE_AUDIO,
        -1,
        -1,
        const_cast<const AVCodec**> (&codec),
        0);
    if (audio_stream_index < 0)
    {
        throw std::runtime_error (
            "Could not determine the best stream to use in the file: " + file_name);
    }

    // Allocate context for decoding the codec
    codec_context = avcodec_alloc_context3 (codec);
    if (! codec_context)
    {
        throw std::runtime_error (
            "Could not allocate a decoding context for file: " + file_name);
    }

    // Fill the codecContext with parameters of the codec
    if ((error = avcodec_parameters_to_context (
             codec_context,
             format_context->streams[audio_stream_index]->codecpar))
        != 0)
    {
        throw std::runtime_error (
            "Could not set codec context parameters for file: " + file_name);
    }

    // Initialize the decoder
    if (error = avcodec_open2 (codec_context, codec, nullptr); error != 0)
    {
        av_strerror (error, errbuf, errbuf_size);
        throw std::runtime_error (
            "Could not initialize the decoder for file: " + file_name + "\n" + "Error: " + std::string (errbuf));
    }

    // Make sure there is a channel layout
    if (codec_context->ch_layout.nb_channels == 0)
    {
        throw std::runtime_error (
            "Unable to determine channel layout for file: " + file_name);
    }

    // Fetch the sample rate
    const auto sample_rate = codec_context->sample_rate;
    if (sample_rate <= 0)
    {
        throw std::runtime_error (
            "File has an invalid sample rate " + std::to_string (sample_rate));
    }

    // Initialize a resampler
    // we're not actually going to resample the audio here
    // but we need this for doinga format conversion.
    swr_alloc_set_opts2 (&resample_context,
                         &codec_context->ch_layout,
                         AV_SAMPLE_FMT_FLTP,
                         sample_rate,
                         &codec_context->ch_layout,
                         codec_context->sample_fmt,
                         sample_rate,
                         0,
                         nullptr);
    if (! resample_context)
    {
        throw std::runtime_error (
            "Could not allocate resample context for file: " + file_name);
    }

    // Open the resampler context with the specified parameters
    if (error = swr_init (resample_context); error < 0)
    {
        throw std::runtime_error (
            "Could not open resample context for file: " + file_name);
    }

    // Initialize the input frame
    if (frame = av_frame_alloc(); frame == nullptr)
    {
        throw std::runtime_error (
            "Could not allocate audio frame for file: " + file_name);
    }

    // prepare a packet
    packet = av_packet_alloc();

    // Allocate the output buffer
    const auto length_samples = ((double) format_context->duration / (double) AV_TIME_BASE) * (double) sample_rate;
    juce::AudioBuffer<float> audio { codec_context->ch_layout.nb_channels,
                                     static_cast<int> (length_samples) };
    audio.clear();

    // Read the file until either nothing is left
    // or we reach desired end of sample
    int64_t sample_counter = 0;
    while (sample_counter < static_cast<int64_t> (length_samples))
    {
        // Read from the frame
        error = av_read_frame (format_context, packet);
        if (error == AVERROR_EOF)
        {
            break;
        }
        else if (error < 0)
        {
            av_strerror (error, errbuf, errbuf_size);
            throw std::runtime_error (
                "Error reading from file: " + file_name + "\n" + "Error: " + std::string (errbuf));
        }

        // Is this the correct stream?
        if (packet->stream_index != audio_stream_index)
        {
            // Otherwise move on
            continue;
        }

        // Send the packet to the decoder
        if ((error = avcodec_send_packet (codec_context, packet)) < 0)
        {
            av_strerror (error, errbuf, errbuf_size);
            throw std::runtime_error (
                "Could not send packet to decoder for file: " + file_name + "\n" + "Error: " + std::string (errbuf));
        }

        // Receive a decoded frame from the decoder
        while ((error = avcodec_receive_frame (codec_context, frame)) == 0)
        {
            // Send the frame to the resampler
            const auto out_frame = chowdsp::BufferView {
                audio,
                static_cast<int> (sample_counter),
                frame->nb_samples,
            };
            if ((error = swr_convert (resample_context,
                                      const_cast<uint8_t**> (reinterpret_cast<uint8_t* const*> (out_frame.getArrayOfWritePointers())),
                                      out_frame.getNumSamples(),
                                      const_cast<const uint8_t**> (frame->extended_data),
                                      frame->nb_samples))
                < 0)
            {
                av_strerror (error, errbuf, errbuf_size);
                throw std::runtime_error (
                    "Could not resample frame for file: " + file_name + "\n" + "Error: " + std::string (errbuf));
            }

            // // Send the frame to the resampler
            // const auto frame_data = reinterpret_cast<double**> (frame->extended_data);
            //
            // // Update the frame
            // for (int channel = 0; channel < audio.getNumChannels(); ++channel)
            // {
            //     for (int n = 0; n < frame->nb_samples; ++n)
            //     {
            //         const auto sample = frame_data[channel][n]; //[audio.getNumChannels() * n + channel];
            //         jassert (! std::isnan (sample));
            //         // audio_data[channel][sample_counter + n] = static_cast<float> (sample);
            //     }
            // }

            // Increment the stamp
            sample_counter += frame->nb_samples;
        }

        // Check if the decoder had any errors
        if (error != AVERROR (EAGAIN))
        {
            av_strerror (error, errbuf, errbuf_size);
            throw std::runtime_error (
                "Error receiving packet from decoder for file: " + file_name + "\n" + "Error: " + std::string (errbuf));
        }
    }

    return std::make_tuple (audio, codec_context->sample_rate);
}
} // namespace ffmpeg_reader
