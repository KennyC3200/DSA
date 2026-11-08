#pragma once

#include <cstddef>
#include <vector>
#include <memory>
#include <cassert>
#include <new>

// This class actually owns the memory and the free list
// It is untyped, and knows the SlotSize at compile time
// MemoryPoolAlloc is the cheap handle for this
template <
    std::size_t SlotSize,
    std::size_t SlotAlignment = alignof(std::max_align_t),
    std::size_t SlotsPerBlock = 1024>
class MemoryPool {
    union Slot {
        Slot* m_next;
        alignas(SlotAlignment) std::byte m_storage[SlotSize];
    };

    Slot* m_free_head = nullptr;
    std::vector<std::unique_ptr<Slot[]>> m_blocks;
    std::size_t m_live = 0;

    void Grow() {
        // push_back first, in the case it throws so m_free_head stays valid
        m_blocks.push_back(std::unique_ptr<Slot[]>(new Slot[SlotsPerBlock]));
        Slot* block = m_blocks.back().get();

        // Back-to-front so allocations walk through memory
        for (std::size_t i = SlotsPerBlock; i-- > 0;) {
            block[i].m_next = m_free_head;
            m_free_head = &block[i];
        }
    }

public:
    MemoryPool() = default;
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    ~MemoryPool() { assert(m_live == 0 && "Memory pool destroyed with live objects!"); }

    template <std::size_t ReqAlignment>
    [[nodiscard]] std::byte* allocate() {
        static_assert(
            ReqAlignment <= SlotAlignment, 
            "The alignment is too large for this memory pool");

        if (!m_free_head) Grow();

        // Get the slot and increment the head
        Slot* slot = m_free_head;
        m_free_head = m_free_head->m_next;

        m_live++;
        return static_cast<std::byte*>(slot->m_storage);
    }

    void deallocate(std::byte* ptr) noexcept {
        if (!ptr) return;

        Slot* s = reinterpret_cast<Slot*>(ptr);
        s->m_next = m_free_head;
        m_free_head = s;

        m_live--;
    }
};

// This class can only work with node-based containers (std::list, std::forward_list, std::map,
// std::set) and their multi versions. Containers like std::vector that require contiguous
// allocations will not work
template <
    class T, 
    std::size_t SlotSize,
    std::size_t SlotAlignment = alignof(std::max_align_t),
    std::size_t SlotsPerBlock = 1024>
class MemoryPoolAlloc {
public:
    using value_type = T;
    static auto constexpr slot_size = SlotSize;
    static auto constexpr slot_alignment = SlotAlignment;
    static auto constexpr slots_per_block = SlotsPerBlock;
    using memory_pool_type = MemoryPool<SlotSize, slot_alignment, slots_per_block>;

    MemoryPoolAlloc(const MemoryPoolAlloc&) = default;
    MemoryPoolAlloc& operator=(const MemoryPoolAlloc&) = delete;

    MemoryPoolAlloc(memory_pool_type& memory_pool) noexcept : m_memory_pool(memory_pool) {}

    template <class U>
    MemoryPoolAlloc(
        const MemoryPoolAlloc<U, slot_size, slot_alignment, slots_per_block>& other) noexcept
        : m_memory_pool(other.m_memory_pool) {}

    template <class U>
    struct rebind { using other = MemoryPoolAlloc<U, slot_size, slot_alignment, slots_per_block>; };

    [[nodiscard]] T* allocate(std::size_t n) {
        static_assert(sizeof(T) <= SlotSize, "T does not fit in a slot");
        if (n == 1) return reinterpret_cast<T*>(m_memory_pool.template allocate<alignof(T)>());
        throw std::bad_alloc();
    }

    void deallocate(T* ptr, std::size_t n) noexcept {
        assert(n == 1);
        m_memory_pool.deallocate(reinterpret_cast<std::byte*>(ptr));
    }

    template <class T1, class T2, std::size_t N, std::size_t A, std::size_t B>
    friend bool operator==(
        const MemoryPoolAlloc<T1, N, A, B>& x,
        const MemoryPoolAlloc<T2, N, A, B>& y) noexcept; 

    template <class U, std::size_t N, std::size_t A, std::size_t B> friend class MemoryPoolAlloc;

private:
    memory_pool_type& m_memory_pool;
};

template <class T1, class T2, std::size_t N, std::size_t A, std::size_t B>
inline bool operator==(
    const MemoryPoolAlloc<T1, N, A, B>& x,
    const MemoryPoolAlloc<T2, N, A, B>& y) noexcept 
{
    return &x.m_memory_pool == &y.m_memory_pool;
}

#if (defined(_MSVC_LANG) && _MSVC_LANG < 202002L) || (!defined(_MSVC_LANG) && __cplusplus < 202002L)
template <class T1, class T2, std::size_t N, std::size_t A, std::size_t B>
inline bool operator!=(
    const MemoryPoolAlloc<T1, N, A, B>& x,
    const MemoryPoolAlloc<T2, N, A, B>& y) noexcept 
{
    return !(x == y);
}
#endif