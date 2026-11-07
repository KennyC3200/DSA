#include "memory_pool.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

struct Particle {
    float m_x, m_y, m_z;
    float m_vx, m_vy, m_vz;
    int m_life;

    Particle(float x, float y, float z) 
        : m_x(x), m_y(y), m_z(z)
        , m_vx(0), m_vy(0), m_vz(0) 
        , m_life(100) {}
};

template <typename Alloc, typename Free>
double Churn(Alloc alloc, Free free_, std::size_t live_target, std::size_t ops) {
    std::vector<Particle*> live;
    live.reserve(live_target);
    std::mt19937 rng(42);

    auto t0 = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < ops; i++) {
        if (live.size() < live_target) {
            live.push_back(alloc());
        } else {
            // If the allocation is not big enough, just free a random spot and construct a new one
            // in that spot
            std::size_t random_idx = rng() % live.size();
            free_(live[random_idx]);
            live[random_idx] = alloc();
        }
    }

    // Free the entire list
    for (auto* p : live) free_(p);

    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
    // Testing correctness
    {
        MemoryPool<Particle, 4> pool;
        Particle* a = pool.Create(1.0f, 2.0f, 3.0f);
        Particle* b = pool.Create(4.0f, 5.0f, 6.0f);
        pool.Destroy(a);

        Particle* c = pool.Create(7.0f, 8.0f, 9.0f);
        assert(c == a && "Freed slot is reused");

        {
            // Returned to the free list automatically at scope end
            auto h = pool.Make(0.0f, 0.0f, 0.0f);
            assert(pool.Live() == 3);
        }
        assert(pool.Live() == 2);

        pool.Destroy(a);
        pool.Destroy(b);
        std::cout << "sizeof(Particle) = " << sizeof(Particle)
                  << ", slot size = " << MemoryPool<Particle>::SlotSize() << "\n";
        std::cout << "correctness checks passed\n";
    }

    const std::size_t live_target = 100'000;
    const std::size_t ops = 20'000'000;

    double heap_ms = Churn(
        [] { return new Particle(0.0f, 1.0f, 2.0f); }, 
        [](Particle* p) { free(p); },
        live_target, ops);

    MemoryPool<Particle> pool;
    double pool_ms = Churn(
        [&] { return pool.Create(0.0f, 1.0f, 2.0f); }, 
        [&](Particle* p) { pool.Destroy(p); }, 
        live_target, ops);

    std::cout << "new/delete: " << heap_ms << " ms\n"
              << "MemoryPool: " << pool_ms << " ms\n"
              << "speedup   : " << heap_ms / pool_ms << "x\n";
}