#include "music_library.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmacro-redefined", "-Wdeprecated-declarations", "-Wdeprecated-dynamic-exception-spec")
#include <fileref.h>
#include <utf8-cpp/checked.h>

#include "bs_thread_pool.h"

namespace chow_tunes::library
{
static std::u8string_view to_u8string_view (chowdsp::StackAllocator& alloc, const TagLib::String& str)
{
    const auto str_view_start = alloc.data<char8_t> (alloc.get_bytes_used());
    char8_t* str_view_end = nullptr;
    try
    {
        str_view_end = utf8::utf16to8 (str.begin(), str.end(), str_view_start);
    }
    catch ([[maybe_unused]] const utf8::exception& e)
    {
        DBG (e.what());
        return {};
    }

    const auto str_view_length = std::distance (str_view_start, str_view_end);
    [[maybe_unused]] const auto start_ptr = alloc.allocate<char8_t> (str_view_length);
    jassert (start_ptr != nullptr);
    return { str_view_start, static_cast<size_t> (str_view_length) };
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
    if (str[N - 1] == '\0')
        return temp_string (alloc, std::data (str), std::size (str) - 1);
    return temp_string (alloc, std::data (str), std::size (str));
}

inline std::string_view temp_string (chowdsp::StackAllocator& alloc, std::string_view str)
{
    return temp_string (alloc, std::data (str), std::size (str));
}

inline std::u8string_view temp_string (chowdsp::StackAllocator& alloc, std::u8string_view str)
{
    auto* t_data = alloc.allocate<char8_t> (str.size());
    std::copy (str.begin(), str.end(), t_data);
    return { t_data, str.size() };
}

static Artist& get_artist_for_song (Music_Library& library, Song& song, std::u8string_view artist_name)
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
    song_artist.name = artist_name;
    return song_artist;
}

static Album& get_album_for_song (Music_Library& library, Song& song, std::u8string_view album_name, Artist& artist)
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
    song_album.name = album_name;
    song_album.artist_id = song.artist_id;

    if (std::find (artist.album_ids.begin(), artist.album_ids.end(), song.album_id) == artist.album_ids.end())
        artist.album_ids.push_back (song.album_id);

    return song_album;
}

Music_Library index_directory (const std::filesystem::path& path)
{
    Music_Library library;
    library.stack_data.reset (1 << 21);
    library.artists.reserve (500);
    library.albums.reserve (1'000);
    library.songs.reserve (10'000);

    struct Tag_Result
    {
        TagLib::FileRef file;
        const TagLib::Tag* tag = nullptr;
        std::filesystem::path file_path;
    };

    BS::thread_pool thread_pool {};
    std::vector<std::future<Tag_Result>> tag_results;
    tag_results.reserve (10'000);

    //    std::printf ("Indexing directory: %ls\n", path.c_str());
    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator (path))
    {
        //        std::printf ("    Entry: %s\n", dir_entry.path().c_str());
        if (dir_entry.is_regular_file() && dir_entry.path().extension() == ".mp3")
        {
            tag_results.push_back (thread_pool.submit (
                [file_path = dir_entry.path()]() -> Tag_Result
                {
                    TagLib::FileRef file { file_path.c_str() };
                    return {
                        .file = file,
                        .tag = file.tag(),
                        .file_path = file_path,
                    };
                }));
        }
    }

    for (auto& tag_result_future : tag_results)
    {
        const auto [file, tag, file_path] = tag_result_future.get();
        if (tag == nullptr)
        {
            jassertfalse;
            continue;
        }

        const auto title_str = to_u8string_view (library.stack_data, tag->title());
        const auto album_str = to_u8string_view (library.stack_data, tag->album());
        const auto artist_str = to_u8string_view (library.stack_data, tag->artist());
        if (title_str.empty() || album_str.empty() || artist_str.empty())
            continue; // @TODO: figure out what's going on here!

        const auto song_id = library.songs.size();
        auto& song = library.songs.emplace_back();
        song.name = title_str;

        auto& song_artist = get_artist_for_song (library, song, artist_str);
        auto& song_album = get_album_for_song (library, song, album_str, song_artist);
        song_album.song_ids.push_back (song_id);

        song.track_number = static_cast<int> (tag->track());
        song.filepath = temp_string (library.stack_data, file_path.u8string());

        //            std::cout << "-- TAG (basic) --" << std::endl;
        //            std::cout << "title   - \"" << tag->title()   << "\"" << std::endl;
        //            std::cout << "artist  - \"" << tag->artist()  << "\"" << std::endl;
        //            std::cout << "album   - \"" << tag->album()   << "\"" << std::endl;
        //            std::cout << "year    - \"" << tag->year()    << "\"" << std::endl;
        //            std::cout << "comment - \"" << tag->comment() << "\"" << std::endl;
        //            std::cout << "track   - \"" << tag->track()   << "\"" << std::endl;
        //            std::cout << "genre   - \"" << tag->genre()   << "\"" << std::endl;
    }

//    std::printf ("Stack bytes_used %zu out of %d\n", library.stack_data.get_bytes_used(), 1 << 21);

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
        count += sv.size() * sizeof (typename decltype (sv)::value_type);
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
