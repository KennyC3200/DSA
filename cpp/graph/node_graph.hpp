#pragma once

#include "node.hpp"
#include <stdexcept>
#include <iostream>

template <typename T, typename Alloc = std::allocator<T>>
class NodeGraph {
public:
    using value_type = T;
    using allocator_type = Alloc;
    using node_type = Node<T, Alloc>;
    using size_type = std::size_t;

private:
    using node_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
    using node_alloc_traits = std::allocator_traits<node_alloc>;

public:
    explicit NodeGraph(const Alloc& alloc = Alloc()) 
        : m_alloc(alloc)
        , m_node_alloc(alloc) {}

    ~NodeGraph() {
        // Deconstruct all nodes and then deallocate the memory
        for (size_type i = 0; i < m_size; i++)
            node_alloc_traits::destroy(m_node_alloc, m_data + i);
        if (m_data) node_alloc_traits::deallocate(m_node_alloc, m_data, m_capacity);
        m_size = 0;
    }

    NodeGraph(const NodeGraph&) = delete;
    NodeGraph& operator=(const NodeGraph&) = delete;

    NodeGraph(NodeGraph&& other) noexcept
        : m_alloc(std::move(other.m_alloc))
        , m_node_alloc(std::move(other.m_node_alloc))
        , m_data(other.m_data) 
        , m_size(other.m_size)
        , m_capacity(other.m_capacity) 
    {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    template <typename... Args>
    size_type AddNode(Args&&... args) {
        // Check current size to max capacity
        if (m_size == m_capacity) {
            size_type new_capacity = m_capacity == 0 ? 4 : m_capacity * 2;
            Relocate(new_capacity);
        }

        node_alloc_traits::construct(
            m_node_alloc, m_data + m_size, 
            m_alloc, std::forward<Args>(args)...);
        
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
        if (capacity > m_capacity)
            Relocate(capacity);
    }

    allocator_type GetAllocator() { return m_alloc; }

    void Print(std::ostream& os = std::cout) {
        for (size_type i = 0; i < m_size; i++) {
            os << m_data[i].Val() << ": ";
            for (auto adj : m_data[i].Edges())
                os << m_data[adj].Val() << " ";
            os << std::endl;
        }
    }

private:
    void Relocate(size_type new_capacity) {
        // Allocate new chunk of memory and move the nodes over
        node_type* new_data = node_alloc_traits::allocate(m_node_alloc, new_capacity);
        size_type cnt = 0;
        try {
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

    allocator_type m_alloc;  // Original allocator, needed for rebinding when allocating the edges
    node_alloc m_node_alloc; // Allocator for node
    node_type* m_data = nullptr;
    size_type m_size = 0;
    size_type m_capacity = 0;
};
