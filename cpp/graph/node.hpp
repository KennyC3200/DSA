#pragma once

#include <arena_allocator/stack_arena.hpp>

template <typename T, typename Alloc = std::allocator<T>>
class Node {
public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = std::size_t;

    using edge_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<size_type>;
    using edge_container = std::vector<size_type, edge_alloc>;

    template <typename... Args>
    explicit Node(const Alloc& alloc, Args&&... args)
        : m_val(std::forward<Args>(args)...)
        , m_edges(edge_alloc(alloc)) {}

    T& Val() noexcept { return m_val; }
    const T& Val() const noexcept{ return m_val; }

    edge_container& Edges() noexcept { return m_edges; }
    const edge_container& Edges() const noexcept { return m_edges; }

private:
    T m_val;
    edge_container m_edges;
};