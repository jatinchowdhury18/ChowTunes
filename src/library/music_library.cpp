#include "music_library.h"

#if JUCE_WINDOWS
#include <codecvt>
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmacro-redefined", "-Wdeprecated-declarations")
#include <fileref.h>
#include <tag.h>

namespace chow_tunes::library
{
static std::string_view to_string_view (const TagLib::String& str)
{
    return { str.toCString(), (size_t) str.size() };
}

template <typename IntType>
std::string_view temp_string (chowdsp::StackAllocator& alloc, const char* data, IntType count)
{
    auto* t_data = alloc.allocate<char> (count);
    std::copy (data, data + count, t_data);
    return { t_data, (size_t) count };
}

template <size_t N>
std::string_view temp_string (chowdsp::StackAllocator& alloc, const char (&str)[N])
{
    // don't copy the null terminator!
    if (str[N-1] == '\0')
        return temp_string (alloc, std::data (str), std::size (str) - 1);
    return temp_string (alloc, std::data (str), std::size (str));
}

inline std::string_view temp_string (chowdsp::StackAllocator& alloc, std::string_view str)
{
    return temp_string (alloc, std::data (str), std::size (str));
}

static std::string path_to_utf8 (const std::filesystem::path& path)
{
#if JUCE_WINDOWS
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.to_bytes (path.wstring());
#else
    return path.string();
#endif
}

static Artist& get_artist_for_song (Music_Library& library, Song& song, std::string_view artist_name)
{
    for (auto [idx, artist] : chowdsp::enumerate (library.artists))
    {
        if (artist.name == artist_name)
        {
            song.artist_id = idx;
            return artist;
        }
    }

    song.artist_id = library.artists.size();
    auto& song_artist = library.artists.emplace_back();
    song_artist.name = temp_string (library.stack_data, artist_name.data(), artist_name.size());
    return song_artist;
}

static Album& get_album_for_song (Music_Library& library, Song& song, std::string_view album_name, Artist& artist)
{
    for (auto album_id : artist.album_ids)
    {
        auto& album = library.albums[album_id];
        if (album.name == album_name)
        {
            song.album_id = album_id;
            return album;
        }
    }

    song.album_id = library.albums.size();
    auto& song_album = library.albums.emplace_back();
    song_album.name = temp_string (library.stack_data, album_name.data(), album_name.size());
    song_album.artist_id = song.artist_id;

    if (std::find (artist.album_ids.begin(), artist.album_ids.end(), song.album_id) == artist.album_ids.end())
        artist.album_ids.push_back (song.album_id);

    return song_album;
}

Music_Library index_directory (const std::filesystem::path& path)
{
    Music_Library library;
    library.stack_data.reset (1 << 21);
    library.artists.reserve (250);
    library.albums.reserve (500);
    library.songs.reserve (5000);

    std::printf ("Indexing directory: %s\n", path_to_utf8 (path).c_str());
    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator (path))
    {
        //        std::printf ("    Entry: %s\n", dir_entry.path().c_str());
        if (dir_entry.is_regular_file() && dir_entry.path().extension() == ".mp3")
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
            song.name = temp_string (library.stack_data, tag->title().toCString(), tag->title().size());

            auto& song_artist = get_artist_for_song (library, song, to_string_view (tag->artist()));
            auto& song_album = get_album_for_song (library, song, to_string_view (tag->album()), song_artist);

            song.track_number = static_cast<int> (tag->track());
            const auto filepath_str = path_to_utf8 (dir_entry.path());
            song.filepath = temp_string (library.stack_data, filepath_str.data(), filepath_str.size());
            song_album.song_ids.push_back (song_id);

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

std::string print_library (const Music_Library& library)
{
    auto& alloc = const_cast<chowdsp::StackAllocator&> (library.stack_data);
    chowdsp::StackAllocator::StackAllocatorFrame frame { alloc };

    size_t count = 0;
    const auto build_string = [&alloc, &count] (auto&&... args)
    {
        auto sv = temp_string (alloc, std::forward<decltype (args)...> (args...));
        count += sv.size();
    };

    for (const auto& artist : library.artists)
    {
        build_string (artist.name);
        build_string (":\n");

        for (auto album_id : artist.album_ids)
        {
            const auto& album = library.albums[album_id];
            build_string ("\t");
            build_string (album.name);
            build_string (":\n");

            for (auto song_id : album.song_ids)
            {
                const auto& song = library.songs[song_id];
                build_string ("\t\t");
                build_string (song.name);
                build_string (" | ");
                build_string (song.filepath);
                build_string ("\n");
            }
        }
    }

    const auto result = std::string { alloc.data<char> (frame.bytes_used_at_start), count };
    return result;
}
} // namespace chow_tunes::library

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
