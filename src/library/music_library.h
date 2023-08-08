#pragma once

#include <filesystem>
#include <string_view>
#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <tag.h>
#include <fileref.h>
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

inline Music_Library index_directory (const std::filesystem::path& path)
{
    Music_Library library;
    library.stack_data.reset (1 << 20);

    std::printf ("Indexing directory: %s\n", path.c_str());
    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator (path))
    {
        std::printf ("    Entry: %s\n", dir_entry.path().c_str());
        if (dir_entry.is_regular_file() && dir_entry.path().extension() ==  ".mp3")
        {
            TagLib::FileRef file { dir_entry.path().c_str() };
            const auto* tag = file.tag();
            if (tag == nullptr)
            {
                jassertfalse;
                continue;
            }

            const auto song_id = library.songs.size();
            auto& song = library.songs.emplace_back();
            song.name = library.stack_data.temp_string (tag->title().toCString(), tag->title().size());

            Artist* song_artist = nullptr;
            for (auto [idx, artist] : chowdsp::enumerate (library.artists))
            {
                if (std::strncmp (artist.name.data(),
                                  tag->artist().toCString(),
                                  std::min (artist.name.size(), (size_t) tag->artist().size())) == 0)
                {
                    song_artist = &artist;
                    song.artist_id = idx;
                    break;
                }
            }

            if (song_artist == nullptr)
            {
                song.artist_id = library.artists.size();
                song_artist = &library.artists.emplace_back();
                song_artist->name = library.stack_data.temp_string (tag->artist().toCString(), tag->artist().size());
            }

            Album* song_album = nullptr;
            for (auto [idx, album] : chowdsp::enumerate (library.albums))
            {
                if (std::strncmp (album.name.data(),
                                  tag->album().toCString(),
                                  std::min (album.name.size(), (size_t) tag->album().size())) == 0
                    && album.artist_id == song.artist_id)
                {
                    song_album = &album;
                    song.album_id = idx;
                    break;
                }
            }

            if (song_album == nullptr)
            {
                song.album_id = library.albums.size();
                song_album = &library.albums.emplace_back();
                song_album->name = library.stack_data.temp_string (tag->album().toCString(), tag->artist().size());
                song_album->artist_id = song.artist_id;

                if (std::find (song_artist->album_ids.begin(), song_artist->album_ids.end(), song.album_id) == song_artist->album_ids.end())
                    song_artist->album_ids.push_back (song.album_id);
            }

            song.track_number = static_cast<int> (tag->track());
            song.filepath = library.stack_data.temp_string (dir_entry.path().c_str(), dir_entry.path().string().size());
            song_album->song_ids.push_back (song_id);

//            std::cout << "-- TAG (basic) --" << std::endl;
//            std::cout << "title   - \"" << tag->title()   << "\"" << std::endl;
//            std::cout << "artist  - \"" << tag->artist()  << "\"" << std::endl;
//            std::cout << "album   - \"" << tag->album()   << "\"" << std::endl;
//            std::cout << "year    - \"" << tag->year()    << "\"" << std::endl;
//            std::cout << "comment - \"" << tag->comment() << "\"" << std::endl;
//            std::cout << "track   - \"" << tag->track()   << "\"" << std::endl;
//            std::cout << "genre   - \"" << tag->genre()   << "\"" << std::endl;
        }
    }

    return library;
}
} // namespace chow_tunes::library
