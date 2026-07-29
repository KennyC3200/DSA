#pragma once

#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <memory>
#include <cassert>
#include <bit>

constexpr size_t KiB(size_t n) { return n << 10; }
constexpr size_t MiB(size_t n) { return n << 20; }
constexpr size_t GiB(size_t n) { return n << 30; }

// Round `n` to the next multiple of `p`, given that `p` is a power of 2
constexpr size_t align_up_pow2(size_t n, size_t p) {
    // Guard against `p = 0` and ensure `p` is a power of 2
    assert(std::has_single_bit(p));

    // Prevent integer overflow
    assert(n <= (size_t(-1) - (p - 1)));

    return (n + (p - 1)) & ~(p - 1); 
}

class Arena {
public:
    explicit Arena(size_t reserve_size, size_t commit_size)
    {
        // We have an initial commit size because asking the OS for ram requires a kernel call
        // which is relatively slow. So make sure that the commit_size is relatively large--say 1MB

        // Get the page size
        SYSTEM_INFO sys_info = { 0 };
        GetSystemInfo(&sys_info);
        size_t page_size = sys_info.dwPageSize;

        // Align reserve size and commit size
        reserve_size = align_up_pow2(reserve_size, page_size);
        commit_size = align_up_pow2(commit_size, page_size);

        // Tell the OS to set aside a contiguous block of virtual addresses
        // No physical RAM or page file is consumed during this step
        // Ensure that as the arena grows, it remains perfectly contiguous in memory
        m_buffer = static_cast<std::byte*>(
            VirtualAlloc(NULL, reserve_size, MEM_RESERVE, PAGE_READWRITE));

        // The OS allocates physical memory or pagefile space t4o back those virtual addresses
        // As the arena allocator's internal offset pointer moves forward and crosses a page boundary
        // it dynamically commits the next page
        LPVOID ret = VirtualAlloc(m_buffer, commit_size, MEM_COMMIT, PAGE_READWRITE);
        assert(ret);
        if (!ret) { exit(1); }

        m_reserve_size = reserve_size;
        m_commit_size = commit_size;
        m_pos = 0;
        m_commit_pos = commit_size;
    }
    ~Arena() { VirtualFree(m_buffer, 0, MEM_RELEASE); }
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    void* PushRaw(size_t size, size_t alignment=alignof(std::max_align_t), bool zero=true) {
        size_t aligned_pos = align_up_pow2(m_pos, alignment);
        size_t new_pos = aligned_pos + size;

        if (new_pos > m_reserve_size)
            return nullptr;
        
        if (new_pos > m_commit_pos) {
            size_t new_commit_pos = new_pos;
            new_commit_pos += m_commit_size - 1;
            new_commit_pos -= new_commit_pos % m_commit_size;
            new_commit_pos = std::min(new_commit_pos, m_reserve_size);

            // Increase the commit size
            std::byte* mem = m_buffer + m_commit_pos;
            size_t commit_size = new_commit_pos - m_commit_pos;

            LPVOID ret = VirtualAlloc(mem, commit_size, MEM_COMMIT, PAGE_READWRITE);
            assert(ret);
            if (!ret) { exit(1); }

            // Update the commit position
            m_commit_pos = new_commit_pos;
        }

        m_pos = new_pos;

        void* out = m_buffer + aligned_pos;

        // Remember that `size` is in bytes, so we need to traverse fill_n via bytes
        if (zero) { std::fill_n(static_cast<std::byte*>(out), size, std::byte{ 0 }); }

        return out;
    }

    template <typename T>
    T* Push(bool zero=true) { return static_cast<T*>(PushRaw(sizeof(T), alignof(T), zero)); }

    template<typename T>
    T* PushArray(size_t n, bool zero=true) { 
        return static_cast<T*>(PushRaw(n * sizeof(T), alignof(T), zero)); 
    }

    void Pop(size_t size) { m_pos = (size > m_pos) ? 0 : m_pos - size; }
    void PopTo(size_t pos) { assert(pos > m_pos); m_pos = pos; }
    void Clear() { m_pos = 0; }

private:
    size_t m_reserve_size;
    size_t m_commit_size;
    size_t m_pos;
    size_t m_commit_pos;
    std::byte* m_buffer;
};