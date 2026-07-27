#include "arena_allocator/heap_arena.hpp"
#include <iostream>
#include <string>

int main(void) {
    Arena arena = Arena(GiB(1), MiB(100));

    std::string input;
    while (true) {
        arena.PushRaw(MiB(50));
        std::getline(std::cin, input);
    }

    return 0;
}