#pragma once

#include <cstddef>
#include <vector>
#include <memory>
#include <utility>
#include <cassert>

template <typename T, std::size_t SlotsPerChunk = 1024>
class MemoryPool {
    // A slot either stores a T or, while free, is a link in the "free list" (containing free slots)
    // If the slot is not occupied, it will store the next slot, effectively creating a linked list
    // which is the free list.
    // The union makes sizeof/alignof = max(T, pointer) so the free list costs zero extra-bytes
    // That is, if T < sizeof(T) = 8 bytes, it will default to 8 bytes and if it is > 8 bytes, well
    // it won't matter we will just have extra bytes lol cuz a pointer only needs 8 bytes
    union Slot {
        Slot* m_next;
        alignas(T) std::byte m_storage[sizeof(T)];
    };

    Slot* m_free_head = nullptr;
    std::vector<std::unique_ptr<Slot[]>> m_blocks;
    std::size_t m_live = 0;

    void Grow() {
        std::unique_ptr<Slot[]> block(new Slot[SlotsPerChunk]);

        // Back-to-front so allocations walk forward through memory
        for (std::size_t i = SlotsPerChunk; i-- > 0;) {
            block[i].m_next = m_free_head;
            m_free_head = &block[i];
        }

        m_blocks.push_back(std::move(block));
    }

public:
    MemoryPool() = default;
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    ~MemoryPool() { assert(m_live == 0 && "Memory pool destroyed with live objects!"); }

    template <typename... Args>
    T* Create(Args&&... args) {
        if (!m_free_head) Grow();

        // Get the slot and increment the head
        Slot* slot = m_free_head;
        m_free_head = m_free_head->m_next;

        try {
            // This is weird syntax, but basically the static_cast<void*>(slot->m_storage) 
            // (which we'll call p) is a placement argument for the new-expression, and it calls 
            // the following overload:
            // void* operator new(std::size_t, void* p) noexcept { return p; }
            // Which does not create an allocation (since m_storage already allocates) and instead
            // constructs T at that address
            T* obj = ::new (static_cast<void*>(slot->m_storage)) T(std::forward<Args>(args)...);
            m_live++;
            return obj;
        } catch (...) {
            // Constructor threw: give the slot back
            slot->m_next = m_free_head;
            m_free_head = slot;
            throw;
        }
    }

    void Destroy(T* obj) noexcept {
        // Run the destructor and push the slot into the free list
        if (!obj) return;
        obj->~T();

        Slot* s = reinterpret_cast<Slot*>(obj);
        s->m_next = m_free_head;
        m_free_head = s;

        m_live--;
    }

    // unique_ptr also allows a custom deleter upon destruction
    struct Deleter {
        MemoryPool* m_pool;
        void operator()(T* ptr) const noexcept { m_pool->Destroy(ptr); }
    };

    using Handle = std::unique_ptr<T, Deleter>;

    template <typename... Args>
    Handle Make(Args&&... args) {
        return Handle(Create(std::forward<Args>(args)...), Deleter{this});
    }

    std::size_t Live() const noexcept { return m_live; }
    std::size_t Capacity() const noexcept { return m_blocks.size() * SlotsPerChunk; }

    // Make this constexpr because Slot is determined at compile time
    static constexpr std::size_t SlotSize() noexcept { return sizeof(Slot); }
};