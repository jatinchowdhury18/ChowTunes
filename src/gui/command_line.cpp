#include "command_line.h"
#include "library/music_library.h"
#include "play_queue/play_queue.h"
#include "library/library_view.h"

namespace chow_tunes::cli
{
void play_command (const juce::StringArray& args,
                   const library::Music_Library& library,
                   play_queue::Play_Queue& play_queue)
{
    const auto index_offset = args.indexOf ("--play");
    const auto arg_to_str = [&args, index_offset] (int arg_number) -> std::u8string_view
    {
        if (index_offset + arg_number >= args.size())
            return {};

        auto arg_str = std::u8string_view { (const char8_t*) args[index_offset + arg_number].toRawUTF8(),
                                            args[index_offset + arg_number].getNumBytesAsUTF8() };

        if (arg_str[0] == '\"')
            return arg_str.substr (1, arg_str.size() - 2);
        return arg_str;
    };

    const auto artist_name = arg_to_str (1);
    const auto album_name = arg_to_str (2);
    const auto song_name = arg_to_str (3);

    if (artist_name.empty() || album_name.empty())
    {
        std::cout << "Artist and/or album name not provided!\n";
        return;
    }

    const auto artist_iter = std::find_if (library.artists.begin(),
                                           library.artists.end(),
                                           [artist_name] (const chow_tunes::library::Artist& artist)
                                           {
                                               return artist_name == artist.name;
                                           });
    if (artist_iter == library.artists.end())
    {
        std::cout << "Artist not found!\n";
        return;
    }

    const auto album_iter = std::find_if (artist_iter->album_ids.begin(),
                                          artist_iter->album_ids.end(),
                                          [&library, album_name] (size_t album_id)
                                          {
                                              return album_name == library.albums[album_id].name;
                                          });
    if (album_iter == artist_iter->album_ids.end())
    {
        std::cout << "Album not found!\n";
        return;
    }

    const auto& album = library.albums[*album_iter];

    static constexpr auto play_now = chow_tunes::play_queue::Play_Queue::Add_To_Queue_Action::Play_Now;
    if (song_name.empty())
    {
        // play whole album
        std::cout << "Playing album: "
                  << juce::String::fromUTF8 ((const char*) album.name.data(), (int) album.name.size())
                  << ", by artist: "
                  << juce::String::fromUTF8 ((const char*) artist_iter->name.data(), (int) artist_iter->name.size())
                  << std::endl;
        auto album_songs = gui::get_album_songs (album, library);
        play_queue.add_to_queue (album_songs, play_now);
        return;
    }

    const auto song_iter = std::find_if (album.song_ids.begin(),
                                         album.song_ids.end(),
                                         [&library, song_name] (size_t song_id)
                                         {
                                             return song_name == library.songs[song_id].name;
                                         });
    if (song_iter == album.song_ids.end())
    {
        std::cout << "Song not found!\n";
        return;
    }

    // play single song
    const auto& song = library.songs[*song_iter];
    std::cout << "Playing song: "
              << juce::String::fromUTF8 ((const char*) song.name.data(), (int) song.name.size())
              << ", from album: "
              << juce::String::fromUTF8 ((const char*) album.name.data(), (int) album.name.size())
              << ", by artist: "
              << juce::String::fromUTF8 ((const char*) artist_iter->name.data(), (int) artist_iter->name.size())
              << std::endl;
    std::array<const chow_tunes::library::Song*, 1> songs { &song };
    play_queue.add_to_queue (songs, play_now);
}
}
