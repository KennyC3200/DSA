#include "complete_tree.hpp"
#include "../arena_allocator/stack_arena.hpp"

constexpr std::size_t arena_bytes = 1024;

template <class T, std::size_t BufferSize = arena_bytes>
using Tree = CompleteTree<T, short_alloc<T, BufferSize>, 2>;
using BinaryTree = Tree<int>;

void PrintGraph(const BinaryTree& tree) {
    for (const auto& node_val : tree) {
        std::cout << node_val << '\n';
    }
}

int main() {
    BinaryTree::allocator_type::arena_type arena;
    BinaryTree tree(arena);

    tree.push_back(0);
    tree.push_back(1);

    PrintGraph(tree);
}