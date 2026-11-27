// MUST BE COMPILED WITH -std=c++20 because of complete_tree.hpp

#include "heap.hpp"
#include "../graph/node_graph.hpp"
#include "../arena_allocator/stack_arena.hpp"

template <typename T, std::size_t BufferSize = 4096>
using MyArena = short_alloc<T, BufferSize>;

template <typename T, std::size_t Arity = 4>
using MyTree = CompleteTree<T, MyArena<T>, Arity>;

template <typename T, std::size_t Arity = 4>
using MyHeap = Heap<T, MyTree<T>>;

using dtype = int;

int main() {
    MyArena<dtype>::arena_type arena;
    MyHeap<dtype> heap(std::less<dtype>{}, MyTree<dtype>(arena));
    for (dtype v : {4, 12, 7, 1, 15, 9, 3, 11, 67, 69, 21, 100, -10, -11, -12}) {
        heap.push(v);
    }
    heap.print();
    std::cout << "Arena Used: " << arena.used() << " / " << arena.size() << " bytes\n";
}
