#ifndef DPQS_PARALLEL_BUFFER_MANAGER_HPP
#define DPQS_PARALLEL_BUFFER_MANAGER_HPP

#include <vector>
#include <memory>
#include <algorithm>

namespace dual_pivot {

template<typename T, typename Allocator = std::allocator<T>>
class BufferManager {
private:
    static thread_local std::vector<T, Allocator> buffer_pool;
    static thread_local bool pool_initialized;
    static thread_local std::vector<int> buffer_offsets;
    static thread_local int buffer_usage_count;

public:
    static T* getBuffer(int size, int& offset) {
        if (!pool_initialized || buffer_pool.size() < size) {
            int new_size = std::max(size, static_cast<int>(buffer_pool.size() * 1.5));
            buffer_pool.resize(new_size);
            buffer_offsets.resize(new_size / 64 + 1, 0);
            pool_initialized = true;
            offset = 0;
            buffer_usage_count = 0;
            return buffer_pool.data();
        }

        offset = (buffer_usage_count * 32) % (buffer_pool.size() / 2);
        buffer_usage_count++;

        return buffer_pool.data();
    }

    static void returnBuffer(T* buffer, int size, int offset) {
        if (buffer >= buffer_pool.data() &&
            buffer < buffer_pool.data() + buffer_pool.size()) {
            int chunk_index = offset / 64;
            if (chunk_index < buffer_offsets.size()) {
                buffer_offsets[chunk_index] = 0;
            }
        }
    }

    static int getBufferUsage() {
        return buffer_usage_count;
    }

    static void optimizePool() {
        if (buffer_usage_count > 100) {
            buffer_usage_count = 0;
            std::fill(buffer_offsets.begin(), buffer_offsets.end(), 0);
        }
    }
};

template<typename T, typename Allocator>
thread_local std::vector<T, Allocator> BufferManager<T, Allocator>::buffer_pool;

template<typename T, typename Allocator>
thread_local bool BufferManager<T, Allocator>::pool_initialized = false;

template<typename T, typename Allocator>
thread_local std::vector<int> BufferManager<T, Allocator>::buffer_offsets;

template<typename T, typename Allocator>
thread_local int BufferManager<T, Allocator>::buffer_usage_count = 0;

} // namespace dual_pivot

#endif // DPQS_PARALLEL_BUFFER_MANAGER_HPP
