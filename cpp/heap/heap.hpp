#pragma once

#include <vector>
#include <functional>

// Heap class with different allocators
// E.g. T = std::vector or T = NodeGraph

template <
    typename T, 
    typename Container = std::vector<T>, 
    typename Compare = std::less<typename Container::value_type>>
class Heap {
    static_assert(std::is_same_v<T, typename Container::value_type>);
};