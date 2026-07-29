# Graph
This implementation utilizes two classes: `Node` and `TreeNode`. It also utilizes allocators which you can specify, defaulting to `std::allocator<T>`.

The `edge`'s value is the index of the corresponding node in the `NodeGraph` nodes array.

## Usage
The most basic usage with the default allocator
```cpp
int main(void) {
    NodeGraph<int> graph;
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

    return 0;
}
```
And if you were to say use a custom [Stack Arena](../arena_allocator/README.md)
```cpp
constexpr std::size_t ARENA_BYTES = 1024;
constexpr std::size_t NODE_CNT = 8;

template <typename T, std::size_t BufferSize = ARENA_BYTES>
using MyGraph = NodeGraph<T, short_alloc<T, BufferSize>>;

int main(void) {
    MyGraph<int>::allocator_type::arena_type arena;
    MyGraph<int> graph(arena);
    graph.Reserve(NODE_CNT);

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

    return 0;
}
```