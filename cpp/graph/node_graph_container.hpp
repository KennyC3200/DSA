#pragma once

#include "node_graph.hpp"
#include <memory>
#include <iterator>
#include <type_traits>
#include <cstddef>

template <typename T, typename Alloc = std::allocator<T>>
class NodeGraphContainer {
public:
    using graph_type = NodeGraph<T, Alloc>;
    using value_type = T;
    using size_type = typename graph_type::size_type;
    using reference = T&;
    using const_reference = const T&;
    using allocator_type = Alloc;

private:
    using node_type = typename graph_type::node_type;

    template <bool IsConst>
    class basic_iterator {
        using node_ptr = std::conditional_t<IsConst, const node_type*, node_type*>;
        template <bool> friend class basic_iterator;

    public:
        using iterator_concept = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;

        basic_iterator() = default;
        explicit basic_iterator(node_ptr p) noexcept : m_ptr(p) {}

        // iterator -> const_iterator
        template <bool Other> requires (IsConst && !Other)
        basic_iterator(const basic_iterator<Other>& o) noexcept : m_ptr(o.m_ptr) {}

        [[nodiscard]] reference operator*() const noexcept { return m_ptr->Val(); }
        [[nodiscard]] pointer operator->() const noexcept { return std::addressof(m_ptr->Val()); }
        [[nodiscard]] reference operator[](difference_type offset) const noexcept { return m_ptr[offset].Val(); }

        basic_iterator& operator++() noexcept { ++m_ptr; return *this; }
        basic_iterator operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }

        basic_iterator& operator--() noexcept { --m_ptr; return *this; }
        basic_iterator operator--(int) noexcept { auto tmp = *this; --*this; return tmp; }

        basic_iterator& operator+=(const difference_type offset) noexcept { 
            m_ptr += offset; 
            return *this; 
        }

        basic_iterator& operator-=(const difference_type offset) noexcept { 
            m_ptr -= offset; 
            return *this; 
        }

        [[nodiscard]] basic_iterator operator+(const difference_type offset) const noexcept { 
            auto tmp = *this; 
            tmp += offset; 
            return tmp;
        }

        [[nodiscard]] friend basic_iterator operator+(
            difference_type offset, basic_iterator it) noexcept 
        {
            return it += offset;
        }

        [[nodiscard]] basic_iterator operator-(const difference_type offset) const noexcept { 
            auto tmp = *this; 
            tmp -= offset; 
            return tmp;
        }

        friend difference_type operator-(
            const basic_iterator& a, const basic_iterator& b) noexcept 
        {
            return a.m_ptr - b.m_ptr;
        }

        bool operator==(const basic_iterator& a) const noexcept = default;
        auto operator<=>(const basic_iterator& a) const noexcept = default;
    
    private:
        node_ptr m_ptr = nullptr;
    };

public:
    using iterator = basic_iterator<false>;
    using const_iterator = basic_iterator<true>;

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
    reference emplace_back(Args&&... args) {
        size_type idx = m_graph.AddNode(std::forward<Args>(args)...);
        if (idx > 0) m_graph.AddEdge(parent_idx(idx), idx);
        return m_graph[idx].Val();
    }

    void pop_back() {
        size_type idx = m_graph.Size() - 1;
        if (idx > 0) m_graph[parent_idx(idx)].Edges().pop_back(); // pop_back() removes last child
        m_graph.PopNode();
    }

    [[nodiscard]] iterator begin() noexcept { return iterator(m_graph.Data()); }
    [[nodiscard]] iterator end() noexcept { return iterator(m_graph.Data() + size()); }
    [[nodiscard]] const_iterator begin() const noexcept { return const_iterator(m_graph.Data()); }
    [[nodiscard]] const_iterator end() const noexcept { 
        return const_iterator(m_graph.Data() + size()); 
    }
    [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }
    [[nodiscard]] const_iterator cend() const noexcept { return end(); }

private:
    static size_type parent_idx(size_type child_idx) {
        return (child_idx - 1) / 2;
    }

    graph_type m_graph;
};

static_assert(std::random_access_iterator<NodeGraphContainer<int>::iterator>);
static_assert(std::random_access_iterator<NodeGraphContainer<int>::const_iterator>);