#pragma once

#include <filesystem>
#include <string_view>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

namespace chow_tunes::library
{
struct Song;
struct Album;

struct Artist
{
    std::u8string_view name {};
    chowdsp::SmallVector<size_t, 10> album_ids {};
    // std::span<Song> loose_songs {};
};

struct Album
{
    std::u8string_view name {};
    chowdsp::SmallVector<size_t, 20> song_ids {};
    size_t artist_id {};
    size_t year = 0;
};

struct Song
{
    std::u8string_view name {};
    size_t artist_id {};
    size_t album_id {};
    std::u8string_view filepath {};
    int track_number = -1; // starts indexing at 0, -1 is "invalid"
    std::u8string_view artwork_file {};
    int track_length_seconds = 0;
};

struct Music_Library
{
    mutable chowdsp::ArenaAllocator<> stack_data;

    std::vector<Song> songs {};
    std::vector<Album> albums {};
    std::vector<Artist> artists {};
};

using Update_Callback = std::function<void(const Music_Library&, bool is_loading_complete)>;
std::shared_ptr<Music_Library> index_directory (const std::filesystem::path& path, const Update_Callback& callback = {});

std::string print_library (const Music_Library& library);
} // namespace chow_tunes::library
