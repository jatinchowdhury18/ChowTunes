#include "music_library.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmacro-redefined", "-Wdeprecated-declarations", "-Wdeprecated-dynamic-exception-spec")
#include <fileref.h>
#include <tpropertymap.h>
#include <utf8-cpp/checked.h>

#include "bs_thread_pool.h"

namespace chow_tunes::library
{
static std::u8string_view to_u8string_view (chowdsp::ArenaAllocatorView alloc, const TagLib::String& str)
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

static bool equals_ignore_case (const std::u8string_view& lhs, const std::u8string_view& rhs)
{
    return std::equal (lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [] (char a, char b)
                       { return std::tolower (a) == std::tolower (b); });
}

template <typename IntType>
std::string_view temp_string (chowdsp::ArenaAllocatorView alloc, const char* data, IntType count)
{
    auto* t_data = alloc.allocate<char> (count);
    std::copy (data, data + count, t_data);
    return { t_data, (size_t) count };
}

template <size_t N>
std::string_view temp_string (chowdsp::ArenaAllocatorView alloc, const char (&str)[N])
{
    // don't copy the null terminator!
    if (str[N - 1] == '\0')
        return temp_string (alloc, std::data (str), std::size (str) - 1);
    return temp_string (alloc, std::data (str), std::size (str));
}

inline std::string_view temp_string (chowdsp::ArenaAllocatorView alloc, std::string_view str)
{
    return temp_string (alloc, std::data (str), std::size (str));
}

inline std::u8string_view temp_string (chowdsp::ArenaAllocatorView alloc, std::u8string_view str)
{
    auto* t_data = alloc.allocate<char8_t> (str.size());
    std::copy (str.begin(), str.end(), t_data);
    return { t_data, str.size() };
}

static Artist& get_artist_for_song (Music_Library& library, Song& song, std::u8string_view artist_name)
{
    for (auto [idx, artist] : chowdsp::enumerate (library.artists))
    {
        if (equals_ignore_case (artist.name, artist_name))
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

static Album& get_album_for_song (Music_Library& library,
                                  Song& song,
                                  std::u8string_view album_name,
                                  size_t album_year,
                                  Artist& artist)
{
    chowdsp::EndOfScopeAction _ {
        [&artist, &song]
        {
            if (std::find (artist.album_ids.begin(), artist.album_ids.end(), song.album_id) == artist.album_ids.end())
                artist.album_ids.push_back (song.album_id);
        }
    };

    const auto roughly_the_same_artist = [&library, &song_artist = library.artists[song.artist_id]] (const Album& album) -> bool
    {
        const auto album_artist = library.artists[album.artist_id];
        if (&album_artist == &song_artist)
            return true;

        if (album_artist.name.size() == song_artist.name.size())
            return album_artist.name == song_artist.name;

        const auto larger_name = album_artist.name.size() > song_artist.name.size() ? album_artist.name : song_artist.name;
        const auto smaller_name = album_artist.name.size() < song_artist.name.size() ? album_artist.name : song_artist.name;
        return larger_name.find (smaller_name) != std::u8string_view::npos;
    };

    for (auto [idx, album] : chowdsp::enumerate (library.albums))
    {
        // So we have this problem where we want to figure out which album a song is from
        // even though an album might contain multiple songs from different artists. So
        // we're going to say that if two songs are from albums with the same name
        // From either the same year or the same artist, then we'll consider them to be
        // from the same album.
        //
        // @TODO: can we split up Disc 1 from Disc 2?

        if (equals_ignore_case (album.name, album_name) && (album.year == album_year || roughly_the_same_artist (album)))
        {
            song.album_id = idx;
            return album;
        }
    }

    song.album_id = library.albums.size();
    auto& song_album = library.albums.emplace_back();
    song_album.name = album_name;
    song_album.artist_id = song.artist_id;
    return song_album;
}

static bool song_is_already_in_library (const Music_Library& library,
                                        const std::u8string_view& artist_name,
                                        const std::u8string_view& album_name,
                                        const std::u8string_view& song_name)
{
    for (const auto& test_song : library.songs)
    {
        if (test_song.name != song_name)
            continue;

        const auto test_artist = library.artists[test_song.artist_id];
        const auto test_album = library.albums[test_song.album_id];
        if (equals_ignore_case (test_artist.name, artist_name)
            && equals_ignore_case (test_album.name, album_name))
            return true;
    }
    return false;
}

std::shared_ptr<Music_Library> index_directory (const std::filesystem::path& path, const Update_Callback& callback)
{
    auto library_ptr = std::make_shared<Music_Library>();
    library_ptr->stack_data.reset (1 << 21);
    library_ptr->artists.reserve (500);
    library_ptr->albums.reserve (1'000);
    library_ptr->songs.reserve (10'000);

    struct Tag_Result
    {
        TagLib::FileRef file;
        const TagLib::Tag* tag = nullptr;
        std::filesystem::path file_path;
        std::filesystem::path artwork_path;
    };

    BS::thread_pool thread_pool {};
    auto tag_results = std::make_shared<std::vector<std::future<Tag_Result>>>();
    tag_results->reserve (10'000);

    //    std::printf ("Indexing directory: %ls\n", path.c_str());
    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator (path))
    {
        //        std::printf ("    Entry: %s\n", dir_entry.path().c_str());
        const auto extension = dir_entry.path().extension();
        if (dir_entry.is_regular_file()
            && (extension == ".mp3" || extension == ".flac"
                || extension == ".m4a" || extension == ".aac"
                || extension == ".ogg"))
        {
            tag_results->push_back (thread_pool.submit (
                [file_path = dir_entry.path()]() -> Tag_Result
                {
                    TagLib::FileRef file { file_path.c_str() };

                    const auto potential_artwork_file = [search_path = file_path.parent_path()]() -> std::filesystem::path
                    {
                        for (const auto& test_art_file :
                             std::initializer_list<std::string_view> { "cover.jpg", "Folder.jpg" })
                        {
                            auto artwork_file = search_path / test_art_file;
                            if (std::filesystem::exists (artwork_file))
                                return artwork_file;
                        }

                        for (std::filesystem::directory_iterator iter { search_path }; iter != std::filesystem::directory_iterator {}; iter++)
                        {
                            const auto file_ext = iter->path().extension();
                            if (std::filesystem::is_regular_file (*iter))
                            {
                                if (file_ext == L".jpg" || file_ext == L".png")
                                    return iter->path();
                            }
                        }
                        return {};
                    }();

                    return {
                        .file = file,
                        .tag = file.tag(),
                        .file_path = file_path,
                        .artwork_path = potential_artwork_file,
                    };
                }));
        }
    }

    auto results_loader = [library_ptr, callback, tag_results]() mutable
    {
        auto& library = *library_ptr;
        auto last_update_time = std::chrono::steady_clock::now();
        for (auto& tag_result_future : *tag_results)
        {
            const auto [file, tag, file_path, art_path] = tag_result_future.get();
            if (tag == nullptr)
            {
                jassertfalse;
                continue;
            }

            const auto title_str = to_u8string_view (library.stack_data, tag->title());
            const auto album_str = to_u8string_view (library.stack_data, tag->album());
            const auto artist_str = to_u8string_view (library.stack_data, tag->artist());
            if (title_str.empty() || album_str.empty() || artist_str.empty())
            {
                continue; // @TODO: figure out what's going on here!
            }

            if (song_is_already_in_library (library, artist_str, album_str, title_str))
                continue;

            const auto song_id = library.songs.size();
            auto& song = library.songs.emplace_back();
            song.name = title_str;

            auto& song_artist = get_artist_for_song (library, song, artist_str);
            auto& song_album = get_album_for_song (library, song, album_str, tag->year(), song_artist);
            song_album.song_ids.push_back (song_id);
            song_album.year = tag->year();

            song.track_number = static_cast<int> (tag->track());
            song.filepath = temp_string (library.stack_data, file_path.u8string());
            song.artwork_file = temp_string (library.stack_data, art_path.u8string());
            if (const auto* properties = file.audioProperties())
                song.track_length_seconds = properties->lengthInSeconds();

            if (callback != nullptr)
            {
                auto now = std::chrono::steady_clock::now();
                if (now - last_update_time > std::chrono::milliseconds { 100 })
                {
                    last_update_time = now;
                    callback (library, false);
                }
            }
        }

        if (callback != nullptr)
            callback (library, true);

        // std::printf ("Stack bytes_used %zu out of %d\n", library.stack_data.get_bytes_used(), 1 << 21);
    };

    if (callback != nullptr)
        juce::Thread::launch (results_loader);
    else
        results_loader();

    return library_ptr;
}

std::string print_library (const Music_Library& library)
{
    auto& alloc = library.stack_data;
    auto frame = alloc.create_frame();

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
