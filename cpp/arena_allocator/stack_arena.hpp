#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <new>

namespace mem {
    constexpr size_t KiB(size_t n) { return n << 10; }
    constexpr size_t MiB(size_t n) { return n << 20; }
    constexpr size_t GiB(size_t n) { return n << 30; }
};

template <std::size_t N, std::size_t alignment = alignof(std::max_align_t)>
class arena {
public:
    arena() noexcept : m_ptr(m_buffer) {}
    ~arena() { m_ptr = nullptr; }
    arena(const arena&) = delete;
    arena& operator=(const arena&) = delete;

    template <std::size_t ReqAlign> 
    [[nodiscard]] std::byte* allocate(std::size_t n) {
        static_assert(ReqAlign <= alignment, "the alignment is too small for this arena");
        assert(ptr_in_buffer(m_ptr) && "short_alloc has outlived the arena");
        auto const aligned_n = align_up_pow2(n, alignment);

        // Allocate on the stack
        if (aligned_n <= static_cast<decltype(aligned_n)>(m_buffer + N - m_ptr)) {
            std::byte* r = m_ptr;
            m_ptr += aligned_n;
            return r;
        }

        // Allocate on the heap
#if defined(__cpp_aligned_new) && __cpp_aligned_new >= 201606L
        // If the alignment requirement is greater than alignof(std::max_align_t) 
        // we have to manually set the alignment requirement (C++17 onwards)
        if constexpr (alignment > alignof(std::max_align_t)) {
            return static_cast<std::byte*>(::operator new(n, std::align_val_t{alignment}));
        } else {
            return static_cast<std::byte*>(::operator new(n));
        }
#else
        // Fallback for below C++17
        static_assert(alignment <= alignof(std::max_align_t), "you've chosen an "
            "alignment that is larger than alignof(std::max_align_t), and "
            "cannot be guaranteed by normal operator new");
        return static_cast<std::byte*>(::operator new(n));
#endif
    }

    void deallocate(std::byte* p, std::size_t n) noexcept {
        assert(ptr_in_buffer(m_ptr) && "short_alloc has outlived the arena");
        if (ptr_in_buffer(p)) {
            n = align_up_pow2(n, alignment);
            
            // Can only free if it is the most recently allocated block on the stack
            // On the heap, we don't have to deal with this issue
            if (p + n == m_ptr) { m_ptr = p; }
        } else { 
#if defined(__cpp_aligned_new) && __cpp_aligned_new >= 201606L
            if constexpr (alignment > alignof(std::max_align_t)) {
                ::operator delete(p, std::align_val_t{alignment});
            } else {
                ::operator delete(p); 
            }
#else
            ::operator delete(p);
#endif
        }
    }

    static constexpr std::size_t size() noexcept { return N; }
    std::size_t used() const noexcept { return static_cast<std::size_t>(m_ptr - m_buffer); }
    void reset() noexcept { m_ptr = m_buffer; }

private:
    static constexpr std::size_t align_up_pow2(std::size_t n, std::size_t p) noexcept {
        assert(p != 0 && (p & (p - 1)) == 0);
        assert(n <= (std::size_t(-1) - (p - 1)));
        return (n + (p - 1)) & ~(p - 1);
    }

    constexpr bool ptr_in_buffer(const std::byte* p) const noexcept {
        return std::less_equal<const std::byte*>{}(m_buffer, p) &&
            std::less_equal<const std::byte*>{}(p, m_buffer + N);
    }

    // alignas(alignment) is crucial because it aligns &m_buffer[0] which is prerequisite for all
    // later bytes to be aligned by alignment as well
    alignas(alignment) std::byte m_buffer[N];
    std::byte* m_ptr;
};

template <class T, std::size_t N, std::size_t Align = alignof(std::max_align_t)>
class short_alloc {
public:
    using value_type = T;
    static auto constexpr alignment = Align;
    static auto constexpr size = N;
    using arena_type = arena<size, alignment>;

    short_alloc(const short_alloc&) = default;
    short_alloc& operator=(const short_alloc&) = delete;

    short_alloc(arena_type& a) noexcept : m_arena(a) {
        static_assert(size % alignment == 0, "size N needs to be a multiple of alignment Align");
    }

    template <class U>
    short_alloc(const short_alloc<U, N, alignment>& a) noexcept : m_arena(a.m_arena) {}

    template <class U>
    struct rebind { using other = short_alloc<U, N, Align>; };

    [[nodiscard]] T* allocate(std::size_t n) {
        return reinterpret_cast<T*>(m_arena.template allocate<alignof(T)>(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept {
        m_arena.deallocate(reinterpret_cast<std::byte*>(p), n * sizeof(T));
    }

    template <class T1, std::size_t N1, std::size_t A1, class U, std::size_t M, std::size_t A2>
    friend bool operator==(
        const short_alloc<T1, N1, A1>& x, 
        const short_alloc<U, M, A2>& y) noexcept;

    template <class U, std::size_t M, std::size_t A> friend class short_alloc;

private:
    arena_type& m_arena;
};

template <class T1, std::size_t N1, std::size_t A1, class U, std::size_t M, std::size_t A2>
inline bool operator==(
    const short_alloc<T1, N1, A1>& x, 
    const short_alloc<U, M, A2>& y) noexcept
{
    return N1 == M && A1 == A2 && &x.m_arena == &y.m_arena;
}

#if (defined(_MSVC_LANG) && _MSVC_LANG < 202002L) || (!defined(_MSVC_LANG) && __cplusplus < 202002L)
    template <class T1, std::size_t N1, std::size_t A1, class U, std::size_t M, std::size_t A2>
    inline bool operator!=(
        const short_alloc<T1, N1, A1>& x, 
        const short_alloc<U, M, A2>& y) noexcept 
    {
        return !(x == y);
    }
#endif