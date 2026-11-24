// MUST BE COMPILED WITH std=c++20 because of complete_tree.hpp

#include "complete_tree.hpp"
#include "../arena_allocator/stack_arena.hpp"

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
