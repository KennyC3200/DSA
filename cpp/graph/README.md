# Node Graph
This implementation utilizes the `Node` class and allocators which you can specify, defaulting to `std::allocator<T>`. The full source code is [here](node_graph.hpp).

The `edge`'s value is the index of the corresponding node in the `NodeGraph` nodes array.

## Usage
The most [basic usage](example1.cpp) with the default allocator which uses new/delete on the heap
```cpp
int main() {
    NodeGraph<int> graph;

    auto a = graph.AddNode(1);
    auto b = graph.AddNode(2);
    auto c = graph.AddNode(3);

    graph.AddEdge(a, b);
    graph.AddEdge(a, c);
    graph.AddEdge(b, c);
    graph.Print();
}
```

Producing the following output
```
1: 2 3 
2: 3 
3:
```

And if you were to say use a custom [stack arena](../arena_allocator/README.md)
```cpp
constexpr std::size_t ARENA_BYTES = 1024;
constexpr std::size_t NODES_CNT = 8;

template <class T, std::size_t BUFFER_SIZE = ARENA_BYTES>
using Graph = UndirectedGraph<T, short_alloc<T, BUFFER_SIZE>>;

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
    graph.Print();
}
```

Producing the following output
```
1: 2 3
2: 1 3
3: 1 2
```
The souce code for this example is [here](example2.cpp).

## Node Class
The node class itself uses `32 + sizeof(T) + padding(T)` bytes:
```
T m_val;
edge_container m_edges;
```
For `T`, it is self explanatory. E.g. if `T` was an `int`, that would be 4 bytes + 4 bytes padding for a standard alignment of 8 bytes. Next, since `edge_container` is a `std::vector`, it is 8 bytes (an allocator reference), and 24 bytes for another three pointers (`begin`, `end`, `capacity`)--yielding a total of 32 bytes. So in total, it will be 32 + 8 = 40 bytes.

# Complete Tree
A node-based container that encapsulates dynamic size complete trees. This container wraps the `NodeGraph` class and follows the guidlines of C++ STL's containers. The implementation is [here](complete_tree.hpp). 

Keep in mind that it needs to be compiled with at least C++20 because it uses C++20 `concepts`.

## Usage
Consider the following [example](example3.cpp)
```cpp
constexpr std::size_t arena_bytes = 1024;

template <class T, std::size_t BufferSize = arena_bytes>
using Tree = CompleteTree<T, short_alloc<T, BufferSize>, 2>;
using BinaryTree = Tree<int>;

int main() {
    BinaryTree::allocator_type::arena_type arena;
    BinaryTree tree(arena);

    tree.push_back(0);
    tree.push_back(1);
    tree.push_back(2);
    tree.push_back(3);
    tree.push_back(4);
    tree.push_back(5);
    tree.push_back(6);
    tree.push_back(7);
    tree.push_back(8);
    tree.push_back(9);
    tree.push_back(10);
    tree.push_back(11);
    tree.Print();
}
```

Producing the following output
```
0
|-- 1
|   |-- 3
|   |   |-- 7
|   |   `-- 8
|   `-- 4
|       |-- 9
|       `-- 10
`-- 2
    |-- 5
    |   `-- 11
    `-- 6
```
