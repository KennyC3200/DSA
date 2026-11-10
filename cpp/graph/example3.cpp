#include "node_graph_container.hpp"
#include "../arena_allocator/stack_arena.hpp"

constexpr std::size_t arena_bytes = 1024;

template <class T, std::size_t BufferSize = arena_bytes>
using Graph = NodeGraphContainer<T, short_alloc<T, BufferSize>>;
using MyGraph = Graph<int>;

void PrintGraph(const MyGraph& graph) {
    for (const auto& node_val : graph) {
        std::cout << node_val << '\n';
    }
}

int main() {
    MyGraph::allocator_type::arena_type arena;
    MyGraph graph(arena);

    graph.push_back(0);
    graph.push_back(1);

    PrintGraph(graph);
}