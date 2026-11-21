#pragma once

#include <vector>
#include <functional>
#include <type_traits>
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
    static_assert(Arity >= 2, "Heap arity musst be at least 2");
    static_assert(!is_complete_tree_v<Container> || container_arity<Container>::value == Arity,
        "Heap Arity must match the CompleteTree's Arity");

public:
    static constexpr std::size_t arity = Arity;

    Heap() = default;
    explicit Heap(const Compare& comp, Container c = Container());

    // Indices for a node at index idx:
    // parent = (idx - 1) / 2
    // left   = idx * 2 + 1
    // right  = idx * 2 + 2

    // Insertion:
    // Insert element at very end, and then bubble it up until root

    // Removal:
    // Remove top element, then move last element to the top and bubble it down until leaf

private:
    Container m_container;
    Compare m_comp;
};