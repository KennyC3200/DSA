#pragma once

#include <stdexcept>
#include <vector>
#include <functional>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <iostream>
#include <string>
#include "../graph/complete_tree.hpp"

template <typename Container>
struct container_arity : std::integral_constant<std::size_t, 2> {};

template <typename T, typename Alloc, std::size_t Arity>
struct container_arity<CompleteTree<T, Alloc, Arity>>
    : std::integral_constant<std::size_t, Arity> {};

template <
    typename T,
    typename Container = std::vector<T>,
    typename Compare = std::less<typename Container::value_type>,
    std::size_t Arity = container_arity<Container>::value>
class Heap {
    static_assert(std::is_same_v<T, typename Container::value_type>);
    static_assert(Arity >= 2, "Heap arity must be at least 2");
    static_assert(!is_complete_tree_v<Container> || container_arity<Container>::value == Arity,
        "Heap Arity must match the CompleteTree's Arity");

public:
    using container_type    = Container;
    using value_compare     = Compare;
    using value_type        = typename Container::value_type;
    using size_type         = typename Container::size_type;
    using reference         = typename Container::reference;
    using const_reference   = typename Container::const_reference;

    static constexpr std::size_t arity = Arity;

    Heap() = default;
    explicit Heap(const Compare& comp, Container container = Container())
        : m_container(std::move(container)), m_comp(comp)
    {
        const size_type n = size();
        if (n < 2) return;

        for (size_type i = parent_idx(n - 1) + 1; i-- > 0;) {
            sift_down(i);
        }
    }

    size_type size() const noexcept {
        return m_container.size();
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    const_reference top() const {
        if (m_container.size() < 1) {
            throw std::out_of_range("Heap is empty, cannot get top element");
        }
        return m_container[0];
    }

    // Insert element at very end, sift up from i = size() - 1
    void push(const value_type& value) {
        m_container.push_back(value);
        sift_up(size() - 1);
    }

    void push(value_type&& value) {
        m_container.push_back(std::move(value));
        sift_up(size() - 1);
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        m_container.emplace_back(std::forward<Args>(args)...);
        sift_up(size() - 1);
    }

    // Remove top element and replace it with last element, then sift down from i = 0
    void pop() {
        size_type n = size();
        if (n == 0) throw std::out_of_range("Cannot pop() on empty heap");

        using std::swap;
        if (n > 1) swap(m_container[0], m_container[n - 1]);
        m_container.pop_back();
        if (n > 1) sift_down(0);
    }

    // Prints the heap as a tree with branches, e.g. a binary max-heap:
    // 9
    // |-- 7
    // |   |-- 3
    // |   `-- 5
    // `-- 8
    //     `-- 6
    void print(std::ostream& os = std::cout) const {
        print_node(os, 0, "", true);
    }

private:
    Container m_container;
    Compare m_comp;

    // Indices for a node at index idx:
    // parent = (idx - 1) / Arity
    // 0 = idx * Arity + 1
    // 1 = idx * Arity + 2
    // ...
    // Arity - 1 = idx * Arity + Arity

    // Caller needs to guarantee that i > 0
    static constexpr size_type parent_idx(size_type i) noexcept {
        return (i - 1) / arity;
    }

    // k in [0, arity)
    static constexpr size_type child_idx(size_type i, size_type k) noexcept {
        return i * arity + k + 1;
    }

    // Prints node i, then recurses into its children. `prefix` holds the vertical bars
    // inherited from ancestors, and `last` says whether i is its parent's last child
    void print_node(std::ostream& os, size_type i, const std::string& prefix, bool last) const {
        if (i >= size()) return;

        os << prefix;
        if (i != 0) os << (last ? "`-- " : "|-- ");
        os << m_container[i] << '\n';

        const std::string child_prefix = prefix + (i == 0 ? "" : (last ? "    " : "|   "));
        const size_type first = child_idx(i, 0);
        const size_type end = std::min(first + arity, size());
        for (size_type c = first; c < end; ++c)
            print_node(os, c, child_prefix, c + 1 == end);
    }

    void sift_up(size_type i) {
        using std::swap;
        while (i > 0) {
            size_type parent = parent_idx(i);
            if (!m_comp(m_container[parent], m_container[i])) break;
            swap(m_container[parent], m_container[i]);
            i = parent;
        }
    }

    void sift_down(size_type i) {
        using std::swap;
        const size_type n = size();
        while (true) {
            size_type best = i;
            size_type first = child_idx(i, 0);
            for (size_type child = first; child < first + arity && child < n; child++) {
                if (m_comp(m_container[best], m_container[child])) best = child;
            }
            if (best == i) break;
            swap(m_container[i], m_container[best]);
            i = best;
        }
    }
};
