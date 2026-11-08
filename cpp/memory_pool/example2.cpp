#include "memory_pool_STL.hpp"
#include "benchmark.hpp"
#include <iostream>
#include <iterator>
#include <list>

struct Particle {
    float m_x, m_y, m_z;
    float m_vx, m_vy, m_vz;
    int m_life;

    Particle(float x, float y, float z) 
        : m_x(x), m_y(y), m_z(z)
        , m_vx(0), m_vy(0), m_vz(0) 
        , m_life(100) {}
};

template <class List>
double ChurnList(List& list, std::size_t live_target, std::size_t ops) {
  return Churn(
      [&] {
        list.emplace_back(0.0f, 1.0f, 2.0f);
        // Could also do --list.end() but doesn't work with every container
        return std::prev(list.end());
      },
      [&](typename List::iterator it) { list.erase(it); },
      live_target, ops);
}

// Slot size of 48 is the smallest that works with g++
constexpr std::size_t SLOT_SIZE = 48;
using PoolAlloc = MemoryPoolAlloc<Particle, SLOT_SIZE>;

int main() {
    const std::size_t live_target = 100'000;
    const std::size_t ops = 20'000'000;

    // List with heap allocations
    std::list<Particle> heap_list;
    double heap_ms = ChurnList(heap_list, live_target, ops);

    // List with memory pool allocations
    MemoryPool<SLOT_SIZE> pool;
    PoolAlloc pool_alloc(pool);
    double pool_ms;

    // The list needs to be destroyed before the pool, so give it its own scope
    {
        std::list<Particle, PoolAlloc> pool_list(pool_alloc);
        pool_ms = ChurnList(pool_list, live_target, ops);
    }

    std::cout 
        << "std::list results:" << "\n"
        << " std::allocator: " << heap_ms << " ms\n"
        << "MemoryPoolAlloc: " << pool_ms << " ms\n"
        << "        speedup: " << heap_ms / pool_ms << " times\n";
}