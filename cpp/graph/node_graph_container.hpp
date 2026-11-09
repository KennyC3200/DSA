#pragma once

#include "node_graph.hpp"
#include <memory>

template <typename T, typename Alloc = std::allocator<T>>
class NodeGraphContainer {
public:
    using graph_type = NodeGraph<T, Alloc>;
    using value_type = T;
    using size_type = graph_type::size_type;
    using reference = T&;
    using const_reference = const T&;
    using allocator_type = Alloc;

    explicit NodeGraphContainer(const Alloc& alloc = Alloc()): m_graph(alloc) {}

    reference operator[](size_type pos) { return m_graph[pos].Val(); }
    const_reference operator[](size_type pos) const { return m_graph[pos].Val(); }
    reference front() { return m_graph[0].Val(); }
    const_reference front() const { return m_graph[0].Val(); }

    size_type size() const noexcept { return m_graph.Size(); }
    bool empty() const noexcept { return m_graph.Empty(); }
    void reserve(size_type new_cap) { m_graph.Reserve(new_cap); }
    size_type capacity() const noexcept { return m_graph.Capacity(); }

    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }

    template <class... Args>
    reference emplace_back(Args&&... args) { return _emplace_back(std::forward(args)...); }

    template <class... Args>
    void emplace_back(Args&&... args) { _emplace_back(std::forward(args)...); }

    void pop_back() {
        size_type idx = m_graph.Size() - 1;
        if (idx > 0) m_graph[parent_idx(idx)].Edges().pop_back(); // pop_back() removes last child
        m_graph.PopNode();
    }

private:
    template <class... Args>
    reference _emplace_back(Args&&... args) {
        size_type idx = m_graph.AddNode(std::forward<Args>(args)...);
        if (idx > 0) m_graph.AddEdge(parent_idx(idx), idx);
        return m_graph[idx].Val();
    }

    size_type parent_idx(size_type child_idx) {
        return (child_idx - 1) / 2;
    }

    graph_type m_graph;
};