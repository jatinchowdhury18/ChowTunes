#include "play_queue.h"
#include <ranges>

namespace chow_tunes::play_queue
{
void Play_Queue::init()
{
    queue.clear();
    queue.reserve (100);
}

static void play_song_from_queue (Play_Queue& play_queue, int song_index)
{
    const auto& song = *play_queue.queue[(size_t) song_index];
    play_queue.action_router->route_action (audio::create_play_song_action (song));
    play_queue.queue_changed();
}

void Play_Queue::play_previous_song()
{
    if (currently_playing_song_index < 1)
        return;
    play_song_from_queue (*this, --currently_playing_song_index);
}

void Play_Queue::play_next_song()
{
    if (currently_playing_song_index >= (int) queue.size() - 1)
        return;
    play_song_from_queue (*this, ++currently_playing_song_index);
}

void Play_Queue::restart_current_song()
{
    if (currently_playing_song_index < 0)
        return;
    play_song_from_queue (*this, currently_playing_song_index);
}

void Play_Queue::add_to_queue (std::span<const library::Song*> songs_to_add, Add_To_Queue_Action action)
{
    if (action == Add_To_Queue_Action::Play_Now || queue.empty())
    {
        currently_playing_song_index = -1;
        queue.clear();
        queue.insert (queue.end(), songs_to_add.begin(), songs_to_add.end());

        play_next_song();
    }
    else
    {
        const auto insert_position = [this, action]() -> std::vector<const library::Song*>::const_iterator
        {
            if (action == Add_To_Queue_Action::Insert_Next)
                return queue.begin() + currently_playing_song_index + 1;
            return queue.end();
        }();
        queue.insert (insert_position, songs_to_add.begin(), songs_to_add.end());
    }

    queue_changed();
}

static void move_song_up (std::vector<const library::Song*>& queue,
                          const library::Song* song,
                          int& playing_idx)
{
    auto song_iter = std::find (queue.begin(), queue.end(), song);
    if (song_iter == queue.begin() || song_iter == queue.end())
    {
        jassertfalse;
        return;
    }

    if (std::distance (queue.begin(), song_iter - 1) == playing_idx)
        playing_idx++;

    std::rotate (song_iter - 1, song_iter, song_iter + 1);
}

static void move_song_down (std::vector<const library::Song*>& queue,
                            const library::Song* song,
                            int& playing_idx)
{
    auto song_iter = std::find (queue.begin(), queue.end(), song);
    if (song_iter == queue.end() - 1 || song_iter == queue.end())
    {
        jassertfalse;
        return;
    }

    if (std::distance (queue.begin(), song_iter + 1) == playing_idx)
        playing_idx--;

    std::rotate (song_iter, song_iter + 1, song_iter + 2);
}

void Play_Queue::move_songs_up (std::span<size_t> song_indexes)
{
    if (song_indexes.front() == 0)
        return;

    for (auto i : song_indexes)
        move_song_up (queue, queue[i], currently_playing_song_index);
}

void Play_Queue::move_songs_down (std::span<size_t> song_indexes)
{
    if (song_indexes.back() == queue.size() - 1)
        return;

    for (auto i : std::views::reverse (song_indexes))
        move_song_down (queue, queue[i], currently_playing_song_index);
}

void Play_Queue::remove_songs (std::span<size_t> song_indexes)
{
    for (auto i : std::views::reverse (song_indexes))
        queue.erase (queue.begin() + (int) i);
    queue_changed();
}

void Play_Queue::clear_queue()
{
    action_router->route_action ({ .action_type = audio::Audio_Player_Action_Type::Stop_Song });
    currently_playing_song_index = -1;
    queue.clear();
    queue_changed();
}
} // namespace chow_tunes::play_queue
