#include "stack_arena.hpp"
#include <iostream>
#include <vector>

std::size_t memory = 0;
std::size_t alloc = 0;

void* operator new(std::size_t s) {
    memory += s;
    ++alloc;
    return malloc(s);
}

void operator delete(void* p) {
    --alloc;
    free(p);
}

void memuse() {
    std::cout << "memory: " << memory << '\n';
    std::cout << "alloc: " << alloc << '\n';
}

template <class T, std::size_t BufferSize = 200>
using SmallVector = std::vector<T, short_alloc<T, BufferSize, alignof(T)>>;

int main(void) {
    SmallVector<int>::allocator_type::arena_type a;
    SmallVector<int> v(a);
    for (int i = 0; i < 100; i++) {
        v.push_back(i);
    }
    memuse();

    return 0;
}