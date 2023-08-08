#pragma once

#include <filesystem>
#include <string_view>
#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include "StackAllocator.h"

namespace chow_tunes::library
{
struct Song;
struct Album;

struct Artist
{
    std::string_view name {};
    chowdsp::SmallVector<size_t, 10> album_ids {};
    // std::span<Song> loose_songs {};
    // artwork
};

struct Album
{
    std::string_view name {};
    chowdsp::SmallVector<size_t, 20> song_ids {};
    size_t artist_id {};
    // artwork
};

struct Song
{
    std::string_view name {};
    size_t artist_id {};
    size_t album_id {};
    std::string_view filepath {};
    int track_number = -1; // starts indexing at 0
    // artwork
};

struct Music_Library
{
    chowdsp::StackAllocator stack_data;

    std::vector<Song> songs {};
    std::vector<Album> albums {};
    std::vector<Artist> artists {};
};

Music_Library index_directory (const std::filesystem::path& path);

std::string print_library (const Music_Library& library);
} // namespace chow_tunes::library
