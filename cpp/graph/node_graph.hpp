#pragma once

#include "node.hpp"

#include <stdexcept>
#include <iostream>
#include <iterator>
#include <cassert>
#include <utility>

template <typename T, typename Alloc = std::allocator<T>>
class NodeGraph {
public:
    using value_type        = T;
    using allocator_type    = Alloc;
    using node_type         = Node<T, Alloc>;
    using size_type         = std::size_t;

private:
    using node_alloc        = typename std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
    using node_alloc_traits = std::allocator_traits<node_alloc>;
    using alloc_traits      = std::allocator_traits<Alloc>;

    // If true, the destination takes over the source's allocator. If the old and new allocators
    // differ, it first frees its memory with the old one
    static constexpr bool pocca = node_alloc_traits::propagate_on_container_copy_assignment::value;

    // If true, or if the allocators compare equal, it can take the buffer. Otherwise, it has to
    // move elements one at a time into its own memory
    static constexpr bool pocma = node_alloc_traits::propagate_on_container_move_assignment::value;

    // If false and the allocators differ, the behaviour is undefined
    static constexpr bool pocs = node_alloc_traits::propagate_on_container_swap::value;

    static constexpr bool always_equal = node_alloc_traits::is_always_equal::value;

public:
    explicit NodeGraph(const Alloc& alloc = Alloc())
        : m_node_alloc(alloc) {}

    ~NodeGraph() {
        DestroyAndFree();
    }

    // Copy the source graph `other` into the source's arena
    NodeGraph(const NodeGraph& other, const Alloc& alloc)
        : m_node_alloc(alloc) { CreateFrom(other); }

    // Ask the source which allocator a copy gets
    NodeGraph(const NodeGraph& other)
        : NodeGraph(
            other,
            alloc_traits::select_on_container_copy_construction(other.GetAllocator())) {}

    NodeGraph& operator=(const NodeGraph& other) {
        // STL containers like std::vector do something different--namely if *this has sufficient
        // capacity they will assign element by element into its existing buffer. Issue with this
        // is if an element throws mid copy, the vector ends up partly overwritten
        // Instead what we do is copy into a tmp instead which is simpler and fully safe

        if (this == &other) return *this;

        // Build the result first in case it throws so *this is not impacted
        NodeGraph tmp(other, pocca ? other.m_node_alloc : m_node_alloc);

        // Free previous memory with old allocator
        DestroyAndFree();

        // If POCCA, then switch allocators
        if constexpr (pocca) m_node_alloc = other.m_node_alloc;

        // Since tmp's allocator is *this's allocator, stealing storage from it is allowed
        StealStorage(tmp);
        return *this;
    }

    // Allocator travels with the buffer
    NodeGraph(NodeGraph&& other) noexcept
        : m_node_alloc(other.m_node_alloc) { StealStorage(other); }

    NodeGraph(NodeGraph&& other, const Alloc& alloc)
        : m_node_alloc(alloc)
    {
        // If this and other have the same allocator, steal the storage
        // Otherwise, create this from other
        if (m_node_alloc == other.m_node_alloc) {
            StealStorage(other);
        } else {
            CreateFrom(std::move(other));
        }
    }

    // pocma    always_equal    path taken                              can throw
    // ---------------------------------------------------------------------------
    // true     true/false      take other's allocator, steal buffer    no
    // false    true            steal buffer                            no
    // false    false           steal directly if allocators equal      yes
    //                          else create tmp and steal from tmp
    NodeGraph& operator=(NodeGraph&& other) noexcept(always_equal || pocma) {
        if (this == &other) return *this;
        if constexpr (pocma) {
            DestroyAndFree();
            m_node_alloc = other.m_node_alloc;
            StealStorage(other);
        } else if (m_node_alloc == other.m_node_alloc) {
            DestroyAndFree();
            StealStorage(other);
        } else {
            NodeGraph tmp(std::move(other), m_node_alloc);
            DestroyAndFree();
            StealStorage(tmp);
        }
        return *this;
    }

    void swap(NodeGraph& other) noexcept {
        if constexpr (pocs) {
            using std::swap;
            swap(m_node_alloc, other.m_node_alloc);
        } else {
            assert(m_node_alloc == other.m_node_alloc);
        }
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    friend void swap(NodeGraph& x, NodeGraph& y) noexcept {
        x.swap(y);
    }

    template <typename... Args>
    size_type AddNode(Args&&... args) {
        // Check current size to max capacity
        if (m_size == m_capacity) {
            size_type new_capacity = m_capacity == 0 ? 4 : m_capacity * 2;
            Relocate<true>(new_capacity, std::forward<Args>(args)...);
        } else {
            node_alloc_traits::construct(
                m_node_alloc, m_data + m_size,
                GetAllocator(), std::forward<Args>(args)...);
        }
        return m_size++;
    }

    void AddEdge(size_type from, size_type to) {
        CheckIndex(from);
        CheckIndex(to);
        m_data[from].Edges().push_back(to);
    }

    node_type& operator[](size_type i) { return m_data[i]; }
    const node_type& operator[](size_type i) const { return m_data[i]; }

    node_type& At(size_type i) { CheckIndex(i); return m_data[i]; }
    const node_type& At(size_type i) const { CheckIndex(i); return m_data[i]; }

    size_type Size() const noexcept { return m_size; }
    size_type Capacity() const noexcept { return m_capacity; }
    bool Empty() const noexcept { return m_size == 0; }

    // Allocates memory for `capacity` nodes but no nodes have been constructed yet
    void Reserve(size_type capacity) {
        if (capacity > m_capacity) Relocate(capacity);
    }

    allocator_type GetAllocator() const noexcept { return allocator_type(m_node_alloc); }

    void Print(std::ostream& os = std::cout) const {
        for (size_type i = 0; i < m_size; i++) {
            os << m_data[i].Val() << ": ";
            for (auto adj : m_data[i].Edges())
                os << m_data[adj].Val() << " ";
            os << std::endl;
        }
    }

    // TODO For removing nodes, we currently only have removing the last appended node,
    //      which is dumb lol
    void PopNode() {
        if (m_size == 0) throw std::out_of_range("PopNode on empty NodeGraph");
        node_alloc_traits::destroy(m_node_alloc, m_data + --m_size);
    }

    node_type* Data() noexcept { return m_data; }
    const node_type* Data() const noexcept { return m_data; }

private:
    void DestroyAndFree() noexcept {
        // Deconstruct all nodes and then deallocate the memory
        for (size_type i = 0; i < m_size; i++)
            node_alloc_traits::destroy(m_node_alloc, m_data + i);
        if (m_data) node_alloc_traits::deallocate(m_node_alloc, m_data, m_capacity);
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    // Take other's buffer, on the condition that this allocator == other's allocator
    void StealStorage(NodeGraph& other) noexcept {
        m_data = std::exchange(other.m_data, nullptr);
        m_size = std::exchange(other.m_size, 0);
        m_capacity = std::exchange(other.m_capacity, 0);
    }

    // Copy each node from other
    void CreateFrom(const NodeGraph& other) {
        CreateFromIter(static_cast<const node_type*>(other.m_data), other.m_size);
    }

    // Move each node out of other
    void CreateFrom(NodeGraph&& other) {
        CreateFromIter(std::make_move_iterator(other.m_data), other.m_size);
    }

    template <typename Iter>
    void CreateFromIter(Iter it, size_type n) {
        assert(m_data == nullptr && m_size == 0);
        if (n == 0) return;
        m_data = node_alloc_traits::allocate(m_node_alloc, n);
        m_capacity = n;
        try {
            for (; m_size < n; ++m_size, ++it) {
                node_alloc_traits::construct(m_node_alloc, m_data + m_size, *it, GetAllocator());
            }
        } catch (...) {
            DestroyAndFree();
            throw;
        }
    }

    template <bool Emplace = false, typename... Args>
    void Relocate(size_type new_capacity, Args&&... args) {
        static_assert(Emplace || sizeof...(Args) == 0);

        // Allocate new chunk of memory and move the nodes over
        node_type* new_data = node_alloc_traits::allocate(m_node_alloc, new_capacity);
        size_type cnt = 0;
        try {
            // Need to build the new node first due to edge case: e.g. AddNode(graph[0].Val())
            // Because the new buffer is moving the elements from the previous buffer, the previous
            // buffer will be empty, so trying to construct graph[0].Val() after the fact will
            // construct an empty element
            if constexpr (Emplace) {
                node_alloc_traits::construct(
                    m_node_alloc, new_data + m_size,
                    GetAllocator(), std::forward<Args>(args)...);
            }
            for (; cnt < m_size; cnt++)
                node_alloc_traits::construct(m_node_alloc, new_data + cnt, std::move(m_data[cnt]));
        } catch (...) {
            for (size_type i = 0; i < cnt; i++)
                node_alloc_traits::destroy(m_node_alloc, new_data + i);
            node_alloc_traits::deallocate(m_node_alloc, new_data, new_capacity);
            throw;
        }

        // Destroy and deallocate the previous nodes
        for (size_type i = 0; i < m_size; i++)
            node_alloc_traits::destroy(m_node_alloc, m_data + i);
        if (m_data) node_alloc_traits::deallocate(m_node_alloc, m_data, m_capacity);

        // Update the data pointers
        m_data = new_data;
        m_capacity = new_capacity;
    }

    void CheckIndex(size_type idx) const {
        if (idx >= m_size)
            throw std::out_of_range("node_graph index out of range");
    }

    // Is also used after conversion back to allocator_type to build each node's edge allocator
    node_alloc m_node_alloc;

    node_type* m_data = nullptr;
    size_type m_size = 0;
    size_type m_capacity = 0;
};
