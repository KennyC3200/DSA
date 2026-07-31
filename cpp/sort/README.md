# Topological Sort
This class is to be used in conjunction with the [Node Graph](../graph/README.md) class. It can be used with a custom allocator that adheres to the Allocator concept too.

## Usage
With a default allocator:
```cpp
#include "../graph/node_graph.hpp"

constexpr std::size_t NODES = 8;

int main(void) {
    using Graph = NodeGraph<int>;
    using Topo = TopoSort<Graph>;

    // Initialize the graph
    Graph graph;
    for (std::size_t i = 0; i < NODES; i++) graph.AddNode(i);

    graph.AddEdge(5, 0);
    graph.AddEdge(5, 2);
    graph.AddEdge(4, 0);
    graph.AddEdge(4, 1);
    graph.AddEdge(2, 3);
    graph.AddEdge(3, 1);
    graph.Print();

    // Initialize the topological sort class
    Topo topo(graph);

    // Sort with BFS
    auto bfs = topo.SortBFS();
    Topo::PrintList(bfs);
    std::cout << (Topo::Sorted(graph, bfs) ? "True" : "False") << std::endl;

    // Sort with DFS
    auto dfs = topo.SortDFS();
    Topo::PrintList(dfs);
    std::cout << (Topo::Sorted(graph, dfs) ? "True" : "False") << std::endl;

    return 0;
}
```
Or if you wanted to with a custom allocator, like the [Stack Allocator](../arena_allocator/README.md):
```cpp
constexpr std::size_t ARENA_BYTES = 2048;
constexpr std::size_t NODES = 8;

int main(void) {
    using Graph = NodeGraph<int>;
    using Topo = TopoSort<Graph, stack_arena<int, ARENA_BYTES>>;

    Topo::allocator_type::arena_type arena;

    Graph graph;
    for (std::size_t i = 0; i < NODES; i++) graph.AddNode(i);

    graph.AddEdge(5, 0);
    graph.AddEdge(5, 2);
    graph.AddEdge(4, 0);
    graph.AddEdge(4, 1);
    graph.AddEdge(2, 3);
    graph.AddEdge(3, 1);
    Graph.Print();

    Topo topo(graph, arena);

    auto bfs = topo.SortBFS();
    Topo::PrintList(bfs);
    std::cout << (Topo::Sorted(graph, bfs, arena) ? "True" : "False") << std::endl;

    auto dfs = topo.SortDFS();
    Topo::PrintList(dfs);
    std::cout << (Topo::Sorted(graph, dfs, arena) ? "True" : "False") << std::endl;

    return 0;
}
```
