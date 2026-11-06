#include "node_graph.hpp"
#include "../arena_allocator/stack_arena.hpp"

#include <cstddef>

constexpr std::size_t ARENA_BYTES = 1024;
constexpr std::size_t NODES_CNT = 8;

template <class T, std::size_t BUFFER_SIZE = ARENA_BYTES>
using Graph = NodeGraph<T, short_alloc<T, BUFFER_SIZE>>;

using MyGraph = Graph<int>;

int main() {
    MyGraph::allocator_type::arena_type arena;
    MyGraph graph(arena);
    graph.Reserve(NODES_CNT);

    auto a = graph.AddNode(1);
    auto b = graph.AddNode(2);
    auto c = graph.AddNode(3);

    graph.AddEdge(a, b);
    graph.AddEdge(a, c);
    graph.AddEdge(b, c);

    for (std::size_t i = 0; i < graph.Size(); i++) {
        std::cout << graph[i].Val() << " -> ";
        for (auto adj : graph[i].Edges()) {
            std::cout << graph[adj].Val() << " ";
        }
        std::cout << '\n';
    }
}