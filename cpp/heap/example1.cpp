// MUST BE COMPILED WITH -std=c++20 because of complete_tree.hpp

#include "heap.hpp"

using dtype = int;

int main() {
    Heap<dtype> heap;
    for (dtype v : {4, 12, 7, 1, 15, 9, 3, 11, 67, 69, 21, 100, -10, -11, -12}) {
        heap.push(v);
    }
    heap.print();
}
