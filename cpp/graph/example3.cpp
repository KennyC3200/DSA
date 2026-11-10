#include "complete_tree.hpp"
#include "../arena_allocator/stack_arena.hpp"

constexpr std::size_t arena_bytes = 1024;

template <class T, std::size_t BufferSize = arena_bytes>
using Tree = CompleteTree<T, short_alloc<T, BufferSize>, 2>;
using BinaryTree = Tree<int>;

template <class T, class Alloc, std::size_t Arity>
void PrintTree(const CompleteTree<T, Alloc, Arity>& tree, std::size_t i = 0,
               const std::string& prefix = "", bool last = true) {
    if (i >= tree.size()) return;

    std::cout << prefix;
    if (i != 0) std::cout << (last ? "`-- " : "|-- ");
    std::cout << tree[i] << '\n';

    const std::string child_prefix = prefix + (i == 0 ? "" : (last ? "    " : "|   "));
    const std::size_t first = Arity * i + 1;
    const std::size_t end = std::min(first + Arity, tree.size());
    for (std::size_t c = first; c < end; ++c)
        PrintTree(tree, c, child_prefix, c + 1 == end);
}

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

    PrintTree<int, BinaryTree::allocator_type, 2> (tree);
}