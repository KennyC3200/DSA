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

## Node Class
The node class itself uses `32 + sizeof(T) + padding(T)` bytes:
```
T m_val;
edge_container m_edges;
```
For `T`, it is self explanatory. E.g. if `T` was an `int`, that would be 4 bytes + 4 bytes padding for a standard alignment of 8 bytes. Next, since `edge_container` is a `std::vector`, it is 8 bytes (an allocator reference), and 24 bytes for another three pointers (`begin`, `end`, `capacity`)--yielding a total of 32 bytes. So in total, it will be 32 + 8 = 40 bytes.