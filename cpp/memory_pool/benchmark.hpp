#pragma once

#include <cstddef>
#include <vector>
#include <random>
#include <chrono>

template <typename Alloc, typename Free>
double Churn(Alloc alloc, Free free_, std::size_t live_target, std::size_t ops) {
    std::vector<decltype(alloc())> live;
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
    for (auto p : live) free_(p);

    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}