#include <chowdsp_logging/chowdsp_logging.h>

#include "music_library.h"

#include "bs_thread_pool.h"
#include "taglib.h"

namespace chow_tunes::library
{
static int64_t u16_string_to_u8 (const TagLib::String& str, char8_t* str_view_start)
{
    char8_t* str_view_end = nullptr;
    try
    {
        str_view_end = utf8::utf16to8 (str.begin(), str.end(), str_view_start);
    }
    catch ([[maybe_unused]] const utf8::exception& e)
    {
        DBG (e.what());
        return 0;
    }
    return std::distance (str_view_start, str_view_end);
}

template <typename String_Type>
static std::u8string_view to_u8string_view (chowdsp::ChainedArenaAllocator& alloc, const String_Type& str)
{
    const auto max_char8_needed = str.size() * 2;
    auto& current_arena = alloc.get_current_arena();
    if (current_arena.get_total_num_bytes() - current_arena.get_bytes_used() < max_char8_needed)
    {
        const auto str_view_start = alloc.allocate<char8_t> (max_char8_needed);
        const auto str_view_length = u16_string_to_u8 (str, str_view_start);
        return { str_view_start, static_cast<size_t> (str_view_length) }; // NOLINT
    }

    const auto str_view_start = alloc.data<char8_t> (current_arena.get_bytes_used());
    const auto str_view_length = u16_string_to_u8 (str, str_view_start);
    [[maybe_unused]] const auto start_ptr = alloc.allocate<char8_t> (str_view_length);
    jassert (start_ptr != nullptr);
    return { str_view_start, static_cast<size_t> (str_view_length) }; // NOLINT
}

static bool equals_ignore_case (const std::u8string_view& lhs, const std::u8string_view& rhs)
{
    return std::equal (lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [] (char a, char b)
                       { return std::tolower (a) == std::tolower (b); });
}

template <typename IntType>
std::string_view temp_string (chowdsp::ChainedArenaAllocator& alloc, const char* data, IntType count)
{
    auto* t_data = alloc.allocate<char> (count);
    std::copy (data, data + count, t_data);
    return { t_data, (size_t) count };
}

template <size_t N>
std::string_view temp_string (chowdsp::ChainedArenaAllocator& alloc, const char (&str)[N])
{
    // don't copy the null terminator!
    if (str[N - 1] == '\0')
        return temp_string (alloc, std::data (str), std::size (str) - 1);
    return temp_string (alloc, std::data (str), std::size (str));
}

inline std::string_view temp_string (chowdsp::ChainedArenaAllocator& alloc, std::string_view str)
{
    return temp_string (alloc, std::data (str), std::size (str));
}

inline std::u8string_view temp_string (chowdsp::ChainedArenaAllocator& alloc, std::u8string_view str)
{
    auto* t_data = alloc.allocate<char8_t> (str.size() + 1);
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

static std::filesystem::path get_artwork_file (const std::filesystem::path& song_file_path)
{
    const auto search_path = song_file_path.parent_path();
    for (const auto& test_art_file :
         std::initializer_list<std::string_view> { "cover.jpg", "Folder.jpg" })
    {
        auto artwork_file = search_path / test_art_file;
        if (std::filesystem::exists (artwork_file))
            return artwork_file;
    }

    // for (std::filesystem::directory_iterator iter { search_path }; iter != std::filesystem::directory_iterator {}; iter++)
    // {
    //     const auto file_ext = iter->path().extension();
    //     if (std::filesystem::exists (*iter))
    //     {
    //         if (file_ext == L".jpg" || file_ext == L".png")
    //             return iter->path();
    //     }
    // }

    return {};
}

struct Tag_Result
{
    std::u8string_view title {};
    std::u8string_view album {};
    std::u8string_view artist {};
    std::u8string_view filepath {};
    std::u8string_view artwork_file {};
    size_t year = 0;
    int track_number = -1; // starts indexing at 0, -1 is "invalid"
};

struct Tag_Results_Array
{
    struct Chunk
    {
        static constexpr size_t chunk_size = 100;
        std::array<std::future<Tag_Result>, chunk_size> chunk {};
        size_t count = 0;
        Chunk* next = nullptr;
    };

    chowdsp::ChainedArenaAllocator* arena {};
    Chunk head_chunk {};
    Chunk* tail_chunk = &head_chunk;

    void insert (std::future<Tag_Result>&& next_result)
    {
        if (tail_chunk->count == Chunk::chunk_size) [[unlikely]]
        {
            tail_chunk->next = new (arena->allocate_bytes (sizeof (Chunk), alignof (Chunk))) Chunk {};
            tail_chunk = tail_chunk->next;
        }

        tail_chunk->chunk[tail_chunk->count++] = std::move (next_result);
    }

    template <typename Func>
    void for_each (Func&& func)
    {
        for (auto* chunk = &head_chunk; chunk != nullptr; chunk = chunk->next)
        {
            for (size_t i = 0; i < chunk->count; ++i)
                func (chunk->chunk[i]);
        }
    }
};

std::shared_ptr<Music_Library> index_directory (const std::filesystem::path& path, const Update_Callback& callback)
{
    auto library_ptr = std::make_shared<Music_Library>();
    library_ptr->artists.reserve (500);
    library_ptr->albums.reserve (700);
    library_ptr->songs.reserve (7'000);

    const auto thread_pool_count = std::min (2 * std::thread::hardware_concurrency() - 1, 8U);
    BS::thread_pool thread_pool { thread_pool_count };

    auto* local_arena = new chowdsp::ChainedArenaAllocator { 10 * sizeof (Tag_Results_Array::Chunk) };
    auto* tag_results = new (local_arena->allocate_bytes (sizeof (Tag_Results_Array), alignof (Tag_Results_Array))) Tag_Results_Array { .arena = local_arena };

    auto per_thread_arenas = chowdsp::arena::make_span<chowdsp::ChainedArenaAllocator> (*local_arena, thread_pool_count);
    for (auto& arena : per_thread_arenas)
        arena = chowdsp::ChainedArenaAllocator { 1 << 14 };

    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator (path))
    {
        const auto extension = dir_entry.path().extension();
        if (dir_entry.is_regular_file()
            && (extension == ".mp3" || extension == ".flac"
                || extension == ".m4a" || extension == ".aac"
                || extension == ".ogg" || extension == ".wav"))
        {
            tag_results->insert (thread_pool.submit_task (
                [file_path = dir_entry.path(), per_thread_arenas]() -> Tag_Result
                {
                    auto& arena = per_thread_arenas[*BS::this_thread::get_index()];
                    TagLib::FileRef file { file_path.c_str(), false };
                    const auto potential_artwork_file = get_artwork_file (file_path);

                    const auto* tag = file.tag();
                    if (tag == nullptr)
                    {
                        jassertfalse;
                        return {};
                    }

                    auto title_str = to_u8string_view (arena, tag->title());
                    auto album_str = to_u8string_view (arena, tag->album());
                    auto artist_str = to_u8string_view (arena, tag->artist());

                    if (title_str.empty())
                        title_str = temp_string (arena, file_path.filename().u8string());
                    if (album_str.empty())
                        album_str = temp_string (arena, file_path.parent_path().u8string());
                    if (artist_str.empty())
                        artist_str = u8"Unknown Artist";

                    return {
                        .title = title_str,
                        .album = album_str,
                        .artist = artist_str,
                        .filepath = temp_string (arena, file_path.u8string()),
                        .artwork_file = temp_string (arena, potential_artwork_file.u8string()),
                        .year = tag->year(),
                        .track_number = static_cast<int> (tag->track()),
                    };
                }));
        }
    }

    auto results_loader = [local_arena, library_ptr, callback, tag_results, per_thread_arenas]() mutable
    {
        auto& library = *library_ptr;
        auto last_update_time = std::chrono::steady_clock::now();
        tag_results->for_each (
            [&] (std::future<Tag_Result>& tag_result_future)
            {
                const auto result = tag_result_future.get();
                if (result.title.empty())
                    return;

                if (song_is_already_in_library (library, result.artist, result.album, result.title))
                    return; // @TODO: here we should filter by the preferred file format!

                const auto song_id = library.songs.size();
                auto& song = library.songs.emplace_back();
                song.name = result.title;

                auto& song_artist = get_artist_for_song (library, song, result.artist);
                auto& song_album = get_album_for_song (library, song, result.album, result.year, song_artist);
                song_album.song_ids.push_back (song_id);
                song_album.year = result.year;

                song.track_number = result.track_number;
                song.filepath = result.filepath;
                song.artwork_file = result.artwork_file;

                if (callback != nullptr)
                {
                    auto now = std::chrono::steady_clock::now();
                    if (now - last_update_time > std::chrono::milliseconds { 100 })
                    {
                        last_update_time = now;
                        callback (library, false);
                    }
                }
            });

        if (callback != nullptr)
            callback (library, true);

        library_ptr->stack_data = {};
        for (auto& arena : per_thread_arenas)
            library_ptr->stack_data.merge (arena);

        delete local_arena;
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

    const auto result = std::string { alloc.data<char> (frame.arena_frame.bytes_used_at_start), count };
    return result;
}
} // namespace chow_tunes::library
