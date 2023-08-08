#pragma once

#include <vector>

namespace chowdsp
{
class StackAllocator
{
public:
    StackAllocator() = default;

    StackAllocator (const StackAllocator&) = delete;
    StackAllocator& operator= (const StackAllocator&) = delete;

    StackAllocator (StackAllocator&&) noexcept = default;
    StackAllocator& operator= (StackAllocator&&) noexcept = default;

    void reset (size_t new_size_bytes)
    {
        clear();
        raw_data.resize (new_size_bytes, {});
    }

    void clear() noexcept
    {
        bytes_used = 0;
    }

    void* allocate_bytes (size_t num_bytes) noexcept
    {
        if (bytes_used + num_bytes > raw_data.size())
            return nullptr;

        auto* pointer = raw_data.data() + bytes_used;
        bytes_used += num_bytes;
        return pointer;
    }

    template <typename T, typename IntType>
    T* allocate (IntType num_Ts) noexcept
    {
        return static_cast<T*> (allocate_bytes ((size_t) num_Ts * sizeof (T)));
    }

    template <typename T, typename IntType>
    T* data (IntType offset) noexcept
    {
        return reinterpret_cast<T*> (raw_data.data() + offset);
    }

    struct StackAllocatorFrame
    {
        explicit StackAllocatorFrame (StackAllocator& allocator)
            : alloc (allocator),
              bytes_used_at_start (alloc.bytes_used)
        {
        }

        ~StackAllocatorFrame()
        {
            alloc.bytes_used = bytes_used_at_start;
        }

        StackAllocator& alloc;
        const size_t bytes_used_at_start;
    };

private:
    std::vector<std::byte> raw_data {};
    size_t bytes_used = 0;
};
} // namespace chowdsp
